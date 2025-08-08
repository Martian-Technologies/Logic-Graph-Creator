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
#include "directionEnum.h"

typedef unsigned int evaluator_id_t;

enum class SimulatorMappingUpdateType {
	BLOCK,
	PIN
};

struct SimulatorMappingUpdate {
	Position portPosition;
	simulator_id_t simulatorId;
	SimulatorMappingUpdateType type;
};

typedef std::function<void(const std::vector<SimulatorMappingUpdate>&)> SimulatorMappingUpdateListenerFunction;

struct SimulatorMappingUpdateListener {
	eval_circuit_id_t evalCircuitId;
	std::function<void(const std::vector<SimulatorMappingUpdate>&)> callback;
};

class DataUpdateEventManager;

struct CircuitPortDependency {
	circuit_id_t circuitId;
	connection_end_id_t connectionEndId;
	auto operator<=>(const CircuitPortDependency& other) const {
		return std::tie(circuitId, connectionEndId) <=> std::tie(other.circuitId, other.connectionEndId);
	}
};

struct InterCircuitConnection {
	EvalConnection connection;
	std::set<CircuitPortDependency> circuitPortDependencies;
	std::set<CircuitNode> circuitNodeDependencies;
};

struct DependentConnectionPoint {
	EvalConnectionPoint connectionPoint;
	std::set<CircuitPortDependency> circuitPortDependencies;
	std::set<CircuitNode> circuitNodeDependencies;
};

class Evaluator {
public:
	typedef std::tuple<BlockType, connection_end_id_t, Position> RemoveCircuitIOData; // I hate tuples, but this is how I get the data
	typedef std::pair<BlockType, connection_end_id_t> SetCircuitIOData;

	Evaluator(
		evaluator_id_t evaluatorId,
		CircuitManager& circuitManager,
		BlockDataManager& blockDataManager,
		CircuitBlockDataManager& circuitBlockDataManager,
		circuit_id_t circuitId,
		DataUpdateEventManager* dataUpdateEventManager
	);

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
	void setPause(bool pause) { evalConfig.setRunning(!pause); }
	bool isPause() const { return !evalConfig.isRunning(); }
	void setRealistic(bool realistic) { evalConfig.setRealistic(realistic); }
	bool isRealistic() const { return evalConfig.isRealistic(); }
	void setTickrate(unsigned long long tickrate) { evalConfig.setTargetTickrate(tickrate); }
	unsigned long long getTickrate() const { return evalConfig.getTargetTickrate(); }
	void setUseTickrate(bool useTickrate) { evalConfig.setTickrateLimiter(useTickrate); }
	bool getUseTickrate() const { return evalConfig.isTickrateLimiterEnabled(); }
	float getRealTickrate() const { return evalSimulator.getAverageTickrate(); }
	void makeEdit(DifferenceSharedPtr difference, circuit_id_t circuitId);
	logic_state_t getState(const Address& address);
	bool getBoolState(const Address& address) {
		return toBool(getState(address));
	};
	void setState(const Address& address, logic_state_t state);
	void setState(const Address& address, bool state) {
		setState(address, fromBool(state));
	}
	std::vector<logic_state_t> getBulkStates(const std::vector<Address>& addresses) {
		logWarning("not implemented yet", "Evaluator::getBulkStates");
		return std::vector<logic_state_t>(addresses.size(), logic_state_t::UNDEFINED);
	};
	std::vector<logic_state_t> getBulkStates(const std::vector<Address>& addresses, const Address& addressOrigin);
	std::vector<logic_state_t> getBulkStates(const std::vector<Position>& positions, const Address& addressOrigin);
	std::vector<logic_state_t> getBulkPinStates(const std::vector<Position>& positions, const Address& addressOrigin);
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
		std::shared_lock lk(simMutex);
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

	std::vector<simulator_id_t> getBlockSimulatorIds(const Address& addressOrigin, const std::vector<Position>& positions) const;
	std::vector<simulator_id_t> getPinSimulatorIds(const Address& addressOrigin, const std::vector<Position>& positions) const;
	std::vector<logic_state_t> getStatesFromSimulatorIds(const std::vector<simulator_id_t>& simulatorIds) const;

	void connectListener(
		void* object,
		const Address& address,
		SimulatorMappingUpdateListenerFunction func
	) {
		std::optional<eval_circuit_id_t> evalCircuitId = evalCircuitContainer.traverseToTopLevelIC(address);
		if (!evalCircuitId) {
			logError("Failed to connect listener for address {}: No top-level IC found", "Evaluator::connectListener", address.toString());
			return;
		}
		listeners[object] = { evalCircuitId.value(), func };
	}
	void disconnectListener(void* object) {
		auto iter = listeners.find(object);
		if (iter != listeners.end()) {
			listeners.erase(iter);
		}
	}

private:
	evaluator_id_t evaluatorId;
	CircuitManager& circuitManager;
	BlockDataManager& blockDataManager;
	CircuitBlockDataManager& circuitBlockDataManager;
	DataUpdateEventManager* dataUpdateEventManager;
	DataUpdateEventManager::DataUpdateEventReceiver receiver;
	EvalCircuitContainer evalCircuitContainer;
	EvalConfig evalConfig;
	IdProvider<middle_id_t> middleIdProvider;
	EvalSimulator evalSimulator;

