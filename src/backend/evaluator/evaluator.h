#ifndef evaluator_h
#define evaluator_h

#include "backend/circuit/circuit.h"
#include "backend/container/difference.h"
#include "logicSimulator.h"
#include "addressTree.h"
#include "backend/address.h"
#include "logicState.h"

typedef unsigned int evaluator_id_t;

class Evaluator {
public:
	Evaluator(evaluator_id_t evaluatorId, SharedCircuit circuit);

	inline evaluator_id_t getEvaluatorId() const { return evaluatorId; }

	// pause/unpause used once the evaluator is "started" 
	void setPause(bool pause);
	void reset();
	void setTickrate(unsigned long long tickrate);
	void setUseTickrate(bool useTickrate);
	long long int getRealTickrate() const;
	void runNTicks(unsigned long long n);
	void makeEdit(DifferenceSharedPtr difference, circuit_id_t circuitId);
	logic_state_t getState(const Address& address);
	void setState(const Address& address, logic_state_t state);
	std::vector<logic_state_t> getBulkStates(const std::vector<Address>& addresses);
	std::vector<logic_state_t> getBulkStates(const std::vector<Address>& addresses, const Address& addressOrigin);
	void setBulkStates(const std::vector<Address>& addresses, const std::vector<logic_state_t>& states);
	void setBulkStates(const std::vector<Address>& addresses, const std::vector<logic_state_t>& states, const Address& addressOrigin);

private:
	evaluator_id_t evaluatorId;
	bool paused;
	bool usingTickrate;
	unsigned long long targetTickrate;
	LogicSimulator logicSimulator;
	AddressTreeNode<block_id_t> addressTree;
};

GateType circuitToEvaluatorGatetype(BlockType blockType);

typedef std::shared_ptr<Evaluator> SharedEvaluator;

#endif // evaluator_h
