//
// Created by Tobii on 11.01.2022.
//

#ifndef MCTSAGENT_CPP_PRUNING_UTILITY_H
#define MCTSAGENT_CPP_PRUNING_UTILITY_H

#include "state.h"
#include "bboard.hpp"
#include "step_utility.hpp"
#include <vector>
#include <unordered_set>
#include <algorithm>
#include <stdexcept>

namespace action_pruning
{

const float EPSILON = 0.001;
const int INT_MAX = 9999;
const int FLAME_LIFE = 2;

// Step 0:
// additional helpers
std::vector<std::vector<int>> get_bomb_blast_st(bboard::FixedQueue<bboard::Bomb,
  bboard::MAX_BOMBS> bombs);
std::vector<std::vector<int>> get_bomb_life(bboard::FixedQueue<bboard::Bomb,
  bboard::MAX_BOMBS> bombs);
// Step 1:
// müssen wir die wirklich hier aufführen? sind ja nur intern
bool moving_bomb_check(bboard::Board board,
  std::vector<std::vector<int>> bomb_life,
  std::vector<std::vector<int>> blast_st, bboard::Position moving_bomb_pos,
  bboard::Direction p_dir, int time_elapsed);
  bool kick_test(bboard::Board board,
    std::vector<std::vector<int>> blast_st,
    std::vector<std::vector<int>> bomb_life, bboard::Position my_position,
    bboard::Move direction);
float manhattan_distance(bboard::Position pos1, bboard::Position pos2);
bool stop_condition(bboard::Board board, bboard::Position pos,
  bool exclude_agent = true);
// can't be bboard::Direction because of idle
std::vector<bboard::Move> all_directions(bool exclude_stop = true);
bboard::Direction opposite_direction(bboard::Direction direction);
// Step 2:
int get_bomb_real_life(bboard::Position pos, int bomb_real_life);
std::vector<std::vector<int>> all_bomb_real_life(bboard::Board board,
  std::vector<std::vector<int>> bomb_life,
  std::vector<std::vector<int>> bomb_blast_st);
std::tuple<bool, int, int> position_covered_by_bomb(bboard::Observation obs,
  bboard::Position pos, std::vector<std::vector<int>> bomb_real_life_map);
bool check_if_flame_will_be_gone(bboard::Observation obs,
  std::vector<bboard::Observation> prev_two_obs, bboard::Position flame_pos);
// Step 3:
int compute_min_evade_step(bboard::Observation obs,
  std::unordered_set<bboard::Position> parent_pos_list, bboard::Position pos,
  std::vector<std::vector<int>> bomb_real_life);
// Step 4:
unordered_set<bboard::Move> compute_safe_actions(bboard::Observation obs,
  std::vector<bboard::Observation> prev_two_obs, bool exclude_kicking = false);
}

#endif //MCTSAGENT_CPP_PRUNING_UTILITY_H
