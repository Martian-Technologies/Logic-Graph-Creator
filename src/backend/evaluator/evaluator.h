#ifndef evaluator_h
#define evaluator_h

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
    void setPause(bool pause);
    void reset();
    void setTickrate(unsigned long long tickrate);
    void setUseTickrate(bool useTickrate);
    long long int getRealTickrate() const;
    void runNTicks(unsigned long long n);
    void makeEdit(DifferenceSharedPtr difference, block_container_wrapper_id_t containerId);
    logic_state_t getState(const Address& address);
    void setState(const Address& address, logic_state_t state);
    std::vector<logic_state_t> getBulkStates(const std::vector<Address>& addresses);
    std::vector<logic_state_t> getBulkStates(const std::vector<Address>& addresses, const Address& addressOrigin);
    void setBulkStates(const std::vector<Address>& addresses, const std::vector<logic_state_t>& states);
    void setBulkStates(const std::vector<Address>& addresses, const std::vector<logic_state_t>& states, const Address& addressOrigin);

private:
    bool paused;
    bool usingTickrate;
    unsigned long long targetTickrate;
    LogicSimulator logicSimulator;
    AddressTreeNode<block_id_t> addressTree;
};

GateType blockContainerToEvaluatorGatetype(BlockType blockType);

#endif // evaluator_h
