#ifndef evaluator_h
#define evaluator_h

#include "backend/circuit/circuit.h"
#include "backend/circuit/circuitManager.h"
#include "backend/container/difference.h"
#include "backend/dataUpdateEventManager.h"

#include "backend/address.h"
#include "logicState.h"
#include "diffCache.h"

typedef unsigned int evaluator_id_t;

class DataUpdateEventManager;

class Evaluator {
public:
	typedef std::pair<BlockType, connection_end_id_t> RemoveCircuitIOData;
	struct EvaluatorGate {
		wrapper_gate_id_t gateId;
		BlockType blockType;
		Rotation rotation;
	};

	Evaluator(evaluator_id_t evaluatorId, CircuitManager& circuitManager, circuit_id_t circuitId, DataUpdateEventManager* dataUpdateEventManager);

	inline evaluator_id_t getEvaluatorId() const { return evaluatorId; }
	std::string getEvaluatorName() const { return "Eval " + std::to_string(evaluatorId) + " (" + circuitManager.getCircuit(addressTree.getContainerId())->getCircuitNameNumber() + ")"; }

	void reset();
	void setPause(bool pause);
	bool isPause() const { return paused; }
	void setTickrate(unsigned long long tickrate);
	unsigned long long getTickrate() const { return targetTickrate; }
	void setUseTickrate(bool useTickrate);
	bool getUseTickrate() const { return usingTickrate; }
	long long int getRealTickrate() const;
	void runNTicks(unsigned long long n);
	void makeEdit(DifferenceSharedPtr difference, circuit_id_t circuitId);
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
	CircuitManager& circuitManager;
	DataUpdateEventManager::DataUpdateEventReceiver receiver;

	void makeEditInPlace(DifferenceSharedPtr difference, circuit_id_t circuitId, AddressTreeNode<EvaluatorGate>& addressTree, DiffCache& diffCache, bool insideIC);
};

GateType circuitToEvaluatorGatetype(BlockType blockType, bool insideIC);

typedef std::shared_ptr<Evaluator> SharedEvaluator;

#endif // evaluator_h
