#ifndef evaluator_h
#define evaluator_h

#include <unordered_map>
#include <vector>

#include "middleEnd/blockContainerWrapper.h"
#include "../container/difference.h"
#include "logicSimulator.h"
#include "addressTree.h"
#include "../address.h"
#include "logicState.h"

class Evaluator {
public:
    Evaluator(std::shared_ptr<BlockContainerWrapper> blockContainerWrapper);
    // pause/unpause used once the evaluator is "started" 
    void pause();
    void unpause();
    void reset();
    void setTickrate(unsigned long long tickrate);
    void runNTicks(unsigned long long n);
    void makeEdit(DifferenceSharedPtr difference, block_container_wrapper_id_t containerId);
    logic_state_t getState(const Address& address) const;
    void setState(const Address& address, logic_state_t state);
    std::vector<logic_state_t> getBulkStates(const std::vector<Address>& addresses) const;
    std::vector<logic_state_t> getBulkStates(const std::vector<Address>& addresses, const Address& addressOrigin) const;
    void setBulkStates(const std::vector<Address>& addresses, const std::vector<logic_state_t>& states);
    void setBulkStates(const std::vector<Address>& addresses, const std::vector<logic_state_t>& states, const Address& addressOrigin);

private:
    bool paused;
    unsigned long long targetTickrate;
    LogicSimulator logicSimulator;
    AddressTreeNode<block_id_t> addressTree;
};

GateType blockContainerToEvaluatorGatetype(BlockType blockType);

#endif // evaluator_h