//
// Created by Tobii on 11.01.2022.
//
#include "pruning_utility.h"
#include "cmath" // for abs??

float manhattan_distance(bboard::Position pos1, bboard:Position pos2){
    return std::abs(pos1.x - pos2.x) + std::abs(pos1.y - pos2.y)
}


bool stop_condition(bboard::board board, bboard:Position pos, bool exclude_agent){
    //TODO da ist keine util funktion
    if(bboard::util::IsOutOfBounds(pos)){
        return true;
    /*} else if(){
        TODO position is fog
        TODO position is wall
        TODO position is agent if bool == false
    }*/ else{
        return false;
    }
}


std::vector<bboard::Move> all_directions(bool exclude_stop = true){
    std::vector<bboard::Move> dirs;
    dirs.push_back(bboard::Move::LEFT);
    dirs.push_back(bboard::Move::RIGHT);
    dirs.push_back(bboard::Move::UP);
    dirs.push_back(bboard::Move::DOWN);
    if(exlude_stop == false){
        dirs.push_back(bboard::Move::IDLE);
    }
    return dirs;
}


bboard::Direction opposite_direction(bboard::Direction direction){
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