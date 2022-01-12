//
// Created by Tobii on 11.01.2022.
//

#ifndef MCTSAGENT_CPP_PRUNING_UTILITY_H
#define MCTSAGENT_CPP_PRUNING_UTILITY_H

#include "state.h"
#include "bboard.hpp"
#include <vector>


float manhattan_distance(bboard::Position pos1, bboard:Position pos2);
bool stop_condition(bboard::board board, bboard:Position pos, bool exclude_agent);
// can't be bboard::Direction because of idle
std::vector<bboard::Move> all_directions(bool exclude_stop = true);
bboard::Direction opposite_direction(bboard::Direction direction);

#endif //MCTSAGENT_CPP_PRUNING_UTILITY_H
