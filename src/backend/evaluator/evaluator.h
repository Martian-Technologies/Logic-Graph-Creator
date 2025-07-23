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
#include "evalSimulator.h"

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
		auto circuit = circuitManager.getCircuit(circuitId.value());
		if (!circuit) {
			return "Eval " + std::to_string(evaluatorId) + " (Invalid Circuit)";
		}
		return "Eval " + std::to_string(evaluatorId) + " (" + circuit->getCircuitNameNumber() + ")";
	}

	void reset();
	void setPause(bool pause) { logWarning("not implemented yet", "Evaluator::setPause"); };
	bool isPause() const { return !evalConfig.running; }
	void setTickrate(unsigned long long tickrate) { evalConfig.targetTickrate = tickrate; }
	unsigned long long getTickrate() const { return evalConfig.targetTickrate; }
	void setUseTickrate(bool useTickrate) { evalConfig.tickrateLimiter = useTickrate; }
	bool getUseTickrate() const { return evalConfig.tickrateLimiter; }
	long long int getRealTickrate() const { logWarning("not implemented yet", "Evaluator::getRealTickrate"); return 0; };
	void makeEdit(DifferenceSharedPtr difference, circuit_id_t circuitId);
	logic_state_t getState(const Address& address) {
		logWarning("not implemented yet", "Evaluator::getState");
		return logic_state_t::UNDEFINED;
	};
	bool getBoolState(const Address& address) {
		logWarning("not implemented yet", "Evaluator::getBoolState");
		return false;
	};
	void setState(const Address& address, logic_state_t state) {
		logWarning("not implemented yet", "Evaluator::setState");
	};
	void setState(const Address& address, bool state) { setState(address, fromBool(state)); }
	std::vector<logic_state_t> getBulkStates(const std::vector<Address>& addresses) {
		// logWarning("not implemented yet", "Evaluator::getBulkStates");
		return std::vector<logic_state_t>(addresses.size(), logic_state_t::UNDEFINED);
	};
	std::vector<logic_state_t> getBulkStates(const std::vector<Address>& addresses, const Address& addressOrigin) {
		// logWarning("not implemented yet", "Evaluator::getBulkStates");
		return std::vector<logic_state_t>(addresses.size(), logic_state_t::UNDEFINED);
	};
	void setBulkStates(const std::vector<Address>& addresses, const std::vector<logic_state_t>& states) {
		logWarning("not implemented yet", "Evaluator::setBulkStates");
	};
	void setBulkStates(const std::vector<Address>& addresses, const std::vector<logic_state_t>& states, const Address& addressOrigin) {
		logWarning("not implemented yet", "Evaluator::setBulkStates");
	};
	circuit_id_t getCircuitId() const {
		return evalCircuitContainer.getCircuitId(0).value_or(0);
	}
	circuit_id_t getCircuitId(const Address& address) const {
		eval_circuit_id_t evalCircuitId = 0;
		for (int i = 0; i < address.size(); i++) {
			std::optional<CircuitNode> node = evalCircuitContainer.getNode(address.getPosition(i), evalCircuitId);
			if (!node.has_value()) {
				logError("CircuitNode not found for address {}", "Evaluator::getCircuitId", "Evaluator::getCircuitId", address.toString());
				return getCircuitId(); // Invalid circuit ID
			}
			if (node->isIC()) {
				evalCircuitId = node->getId();
			} else {
				logError("Address {} does not point to an IC", "Evaluator::getCircuitId", address.toString());
				return getCircuitId();
			}
		}
		return evalCircuitContainer.getCircuitId(evalCircuitId).value_or(0);
	}
	const EvalAddressTree buildAddressTree() const;
	const EvalAddressTree buildAddressTree(eval_circuit_id_t evalCircuitId) const;

private:

	evaluator_id_t evaluatorId;
	CircuitManager& circuitManager;
	DataUpdateEventManager::DataUpdateEventReceiver receiver;
	EvalCircuitContainer evalCircuitContainer;
	EvalConfig evalConfig;
	IdProvider<middle_id_t> middleIdProvider;
	EvalSimulator evalSimulator;

	void makeEditInPlace(eval_circuit_id_t evalCircuitId, DifferenceSharedPtr difference, DiffCache& diffCache);

	void edit_removeBlock(eval_circuit_id_t evalCircuitId, DiffCache& diffCache, Position position, Rotation rotation, BlockType type);
	void edit_placeBlock(eval_circuit_id_t evalCircuitId, DiffCache& diffCache, Position position, Rotation rotation, BlockType type);
	void edit_removeConnection(eval_circuit_id_t evalCircuitId, DiffCache& diffCache, Position outputBlockPosition, Position outputPosition, Position inputBlockPosition, Position inputPosition);
	void edit_createConnection(eval_circuit_id_t evalCircuitId, DiffCache& diffCache, Position outputBlockPosition, Position outputPosition, Position inputBlockPosition, Position inputPosition);
	void edit_moveBlock(eval_circuit_id_t evalCircuitId, DiffCache& diffCache, Position curPosition, Rotation curRotation, Position newPosition, Rotation newRotation);
};

typedef std::shared_ptr<Evaluator> SharedEvaluator;

#endif // evaluator_h
