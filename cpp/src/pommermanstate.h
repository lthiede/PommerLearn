/*
 * @file: pommermanstate.h
 * Created on 15.07.2020
 * @author: queensgambit
 *
 * PommermanState implements the State interface for the Pommerman C++ environment.
 */

#ifndef POMMERMANSTATE_H
#define POMMERMANSTATE_H

#include "state.h"
#include "bboard.hpp"
#include "data_representation.h"

#include "clonable.h"


class StateConstantsPommerman : public StateConstantsInterface<StateConstantsPommerman>
{
public:
    static uint BOARD_WIDTH() {
        return 11;
    }
    static uint BOARD_HEIGHT() {
        return 11;
    }
    static uint NB_CHANNELS_TOTAL() {
        return 18;
    }
    static uint NB_LABELS() {
        return 6;
    }
    static uint NB_LABELS_POLICY_MAP() {
        return 6;
    }
    static uint NB_AUXILIARY_OUTPUTS() {
        return 0;  // TODO
    }
    static uint NB_PLAYERS() {
        return 4;
    }
    static std::string action_to_uci(Action action, bool is960) {
        switch(bboard::Move(action)) {
        case (bboard::Move::IDLE):
            return "IDLE";
        case (bboard::Move::UP):
            return "UP";
        case (bboard::Move::DOWN):
            return "DOWN";
        case (bboard::Move::LEFT):
            return "LEFT";
        case (bboard::Move::RIGHT):
            return "RIGHT";
        case (bboard::Move::BOMB):
            return "BOMB";
        default:
            return "UNKNOWN";
        }
    }
    template<PolicyType p = normal, MirrorType m = notMirrored>
    static MoveIdx action_to_index(Action action) {
        return std::clamp(int(action), 0, NUM_MOVES);
    }
    static void init(bool isPolicyMap) {
        return; // pass
    }
};

class PommermanState : public State
{
public:
    PommermanState(uint agentID, bboard::GameMode gameMode);
    bboard::State state;
    bboard::Move moves[bboard::AGENT_COUNT];
    const uint agentID;
    const bboard::GameMode gameMode;
    bool usePartialObservability;
    bboard::ObservationParameters params;
    int eventHash;
    std::vector<float> auxiliaryOutputs;

    /**
     * @brief planningAgents contains other agents which can be used in the planning process.
     */
    std::array<std::unique_ptr<Clonable<bboard::Agent>>, bboard::AGENT_COUNT> planningAgents;

    /**
     * @brief hasPlanningAgents whether there are any planningAgents.
     */
    bool hasPlanningAgents;

    /**
     * @brief hasBufferedActions whether the actions of the planning agents have already been evaluated for the current state.
     */
    bool hasBufferedActions;

public:
    void set_state(const bboard::State* state);
    void set_observation(const bboard::Observation* obs);
    void set_partial_observability(const bboard::ObservationParameters* params);

    // planning agent methods
    void set_planning_agents(const std::array<Clonable<bboard::Agent>*, bboard::AGENT_COUNT> agents);
    void planning_agents_reset();
    void planning_agents_act();

    // State interface
    std::vector<Action> legal_actions() const override;
    void set(const std::string &fenStr, bool isChess960, int variant) override;
    void get_state_planes(bool normalize, float *inputPlanes) const override;
    unsigned int steps_from_null() const override;
    bool is_chess960() const override;
    std::string fen() const override;
    void do_action(Action action) override;
    void undo_action(Action action) override;
    void prepare_action() override;
    unsigned int number_repetitions() const override;
    int side_to_move() const override;
    Key hash_key() const override;
    void flip() override;
    Action uci_to_action(std::string &uciStr) const override;
    std::string action_to_san(Action action, const std::vector<Action>& legalActions, bool leadsToWin, bool bookMove) const override;
    TerminalType is_terminal(size_t numberLegalMoves, bool inCheck, float& customTerminalValue) const override;
    Result check_result(bool inCheck) const override;
    bool gives_check(Action action) const override;
    void print(std::ostream& os) const override;
    Tablebase::WDLScore check_for_tablebase_wdl(Tablebase::ProbeState& result) override;
    void set_auxiliary_outputs(const float* auxiliaryOutputs) override;
    PommermanState* clone() const override;
};

#endif // POMMERMANSTATE_H
