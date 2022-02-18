#include "crazyara_agent.h"
#include <limits>

#ifdef TENSORRT
#include "nn/tensorrtapi.h"
#elif defined (TORCH)
#include "nn/torchapi.h"
#endif

#include "agents/rawnetagent.h"
#include "agents/mctsagent.h"

CrazyAraAgent::CrazyAraAgent(std::string modelDirectory):
    isRawNetAgent(true)
{
    singleNet = load_network(modelDirectory);
    // agent uses default playsettings, are not used anyway
    agent = std::make_unique<RawNetAgent>(singleNet.get(), &this->playSettings, false);
}

CrazyAraAgent::CrazyAraAgent(std::string modelDirectory, PlaySettings playSettings, SearchSettings searchSettings, SearchLimits searchLimits):
    playSettings(playSettings),
    searchSettings(searchSettings),
    searchLimits(searchLimits),
    isRawNetAgent(false)
{
    singleNet = load_network(modelDirectory);
    netBatches = load_network_batches(modelDirectory, searchSettings);
    agent = std::make_unique<MCTSAgent>(this->singleNet.get(), this->netBatches, &this->searchSettings, &this->playSettings);
}

void CrazyAraAgent::init_state(bboard::GameMode gameMode, bboard::ObservationParameters observationParameters, uint8_t valueVersion, PlanningAgentType planningAgentType)
{
    // assuming that the model is stateful when we have auxiliary outputs
    bool statefulModel = singleNet->has_auxiliary_outputs();

    // this is the state object of agent 0
    pommermanState = std::make_unique<PommermanState>(gameMode, statefulModel, 800, valueVersion);
    pommermanState->set_partial_observability(observationParameters);

    if(!this->isRawNetAgent)
    {
        // other agents used for planning
        switch (planningAgentType)
        {
        case SimpleUnbiasedAgent:
            this->planningAgents = {
                new CopyClonable<bboard::Agent, agents::SimpleUnbiasedAgent>(agents::SimpleUnbiasedAgent(rand())),
                new CopyClonable<bboard::Agent, agents::SimpleUnbiasedAgent>(agents::SimpleUnbiasedAgent(rand())),
                new CopyClonable<bboard::Agent, agents::SimpleUnbiasedAgent>(agents::SimpleUnbiasedAgent(rand())),
                new CopyClonable<bboard::Agent, agents::SimpleUnbiasedAgent>(agents::SimpleUnbiasedAgent(rand())),
            };
            break;

        case SimpleAgent:
            this->planningAgents = {
                new CopyClonable<bboard::Agent, agents::SimpleAgent>(agents::SimpleAgent(rand())),
                new CopyClonable<bboard::Agent, agents::SimpleAgent>(agents::SimpleAgent(rand())),
                new CopyClonable<bboard::Agent, agents::SimpleAgent>(agents::SimpleAgent(rand())),
                new CopyClonable<bboard::Agent, agents::SimpleAgent>(agents::SimpleAgent(rand())),
            };
            break;

        case LazyAgent:
            this->planningAgents = {
                new CopyClonable<bboard::Agent, agents::LazyAgent>(agents::LazyAgent()),
                new CopyClonable<bboard::Agent, agents::LazyAgent>(agents::LazyAgent()),
                new CopyClonable<bboard::Agent, agents::LazyAgent>(agents::LazyAgent()),
                new CopyClonable<bboard::Agent, agents::LazyAgent>(agents::LazyAgent()),
            };
            break;

        default:
            break;
        }

        pommermanState->set_planning_agents(this->planningAgents);
    }
}

void _get_policy(EvalInfo* info, float* policyProbs) {
    std::fill_n(policyProbs, NUM_MOVES, 0);
    for (size_t i = 0; i < info->legalMoves.size(); i++) {
        Action a = info->legalMoves.at(i);
        float prob = info->policyProbSmall.at(i);

        size_t index = StateConstantsPommerman::action_to_index(a);
        policyProbs[index] = prob;
    }
}

void _get_q(EvalInfo* info, float* q) {
    std::fill_n(q, NUM_MOVES, std::numeric_limits<float>::quiet_NaN());
    for (size_t i = 0; i < info->legalMoves.size(); i++) {
        Action a = info->legalMoves.at(i);
        float qVal = info->qValues.at(i);

        size_t index = StateConstantsPommerman::action_to_index(a);
        q[index] = qVal;
    }
}

