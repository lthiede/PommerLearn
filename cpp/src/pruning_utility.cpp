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

}
