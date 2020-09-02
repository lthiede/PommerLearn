#include <catch2/catch.hpp>
#include "bboard.hpp"
#include "pomcpp_util.h"

TEST_CASE( "Load State", "[json]" )
{
    std::string jState =
            "{\"board_size\": 11, \"step_count\": 87, \"board\": [[0, 1, 1, 0, 0, 1, 1, 2, 1, 0, 1], [1, 0, 0, 0, 10, 0, 2, 0, 0, 0, 1], [1, 0, 0, 1, 1, 1, 2, 0, 0, 0, 0], [0, 0, 1, 0, 1, 2, 0, 1, 0, 0, 0], [0, 0, 1, 1, 0, 0, 0, 0, 13, 0, 1], [1, 0, 1, 2, 0, 0, 1, 0, 0, 0, 1], [1, 0, 2, 0, 0, 1, 0, 0, 0, 0, 0], [0, 0, 0, 1, 0, 0, 4, 0, 1, 0, 0], [1, 0, 0, 0, 0, 0, 4, 1, 0, 0, 1], [0, 0, 0, 0, 0, 4, 4, 4, 4, 0, 1], [1, 1, 0, 0, 1, 1, 4, 11, 1, 1, 0]], \"agents\": [{\"agent_id\": 0, \"is_alive\": true, \"position\": [1, 4], \"ammo\": 3, \"blast_strength\": 5, \"can_kick\": false}, {\"agent_id\": 1, \"is_alive\": true, \"position\": [10, 7], \"ammo\": 4, \"blast_strength\": 3, \"can_kick\": false}, {\"agent_id\": 2, \"is_alive\": false, \"position\": [3, 8], \"ammo\": 1, \"blast_strength\": 3, \"can_kick\": true}, {\"agent_id\": 3, \"is_alive\": true, \"position\": [4, 8], \"ammo\": 1, \"blast_strength\": 4, \"can_kick\": true}], \"bombs\": [], \"flames\": [{\"position\": [7, 6], \"life\": 2}, {\"position\": [8, 6], \"life\": 2}, {\"position\": [9, 5], \"life\": 2}, {\"position\": [9, 6], \"life\": 2}, {\"position\": [9, 7], \"life\": 2}, {\"position\": [9, 8], \"life\": 2}, {\"position\": [10, 6], \"life\": 2}], \"items\": [[[1, 6], 8], [[9, 5], 8], [[2, 6], 6], [[3, 5], 6], [[5, 3], 8]], \"intended_actions\": [4, 0, 0, 3]}"
    ;

    bboard::State s = StateFromJSON(jState, bboard::GameMode::FreeForAll);
    std::cout << "Loaded state:" << std::endl;
    bboard::PrintState(&s);
}

TEST_CASE( "Load Observation", "[json]" )
{
    std::string jObs =
            "{\"alive\": [0, 1, 3], \"board\": [[0, 1, 1, 0, 0, 1, 1, 2, 1, 0, 1], [1, 0, 0, 0, 0, 10, 2, 0, 0, 0, 1], [1, 0, 0, 1, 1, 1, 2, 0, 0, 0, 0], [0, 0, 1, 0, 1, 2, 0, 1, 0, 0, 0], [0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1], [1, 0, 1, 2, 0, 0, 1, 0, 13, 0, 1], [1, 0, 2, 0, 0, 1, 0, 0, 0, 0, 0], [0, 0, 0, 1, 0, 0, 4, 0, 1, 0, 0], [1, 0, 0, 0, 0, 0, 4, 1, 0, 0, 1], [0, 0, 0, 0, 0, 4, 4, 4, 4, 0, 1], [1, 1, 0, 0, 1, 1, 4, 11, 1, 1, 0]], \"bomb_blast_strength\": [[0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]], \"bomb_life\": [[0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]], \"bomb_moving_direction\": [[0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]], \"flame_life\": [[0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 2.0, 0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 2.0, 0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0, 0.0, 2.0, 2.0, 2.0, 2.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 2.0, 0.0, 0.0, 0.0, 0.0]], \"game_type\": 1, \"game_env\": \"pommerman.envs.v0:Pomme\", \"position\": [1, 5], \"blast_strength\": 5, \"can_kick\": false, \"teammate\": [], \"ammo\": 3, \"enemies\": [1, 2, 3], \"step_count\": 87}"
    ;
    bboard::Observation o = ObservationFromJSON(jObs, 0);
    std::cout << "Loaded observation:" << std::endl;
    bboard::PrintObservation(&o);
}