void _print_arr(float* arr, int count) {
    if (count <= 0) {
        return;
    }
    for (size_t i = 0; i < count - 1; i++) {
        std::cout << arr[i] << ", ";
    }
    std::cout << arr[count - 1] << std::endl;
}

void _print_q(EvalInfo* info) {
    std::cout << "Q values: ";
    for (size_t i = 0; i < info->legalMoves.size(); i++) {
        Action a = info->legalMoves.at(i);
        float qVal = info->qValues.at(i);

        std::cout << StateConstantsPommerman::action_to_uci(a, false) << ": " << qVal;
        if (i < info->legalMoves.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << std::endl;
}

void CrazyAraAgent::set_outgoing(const bboard::Observation *obs)
{
  int word0;
  int word1;
  if (obs->agents[obs->agentID].y < bboard::BOARD_SIZE / 2) {
    // we are in the upper half of the board
    // set first bit of first word to 0
    word0 &= ~(1UL << 0);
  } else {
    // we are in the lower half of the board
    // set first bit of first word to 1
    word0 &= ~(1UL << 1);
  }
  int enemy1Id = -1;
  int enemy2Id;
  for (int i = 0; i < 4; i++) {
    if (obs->Enemies(obs->agentID, i)) {
      if (enemy1Id == -1){
        enemy1Id = i;
      } else {
        enemy2Id = i;
      }
    }
  }
  int enemy1Quad;
  if (!obs->agents[enemy1Id].visible || obs->agents[enemy1Id].dead) {
    enemy1Quad = 0;
  } else if (obs->agents[enemy1Id].x < bboard::BOARD_SIZE / 2 &&
    obs->agents[enemy1Id].y < bboard::BOARD_SIZE / 2) {
    enemy1Quad = 1;
  } else if (obs->agents[enemy1Id].x >= bboard::BOARD_SIZE / 2 &&
    obs->agents[enemy1Id].y < bboard::BOARD_SIZE / 2) {
    enemy1Quad = 2;
  } else if (obs->agents[enemy1Id].x < bboard::BOARD_SIZE / 2 &&
    obs->agents[enemy1Id].y >= bboard::BOARD_SIZE / 2) {
    enemy1Quad = 3;
  } else {
    enemy1Quad = 4;
  }
  int enemy2Quad;
  if (!obs->agents[enemy2Id].visible || obs->agents[enemy2Id].dead) {
    enemy2Quad = 0;
  } else if (obs->agents[enemy2Id].x < bboard::BOARD_SIZE / 2 &&
    obs->agents[enemy2Id].y < bboard::BOARD_SIZE / 2) {
    enemy2Quad = 1;
  } else if (obs->agents[enemy2Id].x >= bboard::BOARD_SIZE / 2 &&
    obs->agents[enemy2Id].y < bboard::BOARD_SIZE / 2) {
    enemy2Quad = 2;
  } else if (obs->agents[enemy2Id].x < bboard::BOARD_SIZE / 2 &&
    obs->agents[enemy2Id].y >= bboard::BOARD_SIZE / 2) {
    enemy2Quad = 3;
  } else {
    enemy2Quad = 4;
  }
  int enemyPosEncoding = enemy1Quad + 5 * enemy2Quad;
  word1 = enemyPosEncoding % 8;
  if (enemyPosEncoding < 8) {
    word0 &= ~(1UL << 1);
    word0 &= ~(1UL << 2);
  } else if (enemyPosEncoding < 16) {
    word0 |= 1UL << 1;
    word0 &= ~(1UL << 2);
  } else {
    word0 &= ~(1UL << 1);
    word0 |= 1UL << 2;
  }
  SendMessage(word0, word1);
}

bboard::Move CrazyAraAgent::act(const bboard::Observation *obs)
{
    set_outgoing(obs);
    pommermanState->set_observation(obs);
    if (this->incoming) {
        bboard::PythonEnvMessage& twoWordsMsgRef = dynamic_cast<bboard::PythonEnvMessage&>(*(this->incoming));
        pommermanState->set_message(&twoWordsMsgRef);
    } else {
        bboard::PythonEnvMessage* twoWordsMsg = new bboard::PythonEnvMessage(0, 0);
        bboard::PythonEnvMessage& twoWordsMsgRef = *twoWordsMsg;
        pommermanState->set_message(&twoWordsMsgRef);
    }
    agent->set_search_settings(pommermanState.get(), &searchLimits, &evalInfo);
    agent->perform_action();

#ifndef DISABLE_UCI_INFO
    MCTSAgent* mctsAgent = dynamic_cast<MCTSAgent*>(agent.get());
    if (mctsAgent != nullptr) {
        mctsAgent->print_root_node();
    }
#endif

#ifdef CRAZYARA_AGENT_PV
    MCTSAgent* mctsAgent = dynamic_cast<MCTSAgent*>(agent.get());
    if (mctsAgent != nullptr) {
        if (evalInfo.pv.size() > 0) {
            std::cout << "BEGIN ========== Principal Variation" << std::endl;
            pommermanState->set_state(state);
            std::cout << "(" << pommermanState->state.timeStep << "): Initial state" << std::endl;
            bboard::PrintState(&pommermanState->state, false);
            for (Action a : evalInfo.pv[0]) {
                pommermanState->do_action(a);
                std::cout << "(" << pommermanState->state.timeStep << "): after " << StateConstantsPommerman::action_to_uci(a, false) << std::endl;
                bboard::PrintState(&pommermanState->state, false);
            }
            std::cout << "END ========== " << std::endl;
        }
    }
#endif

    bboard::Move bestAction = bboard::Move(agent->get_best_action());

    if (has_buffer()) {
        pommermanState->get_state_planes(true, planeBuffer, singleNet->get_version());
        _get_policy(&evalInfo, policyBuffer);
        _get_q(&evalInfo, qBuffer);

        sampleBuffer->addSample(planeBuffer, bestAction, policyBuffer, evalInfo.bestMoveQ.at(0), qBuffer);
    }

    return bestAction;
}

void CrazyAraAgent::reset() {
    // update ID of the agent of MCTS states
    pommermanState->set_agent_id(id);
}

crazyara::Agent* CrazyAraAgent::get_agent()
{
    return agent.get();
}

std::unique_ptr<NeuralNetAPI> CrazyAraAgent::load_network(const std::string& modelDirectory)
{
#ifdef TENSORRT
    return std::make_unique<TensorrtAPI>(0, 1, modelDirectory, "float32");
#elif defined (TORCH)
    return std::make_unique<TorchAPI>("cpu", 0, 1, modelDirectory);
#endif
}

vector<unique_ptr<NeuralNetAPI>> CrazyAraAgent::load_network_batches(const string& modelDirectory, const SearchSettings& searchSettings)
{
    vector<unique_ptr<NeuralNetAPI>> netBatches;
#ifdef MXNET
    #ifdef TENSORRT
        const bool useTensorRT = bool(Options["Use_TensorRT"]);
    #else
        const bool useTensorRT = false;
    #endif
#endif
    int First_Device_ID = 0;
    int Last_Device_ID = 0;
    for (int deviceId = First_Device_ID; deviceId <= Last_Device_ID; ++deviceId) {
        for (size_t i = 0; i < searchSettings.threads; ++i) {
    #ifdef MXNET
            netBatches.push_back(make_unique<MXNetAPI>(Options["Context"], deviceId, searchSettings.batchSize, modelDirectory, useTensorRT));
    #elif defined TENSORRT
            netBatches.push_back(make_unique<TensorrtAPI>(deviceId, searchSettings.batchSize, modelDirectory, "float16"));
    #elif defined TORCH
            netBatches.push_back(make_unique<TorchAPI>("cpu", deviceId, searchSettings.batchSize, modelDirectory));
    #endif
        }
    }
    netBatches[0]->validate_neural_network();
    return netBatches;
}

SearchSettings CrazyAraAgent::get_default_search_settings(const bool selfPlay)
{
    SearchSettings searchSettings;

    searchSettings.virtualLoss = 1;
    searchSettings.batchSize = 8;
    searchSettings.threads = 2;
    searchSettings.useMCGS = false;
    searchSettings.multiPV = 1;
    searchSettings.nodePolicyTemperature = 1.0f;
    if (selfPlay)
    {
        searchSettings.dirichletEpsilon = 0.25f;
    }
    else
    {
        searchSettings.dirichletEpsilon = 0;
    }
    searchSettings.dirichletAlpha = 0.2f;
    searchSettings.epsilonGreedyCounter = 0;
    searchSettings.epsilonChecksCounter = 0;
    searchSettings.qVetoDelta = 0.4;
    searchSettings.qValueWeight = 1.0f;
    searchSettings.reuseTree = false;
    searchSettings.mctsSolver = false;

    return searchSettings;
}