	bool changedICs = false;

	void makeEditInPlace(SimPauseGuard& pauseGuard, eval_circuit_id_t evalCircuitId, DifferenceSharedPtr difference, DiffCache& diffCache);

	void edit_removeBlock(SimPauseGuard& pauseGuard, eval_circuit_id_t evalCircuitId, DiffCache& diffCache, Position position, Orientation orientation, BlockType type);
	void edit_deleteICContents(SimPauseGuard& pauseGuard, eval_circuit_id_t evalCircuitId);
	void edit_placeBlock(SimPauseGuard& pauseGuard, eval_circuit_id_t evalCircuitId, DiffCache& diffCache, Position position, Orientation orientation, BlockType type);
	void edit_placeIC(SimPauseGuard& pauseGuard, eval_circuit_id_t evalCircuitId, DiffCache& diffCache, Position position, Orientation orientation, circuit_id_t circuitId);
	void edit_removeConnection(SimPauseGuard& pauseGuard, eval_circuit_id_t evalCircuitId, DiffCache& diffCache, const BlockContainer* blockContainer, Position outputBlockPosition, Position outputPosition, Position inputBlockPosition, Position inputPosition);
	void edit_createConnection(SimPauseGuard& pauseGuard, eval_circuit_id_t evalCircuitId, DiffCache& diffCache, const BlockContainer* blockContainer, Position outputBlockPosition, Position outputPosition, Position inputBlockPosition, Position inputPosition);
	void edit_moveBlock(SimPauseGuard& pauseGuard, eval_circuit_id_t evalCircuitId, DiffCache& diffCache, Position curPosition, Orientation curOrientation, Position newPosition, Orientation newOrientation);

	void removeDependentInterCircuitConnections(SimPauseGuard& pauseGuard, CircuitPortDependency circuitPortDependency);
	void removeDependentInterCircuitConnections(SimPauseGuard& pauseGuard, CircuitNode node);
	void removeCircuitIO(const DataUpdateEventManager::EventData* data);
	void setCircuitIO(const DataUpdateEventManager::EventData* data);

	std::optional<middle_id_t> getMiddleId(const eval_circuit_id_t startingPoint, const Address& address) const;
	std::optional<middle_id_t> getMiddleId(const eval_circuit_id_t startingPoint, const Address& address, const BlockContainer* blockContainer) const;
	std::optional<middle_id_t> getMiddleId(const Address& address) const;

	std::optional<connection_port_id_t> getPortId(const circuit_id_t circuitId, const Position blockPosition, const Position portPosition, Direction direction) const;
	std::optional<connection_port_id_t> getPortId(const BlockContainer* blockContainer, const Position blockPosition, const Position portPosition, Direction direction) const;
	std::optional<EvalConnectionPoint> getConnectionPoint(const eval_circuit_id_t evalCircuitId, const Position portPosition, Direction direction) const;
	std::optional<EvalConnectionPoint> getConnectionPoint(const eval_circuit_id_t evalCircuitId, const BlockContainer* blockContainer, const Position portPosition, Direction direction) const;
	std::optional<EvalConnectionPoint> getConnectionPoint(
		const eval_circuit_id_t evalCircuitId,
		const Position portPosition,
		Direction direction,
		std::set<CircuitPortDependency>& circuitPortDependencies,
		std::set<CircuitNode>& circuitNodeDependencies
	) const;
	std::optional<EvalConnectionPoint> getConnectionPoint(
		const eval_circuit_id_t evalCircuitId,
		const BlockContainer* blockContainer,
		const Position portPosition,
		Direction direction,
		std::set<CircuitPortDependency>& circuitPortDependencies,
		std::set<CircuitNode>& circuitNodeDependencies
	) const;

	std::vector<InterCircuitConnection> interCircuitConnections;
	void checkToCreateExternalConnections(SimPauseGuard& pauseGuard, eval_circuit_id_t evalCircuitId, Position position);
	void traceOutwardsIC(
		SimPauseGuard& pauseGuard,
		eval_circuit_id_t evalCircuitId,
		Position position,
		Direction direction,
		const EvalConnectionPoint& targetConnectionPoint,
		std::set<CircuitPortDependency>& circuitPortDependencies,
		std::set<CircuitNode>& circuitNodeDependencies
	);
	std::vector<simulator_id_t> dirtySimulatorIds;
	std::unordered_set<EvalPosition> dirtyNodes;
	std::unordered_multimap<simulator_id_t, EvalPosition> portSimulatorIdToEvalPositionMap;
	std::unordered_multimap<simulator_id_t, EvalPosition> pinSimulatorIdToEvalPositionMap;

	std::map<void*, SimulatorMappingUpdateListener> listeners;
	void sendSimulatorMappingUpdate(eval_circuit_id_t targetEvalCircuitId, const std::vector<SimulatorMappingUpdate>& updates) {
		for (const auto& listener : listeners) {
			if (listener.second.evalCircuitId == targetEvalCircuitId) {
				listener.second.callback(updates);
			}
		}
	}

private:
	void processDirtyNodes();
	void dirtyBlockAt(Position position, eval_circuit_id_t evalCircuitId);

	mutable std::shared_mutex simMutex;
};

typedef std::shared_ptr<Evaluator> SharedEvaluator;

#endif /* evaluator_h */
