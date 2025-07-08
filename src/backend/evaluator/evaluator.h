#ifndef evaluator_h
#define evaluator_h

#include "backend/circuit/circuit.h"
#include "backend/circuit/circuitManager.h"
#include "backend/container/difference.h"
#include "backend/dataUpdateEventManager.h"

#include "backend/address.h"
#include "logicState.h"
#include "diffCache.h"
#include "evalCircuitContainer.h"
#include "evalConfig.h"
#include "evalAddressTree.h"
#include "logicSimulator.h"

typedef unsigned int evaluator_id_t;

class DataUpdateEventManager;

class Evaluator {
public:
	typedef std::pair<BlockType, connection_end_id_t> RemoveCircuitIOData; // I hate pairs, but this is how I get the data

	Evaluator(evaluator_id_t evaluatorId, CircuitManager& circuitManager, circuit_id_t circuitId, DataUpdateEventManager* dataUpdateEventManager);

	inline evaluator_id_t getEvaluatorId() const { return evaluatorId; }
	std::string getEvaluatorName() const {
		std::optional<circuit_id_t> circuitId = evalCircuitContainer.getCircuitId(0);
		if (!circuitId.has_value()) {
			return "Eval " + std::to_string(evaluatorId) + " (No Circuit)";
		}
		return "Eval " + std::to_string(evaluatorId) + " (" + circuitManager.getCircuit(circuitId.value())->getCircuitNameNumber() + ")";
	}

	void reset();
	void setPause(bool pause);
	bool isPause() const { return !evalConfig.running; }
	void setTickrate(unsigned long long tickrate) { evalConfig.targetTickrate = tickrate; }
	unsigned long long getTickrate() const { return evalConfig.targetTickrate; }
	void setUseTickrate(bool useTickrate) { evalConfig.tickrateLimiter = useTickrate; }
	bool getUseTickrate() const { return evalConfig.tickrateLimiter; }
	long long int getRealTickrate() const;
	void makeEdit(DifferenceSharedPtr difference, circuit_id_t circuitId);
	logic_state_t getState(const Address& address);
	bool getBoolState(const Address& address);
	void setState(const Address& address, logic_state_t state);
	void setState(const Address& address, bool state) { setState(address, fromBool(state)); }
	std::vector<logic_state_t> getBulkStates(const std::vector<Address>& addresses);
	std::vector<logic_state_t> getBulkStates(const std::vector<Address>& addresses, const Address& addressOrigin);
	void setBulkStates(const std::vector<Address>& addresses, const std::vector<logic_state_t>& states);
	void setBulkStates(const std::vector<Address>& addresses, const std::vector<logic_state_t>& states, const Address& addressOrigin);
	circuit_id_t getCircuitId() const {
		return evalCircuitContainer.getCircuitId(0).value_or(0);
	}
	circuit_id_t getCircuitId(const Address& address) const;
	const EvalAddressTree buildAddressTree() const;
	const EvalAddressTree buildAddressTree(eval_circuit_id_t evalCircuitId) const;

private:

	evaluator_id_t evaluatorId;
	CircuitManager& circuitManager;
	DataUpdateEventManager::DataUpdateEventReceiver receiver;
	EvalCircuitContainer evalCircuitContainer;
	EvalConfig evalConfig;

	void makeEditInPlace(eval_circuit_id_t evalCircuitId, DifferenceSharedPtr difference, DiffCache& diffCache);

	void edit_removeBlock(eval_circuit_id_t evalCircuitId, DiffCache& diffCache, Position position, Rotation rotation, BlockType type);
	void edit_placeBlock(eval_circuit_id_t evalCircuitId, DiffCache& diffCache, Position position, Rotation rotation, BlockType type);
	void edit_removeConnection(eval_circuit_id_t evalCircuitId, DiffCache& diffCache, Position outputBlockPosition, Position outputPosition, Position inputBlockPosition, Position inputPosition);
	void edit_createConnection(eval_circuit_id_t evalCircuitId, DiffCache& diffCache, Position outputBlockPosition, Position outputPosition, Position inputBlockPosition, Position inputPosition);
	void edit_moveBlock(eval_circuit_id_t evalCircuitId, DiffCache& diffCache, Position curPosition, Rotation curRotation, Position newPosition, Rotation newRotation);
	void edit_setData(eval_circuit_id_t evalCircuitId, DiffCache& diffCache, Position position, block_data_t newData, block_data_t oldData);
};

typedef std::shared_ptr<Evaluator> SharedEvaluator;

#endif // evaluator_h
