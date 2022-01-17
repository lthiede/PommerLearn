//
// Created by Tobii on 11.01.2022.
//
#include "pruning_utility.h"
#include "cmath" // for abs??

namespace action_pruning
{

float manhattan_distance(bboard::Position pos1, bboard::Position pos2) {
    return std::abs(pos1.x - pos2.x) + std::abs(pos1.y - pos2.y);
}

bool stop_condition(bboard::Board board, bboard::Position pos, bool exclude_agent) {
    if(bboard::IsOutOfBounds(pos.x, pos.y))
    {
        return true;
    }
    int item = board.GetItem(pos.x, pos.y);
    // is rigid the correct translation for wall or should wood also be considered
    if(item == bboard::Item::RIGID) {
        return true;
    } else if (item == bboard::Item::FOG) {
        return true;
    } else if (!exclude_agent && bboard::IS_AGENT(item)) {
        return true;
    }
    return false;
}

std::vector<bboard::Move> all_directions(bool exclude_stop) {
    std::vector<bboard::Move> dirs;
    dirs.push_back(bboard::Move::LEFT);
    dirs.push_back(bboard::Move::RIGHT);
    dirs.push_back(bboard::Move::UP);
    dirs.push_back(bboard::Move::DOWN);
    if(exclude_stop == false){
        dirs.push_back(bboard::Move::IDLE);
    }
    return dirs;
}

bboard::Direction opposite_direction(bboard::Direction direction) {
    switch(direction){
        case bboard::Direction::LEFT:
            return bboard::Direction::RIGHT;
        case bboard::Direction::RIGHT:
            return bboard::Direction::LEFT;
        case bboard::Direction::UP:
            return bboard::Direction::DOWN;
        case bboard::Direction::DOWN:
            return bboard::Direction::UP;
        /*default:
            return NULL
        */
    }
}

int get_bomb_real_life(bboard::Board board, bboard::Position bomb_position,
  std::vector<std::vector<int>> bomb_real_life,
  std::vector<std::vector<int>> bomb_blast_st) {
    /*One bomb's real life is the minimum life of its adjacent bomb.
      Not that this could be chained, so please call it on each bomb mulitple times until
      converge
      */
    std::vector<bboard::Move> dirs = all_directions(true);
    int min_life = bomb_real_life[bomb_position.x][bomb_position.y];
    for (bboard::Move d : dirs) {
        bboard::Position pos = bomb_position;
        bboard::Position last_pos = bomb_position;
        while (true) {
            pos = bboard::util::DesiredPosition(pos.x, pos.y, d);
            if (stop_condition(board, pos)) {
                break;
            }
            if (bomb_real_life[pos.x][pos.y] > 0) {
                if (bomb_real_life[pos.x][pos.y] < min_life &&
                  manhattan_distance(pos, last_pos) <= bomb_blast_st[pos.x][pos.y] - 1) {
                    min_life = bomb_real_life[pos.x][pos.y];
                    last_pos = pos;
                } else {
                    break;
                }
            }
        }
    }
    return min_life;
}

std::vector<std::vector<int>> all_bomb_real_life(bboard::Board board,
  std::vector<std::vector<int>> bomb_life,
  std::vector<std::vector<int>> bomb_blast_st) {
  std::vector<std::vector<int>> bomb_real_life_map = bomb_life;
    while (true) {
        bool no_change = true;
        for (int i = 0; i < bboard::BOARD_SIZE; i++) {
            for (int j = 0; j < bboard::BOARD_SIZE; j++) {
                int item = board.GetItem(i, j);
                if (item == bboard::Item::RIGID || bboard::IS_POWERUP(item) ||
                  item == bboard::Item::FOG) {
                    continue;
                }
                if (bomb_life[i][j] < 0 + EPSILON) {
                    continue;
                }
                bboard::Position pos;
                pos.x = i;
                pos.y = j;
                int real_life = get_bomb_real_life(board, pos,
                  bomb_real_life_map, bomb_blast_st);
                if (bomb_real_life_map[i][j] != real_life) {
                    no_change = false;
                }
                bomb_real_life_map[i][j] = real_life;
            }
        }
        if (no_change) {
          break;
        }
    }
    return bomb_real_life_map;
}

std::tuple<bool, int, int> position_covered_by_bomb(bboard::Observation obs,
  bboard::Position pos, std::vector<std::vector<int>> bomb_real_life_map) {
    //return a tuple (True/False, min_bomb_life_value, max life value)
    bboard::Position min_bomb_pos, max_bomb_pos;
    min_bomb_pos.x = -1;
    int min_bomb_value = INT_MAX, max_bomb_value = -INT_MAX;
    if (obs.GetBomb(pos.x, pos.y) > 0) {
        min_bomb_value = max_bomb_value = bomb_real_life_map[pos.x][pos.y];
        min_bomb_pos = max_bomb_pos = pos;
    }
    std::vector<bboard::Move> dirs = all_directions();
    // board = obs['board'] weggelassen, da mit cpp obs von board erbt
    for (bboard::Move d : dirs) {
        bboard::Position next_pos = pos;
        while (true) {
            next_pos = bboard::util::DesiredPosition(next_pos.x, next_pos.y, d);
            if (stop_condition(obs, next_pos)) {
                //here should assume agents are dynamic
                break;
            }
            bboard::Bomb bomb = *(obs.GetBomb(next_pos.x, next_pos.y));
            if (bomb > 0 && bboard::BMB_STRENGTH(bomb) - 1 >=
              manhattan_distance(pos, next_pos)) {
                if (bomb_real_life_map[next_pos.x][next_pos.y] < min_bomb_value) {
                    int min_bomb_value = bomb_real_life_map[next_pos.x][next_pos.y];
                    min_bomb_pos = next_pos;
                }
                if (bomb_real_life_map[next_pos.x][next_pos.y] > max_bomb_value) {
                    int max_bomb_value = bomb_real_life_map[next_pos.x][next_pos.y];
                    max_bomb_pos = next_pos;
                }
                break;
            }
        }
    }
    if (min_bomb_pos.x != -1) {
        return std::make_tuple(true, min_bomb_value, max_bomb_value);
    }
    return std::make_tuple(false, INT_MAX, -INT_MAX);
}

}
