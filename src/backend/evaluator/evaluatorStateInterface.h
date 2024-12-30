#ifndef evaluatorStateInterface_h
#define evaluatorStateInterface_h

#include "evaluator.h"

class EvaluatorStateInterface {
public:
    EvaluatorStateInterface() : evaluator(nullptr) {}
    EvaluatorStateInterface(Evaluator* evaluator) : evaluator(evaluator) { assert(evaluator); }
    
    inline logic_state_t getState(const Address& address) {
        if (evaluator) return evaluator->getState(address); return false;
    }
    inline void setState(const Address& address, logic_state_t state) {
        if (evaluator) evaluator->setState(address, state);
    }
    inline std::vector<logic_state_t> getBulkStates(const std::vector<Address>& addresses) {
        if (evaluator) return evaluator->getBulkStates(addresses); return getEmptyVector<logic_state_t>();
    }
    inline std::vector<logic_state_t> getBulkStates(const std::vector<Address>& addresses, const Address& addressOrigin) {
        if (evaluator) return evaluator->getBulkStates(addresses, addressOrigin); return getEmptyVector<logic_state_t>();
    }
    inline void setBulkStates(const std::vector<Address>& addresses, const std::vector<logic_state_t>& states) {
        if (evaluator) evaluator->setBulkStates(addresses, states);
    }
    inline void setBulkStates(const std::vector<Address>& addresses, const std::vector<logic_state_t>& states, const Address& addressOrigin) {
        if (evaluator) evaluator->setBulkStates(addresses, states, addressOrigin);
    }

private:
    Evaluator* evaluator;
};

#endif /* evaluatorStateInterface_h */
