//
// Created by Tobii on 11.01.2022.
//
#include "pruning_utility.h"
#include "cmath" // for abs??

namespace action_pruning
{

std::vector<std::vector<int>> get_bomb_blast_st(bboard::FixedQueue<bboard::Bomb,
  bboard::MAX_BOMBS> bombs) {
    std::vector<std::vector<int>> bomb_blast_st(bboard::BOARD_SIZE,
      std::vector<int>(bboard::BOARD_SIZE));
    while (bombs.count > 0) {
        bboard::Bomb bomb = bombs.PopElem();
        bomb_blast_st[bboard::BMB_POS_X(bomb)][bboard::BMB_POS_Y(bomb)] = bboard::BMB_STRENGTH(bomb);
    }
    return bomb_blast_st;
}

std::vector<std::vector<int>> get_bomb_life(bboard::FixedQueue<bboard::Bomb,
  bboard::MAX_BOMBS> bombs) {
    std::vector<std::vector<int>> bomb_life(bboard::BOARD_SIZE,
      std::vector<int>(bboard::BOARD_SIZE));
    while (bombs.count > 0) {
        bboard::Bomb bomb = bombs.PopElem();
        bomb_life[bboard::BMB_POS_X(bomb)][bboard::BMB_POS_Y(bomb)] = bboard::BMB_TIME(bomb);
    }
    return bomb_life;
}

bboard::Position get_next_position(bboard::Position position, bboard::Direction direction) {

    bboard::Position ret;

    if (direction == bboard::Move::RIGHT) {
        ret.x = position.x;
        ret.y = position.y + 1;
    }
    else if (direction == bboard::Move::LEFT) {
        ret.x = position.x;
        ret.y = position.y - 1;
    }
    else if (direction == bboard::Move::DOWN) {
        ret.x = position.x;
        ret.y = position.y + 1;
    }
    else if (direction == bboard::Move::UP) {
        ret.x = position.x - 1;
        ret.y = position.y;
    }
    else if (direction == bboard::Move::IDLE) {
        ret.x = position.x;
        ret.y = position.y;
    }

    return ret;
}

bool is_moving_direction(bboard::Position bomb_pos, bboard::Direction direction, bboard::Observation prev_obs) {
    rev_d = opposite_direction(direction);
    rev_pos = get_next_position(bomb_pos, rev_d);
    if (bboard::IsOutOfBounds(rev_pos.x, rev_pos.y)) {
        return false;
    }
    bomb_rev_pos = prev_obs.board.GetBomb(rev_pos.x, rev_pos.y); // TODO möglicher Fehler wegen obs.board
    bomb_bomb_pos = obs.board.GetBomb(bomb_pos.x, bomb_pos.y); // TODO möglicher Fehler wegen obs.board
    bomb_life_rev_pos = bboard::BMB_TIME(bomb_rev_pos);
    bomb_life_bomb_pos = bboard::BMB_TIME(bomb_bomb_pos);
    bomb_strength_rev_pos = bboard::BMB_STRENGTH(bomb_rev_pos);
    bomb_strength_bomb_pos = bboard::BMB_STRENGTH(bomb_bomb_pos);
    if (bomb_life_rev_pos - 1 == bomb_life_bomb_pos &&
      bomb_strength_rev_pos == bomb_strength_bomb_pos &&
      prev_obs.board.GetItem(bomb_pos.x, bomb_pos.y) == bboard::Item::PASSAGE) { // TODO möglicher Fehler wegen obs.board
        return true;
    }
    return false;
}

bool moving_bomb_check(bboard::Board board,
  std::vector<std::vector<int>> blast_st,
  std::vector<std::vector<int>> bomb_life, bboard::Position moving_bomb_pos,
  bboard::Move p_dir, int time_elapsed) {
    bboard::Position pos2 = bboard::util::DesiredPosition(moving_bomb_pos.x,
      moving_bomb_pos.y, p_dir);
    int dist = 0;
    for (int i = 0; i < 10; i++) {
        dist += 1;
        if (bboard::IsOutOfBounds(pos2.x, pos2.y)) {
            break;
        }
        if (!bboard::IS_WALKABLE(board.GetItem(pos2.x, pos2.y))) {
            break;
        }
        int life_now = bomb_life[pos2.x][pos2.y] - time_elapsed;
        if (bomb_life[pos2.x][pos2.y] > 0 && life_now >= -2 && life_now <= 0 &&
          dist < blast_st[pos2.x][pos2.y]) {
            return false;
        }
    }
    return true;
}

bool kick_test(bboard::Board board,
  std::vector<std::vector<int>> blast_st,
  std::vector<std::vector<int>> bomb_life, bboard::Position my_position,
  bboard::Move direction) {
    bboard::Position next_position = bboard::util::DesiredPosition(my_position.x,
      my_position.y, direction);
    if (!board.HasBomb(my_position.x, my_position.y)) {
        throw std::invalid_argument( "no bomb at position" );
    }
    int life_value = bomb_life[next_position.x][next_position.y];
    int strength = blast_st[next_position.x][next_position.y];
    int dist = 0;
    bboard::Position pos = bboard::util::DesiredPosition(next_position.x,
      next_position.y, direction);
    std::vector<bboard::Move> perpendicular_dirs;
    if (direction == bboard::Move::LEFT || direction == bboard::Move::LEFT) {
        perpendicular_dirs.push_back(bboard::Move::UP);
        perpendicular_dirs.push_back(bboard::Move::DOWN);
    } else {
        perpendicular_dirs.push_back(bboard::Move::LEFT);
        perpendicular_dirs.push_back(bboard::Move::RIGHT);
    }
    for (int i = 0; i < life_value; i++) {
        if (!bboard::IsOutOfBounds(pos.x, pos.y) &&
          board.GetItem(pos.x, pos.y) == bboard::Item::PASSAGE) {
            // do a check if this position happens to be in flame when the moving bomb arrives!
            if (!(moving_bomb_check(board, blast_st, bomb_life, pos,
              perpendicular_dirs[0], i) && moving_bomb_check(board, blast_st,
              bomb_life, pos, perpendicular_dirs[1], i))) {
                break;
            }
            dist += 1;
        } else {
            break;
        }
        pos = bboard::util::DesiredPosition(pos.x, pos.y, direction);
        // can kick and kick direction is valid
    }
    return dist > strength;
}

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

bool check_if_flame_will_be_gone(bboard::Observation obs,
  std::vector<bboard::Observation> prev_two_obs, bboard::Position flame_pos) {
    /*
    assert(prev_two_obs[0] is not None)
    assert(prev_two_obs[1] is not None)
    if the elements don't exist an error will be thrown
    #check the flame group in current obs, see if
    #the whole group was there prev two obs
    #otherwise, although this flame appears in prev two obs,
    #it could be a old overlap new, thus will not gone next step
    */
    if (!(bboard::IS_FLAME(prev_two_obs.at(0).GetItem(flame_pos.x, flame_pos.y)) &&
      bboard::IS_FLAME(prev_two_obs.at(1).GetItem(flame_pos.x, flame_pos.y)))) {
        return false;
    }
    // board = obs['board'] weggelassen, da mit cpp obs von board erbt
    std::vector<bboard::Position> q;
    // at to the front and remove in the back
    q.insert(q.begin(), flame_pos);
    std::unordered_set<bboard::Position> visited = {flame_pos};
    std::vector<bboard::Move> dirs = all_directions();
    while (q.size() > 0) {
        // remove in the back and at to the front
        bboard::Position pos = q.back();
        q.pop_back();
        if (!(bboard::IS_FLAME(prev_two_obs.at(0).GetItem(pos.x, pos.y)) &&
          bboard::IS_FLAME(prev_two_obs.at(1).GetItem(pos.x, pos.y)))) {
            return false;
        }
        for (bboard::Move d : dirs) {
            bboard::Position next_pos = bboard::util::DesiredPosition(pos.x, pos.y, d);
            if (!bboard::IsOutOfBounds(next_pos.x, next_pos.y) &&
              bboard::IS_AGENT(obs.GetItem(next_pos.x, next_pos.y))) {
                if (visited.find(next_pos) == visited.end()) {
                  q.insert(q.begin(), next_pos);
                  visited.insert(next_pos);
                }
            }
        }
    }
    return true;
}

int compute_min_evade_step(bboard::Observation obs,
  std::unordered_set<bboard::Position> parent_pos_list, bboard::Position pos,
  std::vector<std::vector<int>> bomb_real_life) {
    std::tuple<bool, int, int> covered = position_covered_by_bomb(obs, pos, bomb_real_life);
    bool flag_cover = std::get<0>(covered);
    int min_cover_value = std::get<1>(covered);
    int max_cover_value = std::get<2>(covered);
    if (!flag_cover) {
        return 0;
    } else if (parent_pos_list.size() >= max_cover_value) {
        if (parent_pos_list.size() > max_cover_value + FLAME_LIFE) {
            return 0;
        } else {
            return INT_MAX;
        }
    } else if (parent_pos_list.size() >= min_cover_value) {
        if (parent_pos_list.size() > min_cover_value + FLAME_LIFE) {
            return 0;
        } else {
            return INT_MAX;
        }
    } else {
        // board = obs['board'] weggelassen, da mit cpp obs von board erbt
        std::vector<bboard::Move> dirs = all_directions();
        int min_step = INT_MAX;
        for (bboard::Move d : dirs) {
            bboard::Position next_pos = bboard::util::DesiredPosition(pos.x, pos.y, d);
            if (bboard::IsOutOfBounds(next_pos.x, next_pos.y)) {
                continue;
            }
            // is walkable is identical to is powerup or is item
            if (!bboard::IS_WALKABLE(obs.GetItem(next_pos.x, next_pos.y))) {
                continue;
            }
            if (parent_pos_list.find(next_pos) != parent_pos_list.end()) {
                continue;
            }
            std::unordered_set<bboard::Position> next_parent_pos_list = parent_pos_list;
            next_parent_pos_list.insert(next_pos);
            int x = compute_min_evade_step(obs, next_parent_pos_list,
              next_pos, bomb_real_life);
            int min_step = std::min(min_step, x+1);
        }
        return min_step;
    }
}

unordered_set<bboard::Move> compute_safe_actions(bboard::Observation obs,
  std::vector<bboard::Observation> prev_two_obs, bool exclude_kicking) {
    std::vector<bboard::Move> dirs = all_directions();
    unordered_set<bboard::Move> ret;
    bboard::Position my_position;
    my_position.x = obs.agents[obs.agentID].x;
    my_position.y = obs.agents[obs.agentID].y;
    // board = obs['board'] weggelassen, da mit cpp obs von board erbt
    std::vector<std::vector<int>> blast_st = get_bomb_blast_st(obs.bombs);
    std::vector<std::vector<int>> bomb_life = get_bomb_life(obs.bombs);
    bool can_kick = obs.agents[obs.agentID].canKick;
    bboard::Move kick_dir = bboard::Move::IDLE; // IDLE is null value
    std::vector<std::vector<int>> bomb_real_life_map = all_bomb_real_life(obs,
      bomb_life, blast_st);
    bool flag_cover_passages = false;
    for (bboard::Move direction : dirs) {
        bboard::Position position = bboard::util::DesiredPosition(my_position.x,
          my_position.y, direction);
        if (bboard::IsOutOfBounds(my_position.x, my_position.y)) {
            continue;
        }
        if (!exclude_kicking && obs.HasBomb(position.x, position.y) &&
          can_kick) {
            // filter kick if kick is unsafe
            if (kick_test(obs, blast_st, bomb_real_life_map, my_position,
              direction)) {
                ret.insert(direction);
                kick_dir = direction;
            }
        }
        bboard::Position gone_flame_pos;
        gone_flame_pos.x = -1;
        if (prev_two_obs.size() == 2 && check_if_flame_will_be_gone(obs,
          prev_two_obs, position)) {
            // three consecutive flames means next step this position must be good
            // make this a candidate
            obs.items[position.x][position.y] = bboard::Item::PASSAGE;
            gone_flame_pos = position;
        }

        if (bboard::IS_WALKABLE(obs.GetItem(position.x, position.y))) {
            int my_id = obs.GetItem(my_position.x, my_position.y);
            if (bomb_life[my_position.x][my_position.y] > 0) {
                obs.items[my_position.x][my_position.y] = bboard::Item::BOMB;
            } else {
                obs.items[my_position.x][my_position.y] = bboard::Item::PASSAGE;
            }
            std::tuple<bool, int, int> covered = position_covered_by_bomb(obs,
              position, bomb_real_life_map);
            bool flag_cover = std::get<0>(covered);
            int min_cover_value = std::get<1>(covered);
            int max_cover_value = std::get<2>(covered);
            if (!flag_cover) {
                ret.insert(direction);
            } else {
                flag_cover_passages = true;
                std::unordered_set<bboard::Position> parent_pos_list;
                parent_pos_list.insert(position);
                int min_escape_step = compute_min_evade_step(obs,
                  parent_pos_list, position, bomb_real_life_map);
                // assert min escape step > 0
                if (min_escape_step < min_cover_value) {
                    ret.insert(direction);
                }
            }
            obs.items[my_position.x][my_position.y] = my_id;
        }
        if (gone_flame_pos.x > -1) {
            obs.items[gone_flame_pos.x][gone_flame_pos.y] = bboard::Item::FLAME;
        }
    }
    // Test stop action only when agent is covered by bomb
    // otherwise stop is always an viable option
    int my_id = obs.GetItem(my_position.x, my_position.y);
    if (bomb_life[my_position.x][my_position.y] > 0) {
        obs.items[my_position.x][my_position.y] = bboard::Item::BOMB;
    } else {
        obs.items[my_position.x][my_position.y] = bboard::Item::PASSAGE;
    }
    //REMEMBER: before compute min evade step, modify board accordingly first..
    std::tuple<bool, int, int> covered = position_covered_by_bomb(obs,
      my_position, bomb_real_life_map);
    bool flag_cover = std::get<0>(covered);
    int min_cover_value = std::get<1>(covered);
    int max_cover_value = std::get<2>(covered);
    if (flag_cover) {
        std::unordered_set<bboard::Position> parent_pos_list;
        parent_pos_list.insert(my_position);
        int min_escape_step = compute_min_evade_step(obs, parent_pos_list,
          my_position, bomb_real_life_map);
        if (min_escape_step < min_cover_value) {
            ret.insert(bboard::Move::IDLE);
        }
    } else {
        ret.insert(bboard::Move::IDLE);
    }
    obs.items[my_position.x][my_position.y] = my_id;
    // REMEMBER: change the board back


    // Now test bomb action
    if (!(obs.agents[obs.agentID].maxBombCount -
      obs.agents[obs.agentID].bombCount <= 0 ||
      bomb_life[my_position.x][my_position.y] > 0)) {
        // place bomb might be possible
        if (BOMBING_TEST == "simple") {
            if (!flag_cover) {
                ret.insert(bboard::Move::BOMB);
            }
        } else if (BOMBING_TEST == "simple_adjacent") {
            if (!flag_cover && !flag_cover_passages) {
                ret.insert(bboard::Move::BOMB);
            }
        } else { // lookahead
            if (ret.find(bboard::Move::IDLE) != ret.end() && ret.size() > 1 &&
              kick_dir != bboard::Move::IDLE) { // IDLE is null value
                bboard::Observation obs2 = obs; // hope this is a deep copy
                bboard::Position my_pos;
                my_pos.x = obs2.agents[obs2.agentID].x;
                my_pos.y = obs2.agents[obs2.agentID].y;
                obs.items[my_position.x][my_position.y] = bboard::Item::BOMB;
                std::vector<std::vector<int>> blast_st2 = get_bomb_blast_st(obs.bombs);
                std::vector<std::vector<int>> bomb_life2 = get_bomb_life(obs.bombs);
                if (flag_cover) {
                    bomb_life2[my_pos.x][my_pos.y] = min_cover_value;
                } else {
                    bomb_life2[my_pos.x][my_pos.y] = 10;
                }
                std::vector<std::vector<int>> bomb_real_life_map2 =
                  all_bomb_real_life(obs2, bomb_life2, blast_st2);
                std::unordered_set<bboard::Position> parent_pos_list;
                parent_pos_list.insert(my_position);
                int min_evade_step = compute_min_evade_step(obs2, parent_pos_list,
                  my_pos, bomb_real_life_map2);
                int current_cover_value = bomb_life2[my_pos.x][my_pos.y];
                if (min_evade_step < current_cover_value) {
                    ret.insert(bboard::Move::BOMB);
                }
            }
        }
    }
    return ret;
}

std::vector<bboard::Move> get_filtered_actions(bboard::Observation obs,
  std::vector<bboard::Observation> prev_two_obs) {
    if (obs.agents[obs.agentID].dead) {
        std::vector<bboard::Move> ret;
        ret.insert(bboard::Move::IDLE);
        return ret;
    }
    bboard::Observation obs_cpy = obs;
    if (prev_two_obs.size() >= 1) {
        obs = move
    }
}

bboard::Observation move_moving_bombs_to_next_position(bboard::Observation obs, bboard::Observation prev_obs) {
    \\ TODO
}

}
