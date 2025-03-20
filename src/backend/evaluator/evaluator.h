#ifndef evaluator_h
#define evaluator_h

#include "backend/circuit/circuit.h"
#include "backend/circuit/circuitManager.h"
#include "backend/container/difference.h"
#include "logicSimulator.h"
#include "addressTree.h"
#include "backend/address.h"
#include "logicState.h"

typedef unsigned int evaluator_id_t;

class Evaluator {
public:
	Evaluator(evaluator_id_t evaluatorId, CircuitManager& circuitManager, circuit_id_t circuitId);

	inline evaluator_id_t getEvaluatorId() const { return evaluatorId; }
	std::string getEvaluatorName() const { return "Evaluator " + std::to_string(evaluatorId) + " (Circuit: " + std::to_string(addressTree.getContainerId()) + ")"; }

	circuit_id_t getCircuitId(const Address& address) { return addressTree.getBranch(address).getContainerId(); }

	void setPause(bool pause);
	void reset();
	void setTickrate(unsigned long long tickrate);
	void setUseTickrate(bool useTickrate);
	long long int getRealTickrate() const;
	void runNTicks(unsigned long long n);
	void makeEdit(DifferenceSharedPtr difference, circuit_id_t circuitId);
	void makeEditInPlace(DifferenceSharedPtr difference, circuit_id_t circuitId, AddressTreeNode<block_id_t>& addressTree);
	logic_state_t getState(const Address& address);
	bool getBoolState(const Address& address);
	void setState(const Address& address, logic_state_t state);
	void setState(const Address& address, bool state) { setState(address, fromBool(state)); }
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
	CircuitManager& circuitManager;
};

GateType circuitToEvaluatorGatetype(BlockType blockType);

typedef std::shared_ptr<Evaluator> SharedEvaluator;

#endif // evaluator_h
