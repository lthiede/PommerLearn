//
// Created by Tobii on 11.01.2022.
//

#ifndef MCTSAGENT_CPP_PRUNING_UTILITY_H
#define MCTSAGENT_CPP_PRUNING_UTILITY_H

#include "state.h"
#include "bboard.hpp"
#include "step_utility.hpp"
#include <vector>

namespace action_pruning
{

const float EPSILON = 0.001;
const int INT_MAX = 9999;

//Step 1:
float manhattan_distance(bboard::Position pos1, bboard::Position pos2);
bool stop_condition(bboard::Board board, bboard::Position pos, bool exclude_agent = true);
// can't be bboard::Direction because of idle
std::vector<bboard::Move> all_directions(bool exclude_stop = true);
bboard::Direction opposite_direction(bboard::Direction direction);
// Step 2:
int get_bomb_real_life(bboard::Position pos, int bomb_real_life);
}

#endif //MCTSAGENT_CPP_PRUNING_UTILITY_H
