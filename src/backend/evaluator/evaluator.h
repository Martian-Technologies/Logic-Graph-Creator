#ifndef evaluator_h
#define evaluator_h

#include "../container/difference.h"
#include "../middleEnd/blockContainerWrapper.h"
#include "../address.h"
#include "logicSimulator.h"
#include "logicState.h"
#include <vector>
#include <unordered_map>
#include "addressTree.h"

class Evaluator {
public:
    Evaluator(std::shared_ptr<BlockContainerWrapper> blockContainerWrapper);
    // start/stop used for lock control for threading
    void stop();
    void start();
    void start(unsigned long long tickrate);
    // pause/unpause used once the evaluator is "started" 
    void pause();
    void unpause();
    void reset();
    void setTickrate(unsigned long long tickrate);
    void runNTicks(unsigned long long n);
    void makeEdit(DifferenceSharedPtr difference);
    logic_state_t getState(const Address& address) const;
    void setState(const Address& address, logic_state_t state);
    std::vector<logic_state_t> getBulkStates(const std::vector<Address>& addresses) const;
    std::vector<logic_state_t> getBulkStates(const std::vector<Address>& addresses, const Address& addressOrigin) const;
    void setBulkStates(const std::vector<Address>& addresses, const std::vector<logic_state_t>& states);
    void setBulkStates(const std::vector<Address>& addresses, const std::vector<logic_state_t>& states, const Address& addressOrigin);

private:
    bool running;
    bool paused;
    unsigned long long targetTickrate;
    LogicSimulator logicSimulator;
    AddressTree<block_id_t> addressTree;
};

#endif // evaluator_h