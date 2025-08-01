#include "evaluator.h"

#ifdef TRACY_PROFILER
	#include <tracy/Tracy.hpp>
#endif

Evaluator::Evaluator(
	evaluator_id_t evaluatorId,
	CircuitManager& circuitManager,
	BlockDataManager& blockDataManager,
	CircuitBlockDataManager& circuitBlockDataManager,
	circuit_id_t circuitId,
	DataUpdateEventManager* dataUpdateEventManager
) : evaluatorId(evaluatorId),
	circuitManager(circuitManager),
	blockDataManager(blockDataManager),
	circuitBlockDataManager(circuitBlockDataManager),
	evalCircuitContainer(),
	dataUpdateEventManager(dataUpdateEventManager),
	receiver(dataUpdateEventManager),
	evalConfig(),
	middleIdProvider(),
	evalSimulator(evalConfig, middleIdProvider) {
	const auto circuit = circuitManager.getCircuit(circuitId);
	if (!circuit) {
		logError("Circuit with ID {} not found", "Evaluator::Evaluator", circuitId);
		return;
	}
	logInfo("Creating Evaluator with ID {} for Circuit ID {}", "Evaluator", evaluatorId, circuitId);
	evalCircuitContainer.addCircuit(0, circuitId);
	const auto blockContainer = circuit->getBlockContainer();
	const Difference difference = blockContainer->getCreationDifference();
	receiver.linkFunction("circuitBlockDataConnectionPositionRemove", std::bind(&Evaluator::removeCircuitIO, this, std::placeholders::_1));
	receiver.linkFunction("circuitBlockDataConnectionPositionSet", std::bind(&Evaluator::setCircuitIO, this, std::placeholders::_1));

	makeEdit(std::make_shared<Difference>(difference), circuitId);
}

void Evaluator::makeEdit(DifferenceSharedPtr difference, circuit_id_t circuitId) {
	#ifdef TRACY_PROFILER
		ZoneScoped;
	#endif
	changedICs = false;
	// logInfo("_________________________________________________________________________________________");
	// logInfo("Applying edit to Evaluator with ID {} for Circuit ID {}", "Evaluator::makeEdit", evaluatorId, circuitId);
	{
		SimPauseGuard pauseGuard = evalSimulator.beginEdit();
		std::unique_lock lk(simMutex);
		DiffCache diffCache(circuitManager);
		for (eval_circuit_id_t evalCircuitId = 0; evalCircuitId < evalCircuitContainer.size(); evalCircuitId++) {
			if (evalCircuitContainer.getCircuitId(evalCircuitId) == circuitId) {
				makeEditInPlace(pauseGuard, evalCircuitId, difference, diffCache);
			}
		}
		evalSimulator.endEdit(pauseGuard);
	}
	if (changedICs) {
		dataUpdateEventManager->sendEvent("addressTreeMakeBranch");
	}
}

void Evaluator::makeEditInPlace(SimPauseGuard& pauseGuard, eval_circuit_id_t evalCircuitId, DifferenceSharedPtr difference, DiffCache& diffCache) {
	#ifdef TRACY_PROFILER
		ZoneScoped;
	#endif
	std::optional<eval_circuit_id_t> circuitId = evalCircuitContainer.getCircuitId(evalCircuitId);
	if (!circuitId.has_value()) {
		logError("EvalCircuit with id {} not found", "Evaluator::makeEditInPlace", evalCircuitId);
		return;
	}
	SharedCircuit circuit = circuitManager.getCircuit(circuitId.value());
	if (!circuit) {
		logError("Circuit with id {} not found", "Evaluator::makeEditInPlace", circuitId.value());
		return;
	}
	const BlockContainer* blockContainer = circuit->getBlockContainer();
	if (!blockContainer) {
		logError("BlockContainer not found", "Evaluator::makeEditInPlace");
		return;
	}
	
	const std::vector<Difference::Modification>& modifications = difference->getModifications();
	for (const Difference::Modification& modification : modifications) {
		const auto& [modificationType, modificationData] = modification;
		switch (modificationType) {
			case Difference::ModificationType::REMOVED_BLOCK: {
				const auto& [position, rotation, blockType] = std::get<Difference::block_modification_t>(modificationData);
				edit_removeBlock(pauseGuard, evalCircuitId, diffCache, position, rotation, blockType);
				break;
			}
			case Difference::ModificationType::PLACE_BLOCK: {
				const auto& [position, rotation, blockType] = std::get<Difference::block_modification_t>(modificationData);
				edit_placeBlock(pauseGuard, evalCircuitId, diffCache, position, rotation, blockType);
				break;
			}
			case Difference::ModificationType::MOVE_BLOCK: {
				const auto& [curPosition, curRotation, newPosition, newRotation] = std::get<Difference::move_modification_t>(modificationData);
				edit_moveBlock(pauseGuard, evalCircuitId, diffCache, curPosition, curRotation, newPosition, newRotation);
				break;
			}
			case Difference::ModificationType::REMOVED_CONNECTION: {
				const auto& [outputBlockPosition, outputPosition, inputBlockPosition, inputPosition] = std::get<Difference::connection_modification_t>(modificationData);
				edit_removeConnection(pauseGuard, evalCircuitId, diffCache, blockContainer, outputBlockPosition, outputPosition, inputBlockPosition, inputPosition);
				break;
			}
			case Difference::ModificationType::CREATED_CONNECTION: {
				const auto& [outputBlockPosition, outputPosition, inputBlockPosition, inputPosition] = std::get<Difference::connection_modification_t>(modificationData);
				edit_createConnection(pauseGuard, evalCircuitId, diffCache, blockContainer, outputBlockPosition, outputPosition, inputBlockPosition, inputPosition);
				break;
			}
		}
	}
}

void Evaluator::edit_removeBlock(SimPauseGuard& pauseGuard, eval_circuit_id_t evalCircuitId, DiffCache& diffCache, Position position, Rotation rotation, BlockType type) {
	#ifdef TRACY_PROFILER
		ZoneScoped;
	#endif
	// Find the circuit and remove the block
	EvalCircuit* evalCircuit = evalCircuitContainer.getCircuit(evalCircuitId);
	if (!evalCircuit) {
		logError("EvalCircuit with id {} not found", "Evaluator::edit_removeBlock", evalCircuitId);
		return;
	}
	std::optional<CircuitNode> node = evalCircuit->getNode(position);
	if (!node.has_value()) {
		logError("Node at position {} not found", "Evaluator::edit_removeBlock", position.toString());
		return;
	}
	if (node->isIC()) {
		edit_deleteICContents(pauseGuard, node->getId());
		evalCircuit->removeNode(position);
		return;
	}
	evalSimulator.removeGate(pauseGuard, node->getId());
	middleIdProvider.releaseId(node->getId());
	evalCircuit->removeNode(position);
}

void Evaluator::edit_deleteICContents(SimPauseGuard& pauseGuard, eval_circuit_id_t evalCircuitId) {
	#ifdef TRACY_PROFILER
		ZoneScoped;
	#endif
	changedICs = true;
	EvalCircuit* evalCircuit = evalCircuitContainer.getCircuit(evalCircuitId);
	if (!evalCircuit) {
		logError("EvalCircuit with id {} not found", "Evaluator::edit_deleteIC", evalCircuitId);
		return;
	}
	evalCircuit->forEachNode([&](Position pos, const CircuitNode& node) {
		if (node.isIC()) {
			edit_deleteICContents(pauseGuard, node.getId());
			return;
		}
		evalSimulator.removeGate(pauseGuard, node.getId());
		middleIdProvider.releaseId(node.getId());
	});
	evalCircuitContainer.removeCircuit(evalCircuitId);
}

void Evaluator::edit_placeBlock(SimPauseGuard& pauseGuard, eval_circuit_id_t evalCircuitId, DiffCache& diffCache, Position position, Rotation rotation, BlockType type) {
	#ifdef TRACY_PROFILER
		ZoneScoped;
	#endif
	GateType gateType = GateType::NONE;
	switch (type) {
	case BlockType::AND: gateType = GateType::AND; break;
	case BlockType::OR: gateType = GateType::OR; break;
	case BlockType::XOR: gateType = GateType::XOR; break;
	case BlockType::NAND: gateType = GateType::NAND; break;
	case BlockType::NOR: gateType = GateType::NOR; break;
	case BlockType::XNOR: gateType = GateType::XNOR; break;
	case BlockType::JUNCTION: gateType = GateType::JUNCTION; break;
	case BlockType::TRISTATE_BUFFER: gateType = GateType::TRISTATE_BUFFER; break;
	case BlockType::BUTTON: gateType = GateType::DUMMY_INPUT; break;
	case BlockType::SWITCH: gateType = GateType::DUMMY_INPUT; break;
	case BlockType::TICK_BUTTON: gateType = GateType::TICK_INPUT; break;
	case BlockType::CONSTANT: gateType = GateType::CONSTANT_ON; break;
	case BlockType::LIGHT: gateType = GateType::JUNCTION; break;
	}
	if (gateType == GateType::NONE) {
		const circuit_id_t ICId = circuitBlockDataManager.getCircuitId(type);
		if (ICId != 0) {
			edit_placeIC(pauseGuard, evalCircuitId, diffCache, position, rotation, ICId);
			return;
		}
		logError("Unsupported BlockType {}", "Evaluator::edit_placeBlock", type);
		return;
	}
	middle_id_t gateId = middleIdProvider.getNewId();
	evalSimulator.addGate(pauseGuard, gateType, gateId);
	EvalCircuit* evalCircuit = evalCircuitContainer.getCircuit(evalCircuitId);
	if (!evalCircuit) {
		logError("EvalCircuit with id {} not found", "Evaluator::edit_placeBlock", evalCircuitId);
		return;
	}
	CircuitNode node = CircuitNode::fromMiddle(gateId);
	evalCircuit->setNode(position, node);
	checkToCreateExternalConnections(pauseGuard, evalCircuitId, position);
}

void Evaluator::edit_placeIC(SimPauseGuard& pauseGuard, eval_circuit_id_t evalCircuitId, DiffCache& diffCache, Position position, Rotation rotation, circuit_id_t circuitId) {
	#ifdef TRACY_PROFILER
		ZoneScoped;
	#endif
	changedICs = true;
	EvalCircuit* evalCircuit = evalCircuitContainer.getCircuit(evalCircuitId);
	if (!evalCircuit) {
		logError("EvalCircuit with id {} not found", "Evaluator::edit_placeIC", evalCircuitId);
		return;
	}
	eval_circuit_id_t newEvalCircuitId = evalCircuitContainer.addCircuit(evalCircuitId, circuitId);
	DifferenceSharedPtr diff = diffCache.getDifference(circuitId);
	makeEditInPlace(pauseGuard, newEvalCircuitId, diff, diffCache);
	evalCircuit->setNode(position, CircuitNode::fromIC(newEvalCircuitId));
}

void Evaluator::edit_removeConnection(SimPauseGuard& pauseGuard, eval_circuit_id_t evalCircuitId, DiffCache& diffCache, const BlockContainer* blockContainer, Position outputBlockPosition, Position outputPosition, Position inputBlockPosition, Position inputPosition) {
	#ifdef TRACY_PROFILER
		ZoneScoped;
	#endif
	std::optional<EvalConnectionPoint> outputPoint = getConnectionPoint(evalCircuitId, blockContainer, outputPosition, Direction::OUT);
	if (!outputPoint.has_value()) {
		logError("Output connection point not found for position {}", "Evaluator::edit_removeConnection", outputPosition.toString());
		return;
	}
	std::optional<EvalConnectionPoint> inputPoint = getConnectionPoint(evalCircuitId, blockContainer, inputPosition, Direction::IN);
	if (!inputPoint.has_value()) {
		logError("Input connection point not found for position {}", "Evaluator::edit_removeConnection", inputPosition.toString());
		return;
	}
	EvalConnection connection(outputPoint.value(), inputPoint.value());
	auto it = std::find_if(interCircuitConnections.begin(), interCircuitConnections.end(),
		[&connection](const auto& pair) {
			return pair.connection == connection;
		});
	if (it != interCircuitConnections.end()) {
		interCircuitConnections.erase(it);
	}
	evalSimulator.removeConnection(pauseGuard, connection);
}

void Evaluator::edit_createConnection(SimPauseGuard& pauseGuard, eval_circuit_id_t evalCircuitId, DiffCache& diffCache, const BlockContainer* blockContainer, Position outputBlockPosition, Position outputPosition, Position inputBlockPosition, Position inputPosition) {
	#ifdef TRACY_PROFILER
		ZoneScoped;
	#endif
	std::set<CircuitPortDependency> circuitPortDependencies;
	std::set<CircuitNode> circuitNodeDependencies;

	std::optional<EvalConnectionPoint> outputPoint = getConnectionPoint(evalCircuitId, blockContainer, outputPosition, Direction::OUT, circuitPortDependencies, circuitNodeDependencies);
	if (!outputPoint.has_value()) {
		logError("Output connection point not found for position {}", "Evaluator::edit_createConnection", outputPosition.toString());
		return;
	}
	std::optional<EvalConnectionPoint> inputPoint = getConnectionPoint(evalCircuitId, blockContainer, inputPosition, Direction::IN, circuitPortDependencies, circuitNodeDependencies);
	if (!inputPoint.has_value()) {
		logError("Input connection point not found for position {}", "Evaluator::edit_createConnection", inputPosition.toString());
		return;
	}
	EvalConnection connection(outputPoint.value(), inputPoint.value());
	if (!circuitPortDependencies.empty()) {
		interCircuitConnections.push_back({ connection, circuitPortDependencies, circuitNodeDependencies });
	}
	evalSimulator.makeConnection(pauseGuard, connection);
}

void Evaluator::removeDependentInterCircuitConnections(SimPauseGuard& pauseGuard, CircuitPortDependency circuitPortDependency) {
	// delete any connections that have the pair {circuitId, connectionEndId} in their traceSet
	for (auto it = interCircuitConnections.begin(); it != interCircuitConnections.end();) {
		if (it->circuitPortDependencies.find(circuitPortDependency) != it->circuitPortDependencies.end()) {
			evalSimulator.removeConnection(pauseGuard, it->connection);
			it = interCircuitConnections.erase(it);
		} else {
			++it;
		}
	}
}

void Evaluator::removeDependentInterCircuitConnections(SimPauseGuard& pauseGuard, CircuitNode node) {
	for (auto it = interCircuitConnections.begin(); it != interCircuitConnections.end();) {
		if (it->circuitNodeDependencies.find(node) != it->circuitNodeDependencies.end()) {
			evalSimulator.removeConnection(pauseGuard, it->connection);
			it = interCircuitConnections.erase(it);
		} else {
			++it;
		}
	}
}

void Evaluator::removeCircuitIO(const DataUpdateEventManager::EventData* data) {
	const DataUpdateEventManager::EventDataWithValue<RemoveCircuitIOData>* eventData = dynamic_cast<const DataUpdateEventManager::EventDataWithValue<RemoveCircuitIOData>*>(data);
	if (!eventData) {
		logError("Invalid event data type", "Evaluator::removeCircuitIO");
		return;
	}
	std::tuple<BlockType, connection_end_id_t, Position> dataValue = eventData->get();
	BlockType blockType = std::get<0>(dataValue);
	connection_end_id_t connectionEndId = std::get<1>(dataValue);
	Position position = std::get<2>(dataValue);

	circuit_id_t circuitId = circuitBlockDataManager.getCircuitId(blockType);
	SimPauseGuard pauseGuard = evalSimulator.beginEdit();
	removeDependentInterCircuitConnections(pauseGuard, {circuitId, connectionEndId});
}

void Evaluator::setCircuitIO(const DataUpdateEventManager::EventData* data) {
	const DataUpdateEventManager::EventDataWithValue<SetCircuitIOData>* eventData = dynamic_cast<const DataUpdateEventManager::EventDataWithValue<SetCircuitIOData>*>(data);
	if (!eventData) {
		logError("Invalid event data type", "Evaluator::setCircuitIO");
		return;
	}
	std::pair<BlockType, connection_end_id_t> dataValue = eventData->get();
	BlockType blockType = dataValue.first;
	connection_end_id_t connectionEndId = dataValue.second;

	circuit_id_t circuitId = circuitBlockDataManager.getCircuitId(blockType);
	if (circuitId == 0) {
		logError("Circuit ID for BlockType {} is 0, cannot set IO", "Evaluator::setCircuitIO", blockType);
		return;
	}
	SimPauseGuard pauseGuard = evalSimulator.beginEdit();
	removeDependentInterCircuitConnections(pauseGuard, { circuitId, connectionEndId });
	// get the new position
	CircuitBlockData* circuitBlockData = circuitBlockDataManager.getCircuitBlockData(circuitId);
	if (!circuitBlockData) {
		logError("CircuitBlockData for Circuit ID {} not found", "Evaluator::setCircuitIO", circuitId);
		return;
	}
	const Position* position = circuitBlockData->getConnectionIdToPosition(connectionEndId);
	if (!position) {
		logError("Position for connection end ID {} not found in CircuitBlockData for Circuit ID {}", "Evaluator::setCircuitIO", connectionEndId, circuitId);
		return;
	}
	// use checkToCreateExternalConnections
	// iterate over eval_circuit_id_t
	for (eval_circuit_id_t evalCircuitId = 0; evalCircuitId < evalCircuitContainer.size(); evalCircuitId++) {
		EvalCircuit* evalCircuit = evalCircuitContainer.getCircuit(evalCircuitId);
		if (!evalCircuit) {
			continue;
		}
		if (evalCircuit->getCircuitId() != circuitId) {
			continue;
		}
		checkToCreateExternalConnections(pauseGuard, evalCircuitId, *position);
	}
}

std::optional<connection_port_id_t> Evaluator::getPortId(const circuit_id_t circuitId, const Position blockPosition, const Position portPosition, Direction direction) const {
	SharedCircuit circuit = circuitManager.getCircuit(circuitId);
	if (!circuit) {
		// logError("Circuit with ID {} not found", "Evaluator::getPortId", circuitId);
		return std::nullopt;
	}
	const BlockContainer* blockContainer = circuit->getBlockContainer();
	if (!blockContainer) {
		// logError("BlockContainer not found", "Evaluator::getPortId");
		return std::nullopt;
	}
	const Block* block = blockContainer->getBlock(blockPosition);
	if (!block) {
		// logError("Block not found at position {}", "Evaluator::getPortId", blockPosition.toString());
		return std::nullopt;
	}
	if (direction == Direction::IN) {
		const std::pair<connection_port_id_t, bool> port = block->getInputConnectionId(portPosition);
		if (!port.second) {
			// logError("Input port not found at position {}", "Evaluator::getPortId", portPosition.toString());
			return std::nullopt;
		}
		return port.first;
	} else {
		const std::pair<connection_port_id_t, bool> port = block->getOutputConnectionId(portPosition);
		if (!port.second) {
			// logError("Output port not found at position {}", "Evaluator::getPortId", portPosition.toString());
			return std::nullopt;
		}
		return port.first;
	}
}

std::optional<connection_port_id_t> Evaluator::getPortId(const BlockContainer* blockContainer, const Position blockPosition, const Position portPosition, Direction direction) const {
	const Block* block = blockContainer->getBlock(blockPosition);
	if (!block) {
		return std::nullopt;
	}
	if (direction == Direction::IN) {
		const std::pair<connection_port_id_t, bool> port = block->getInputConnectionId(portPosition);
		if (!port.second) {
			return std::nullopt;
		}
		return port.first;
	} else {
		const std::pair<connection_port_id_t, bool> port = block->getOutputConnectionId(portPosition);
		if (!port.second) {
			return std::nullopt;
		}
		return port.first;
	}
}

std::optional<EvalConnectionPoint> Evaluator::getConnectionPoint(const eval_circuit_id_t evalCircuitId, const Position portPosition, Direction direction) const {
	#ifdef TRACY_PROFILER
		ZoneScoped;
	#endif
	EvalCircuit* evalCircuit = evalCircuitContainer.getCircuit(evalCircuitId);
	if (!evalCircuit) {
		return std::nullopt;
	}
	circuit_id_t circuitId = evalCircuit->getCircuitId();
	SharedCircuit circuit = circuitManager.getCircuit(circuitId);
	if (!circuit) {
		return std::nullopt;
	}
	const BlockContainer* blockContainer = circuit->getBlockContainer();
	if (!blockContainer) {
		return std::nullopt;
	}
	return getConnectionPoint(evalCircuitId, blockContainer, portPosition, direction);
}

std::optional<EvalConnectionPoint> Evaluator::getConnectionPoint(const eval_circuit_id_t evalCircuitId, const BlockContainer* blockContainer, const Position portPosition, Direction direction) const {
	#ifdef TRACY_PROFILER
		ZoneScoped;
	#endif
	EvalCircuit* evalCircuit = evalCircuitContainer.getCircuit(evalCircuitId);
	if (!evalCircuit) {
		return std::nullopt;
	}
	const Block* block = blockContainer->getBlock(portPosition);
	if (!block) {
		return std::nullopt;
	}
	BlockType blockType = block->type();
	Position blockPosition = block->getPosition();
	std::optional<CircuitNode> node = evalCircuit->getNode(blockPosition);
	if (!node.has_value()) {
		return std::nullopt;
	}
	if (blockType == BlockType::SWITCH || blockType == BlockType::BUTTON || blockType == BlockType::TICK_BUTTON) {
		// For switches and buttons, we can use the position directly
		if (direction == Direction::IN) {
			return EvalConnectionPoint(node->getId(), 0);
		}
	}
	if (blockType == BlockType::LIGHT) {
		if (direction == Direction::OUT) {
			return EvalConnectionPoint(node->getId(), 0);
		}
	}
	std::optional<connection_port_id_t> portId = getPortId(blockContainer, blockPosition, portPosition, direction);
	if (!portId.has_value()) {
		// logError("Port not found at position {}", "Evaluator::getConnectionPoint", portPosition.toString());
		return std::nullopt;
	}
	if (node->isIC()) {
		circuit_id_t circuitId = circuitBlockDataManager.getCircuitId(blockType);
		CircuitBlockData* circuitBlockData = circuitBlockDataManager.getCircuitBlockData(circuitId);
		if (!circuitBlockData) {
			logError("CircuitBlockData for type {} not found", "Evaluator::getConnectionPoint", blockType);
			return std::nullopt;
		}
		const Position* internalPosition = circuitBlockData->getConnectionIdToPosition(portId.value());
		if (!internalPosition) {
			logError("Internal position for port ID {} not found in CircuitBlockData for type {}", "Evaluator::getConnectionPoint", portId.value(), blockType);
			return std::nullopt;
		}
		return getConnectionPoint(node->getId(), *internalPosition, direction);
	}
	return EvalConnectionPoint(node->getId(), portId.value());
}

std::optional<EvalConnectionPoint> Evaluator::getConnectionPoint(
	const eval_circuit_id_t evalCircuitId,
	const Position portPosition,
	Direction direction,
	std::set<CircuitPortDependency>& circuitPortDependencies,
	std::set<CircuitNode>& circuitNodeDependencies
	) const {
	#ifdef TRACY_PROFILER
		ZoneScoped;
	#endif
	EvalCircuit* evalCircuit = evalCircuitContainer.getCircuit(evalCircuitId);
	if (!evalCircuit) {
		return std::nullopt;
	}
	circuit_id_t circuitId = evalCircuit->getCircuitId();
	SharedCircuit circuit = circuitManager.getCircuit(circuitId);
	if (!circuit) {
		return std::nullopt;
	}
	const BlockContainer* blockContainer = circuit->getBlockContainer();
	if (!blockContainer) {
		return std::nullopt;
	}
	return getConnectionPoint(evalCircuitId, blockContainer, portPosition, direction, circuitPortDependencies, circuitNodeDependencies);
}

std::optional<EvalConnectionPoint> Evaluator::getConnectionPoint(
	const eval_circuit_id_t evalCircuitId,
	const BlockContainer* blockContainer,
	const Position portPosition,
	Direction direction,
	std::set<CircuitPortDependency>& circuitPortDependencies,
	std::set<CircuitNode>& circuitNodeDependencies
) const {
	#ifdef TRACY_PROFILER
		ZoneScoped;
	#endif
	EvalCircuit* evalCircuit = evalCircuitContainer.getCircuit(evalCircuitId);
	if (!evalCircuit) {
		return std::nullopt;
	}
	const Block* block = blockContainer->getBlock(portPosition);
	if (!block) {
		return std::nullopt;
	}
	BlockType blockType = block->type();
	Position blockPosition = block->getPosition();
	std::optional<CircuitNode> node = evalCircuit->getNode(blockPosition);
	if (!node.has_value()) {
		return std::nullopt;
	}
	circuitNodeDependencies.insert(node.value());
	if (blockType == BlockType::SWITCH || blockType == BlockType::BUTTON || blockType == BlockType::TICK_BUTTON) {
		// For switches and buttons, we can use the position directly
		if (direction == Direction::IN) {
			return EvalConnectionPoint(node->getId(), 0);
		}
	}
	if (blockType == BlockType::LIGHT) {
		if (direction == Direction::OUT) {
			return EvalConnectionPoint(node->getId(), 0);
		}
	}
	std::optional<connection_port_id_t> portId = getPortId(blockContainer, blockPosition, portPosition, direction);
	if (!portId.has_value()) {
		// logError("Port not found at position {}", "Evaluator::getConnectionPoint", portPosition.toString());
		return std::nullopt;
	}
	if (node->isIC()) {
		circuit_id_t circuitId = circuitBlockDataManager.getCircuitId(blockType);
		CircuitBlockData* circuitBlockData = circuitBlockDataManager.getCircuitBlockData(circuitId);
		if (!circuitBlockData) {
			logError("CircuitBlockData for type {} not found", "Evaluator::getConnectionPoint", blockType);
			return std::nullopt;
		}
		const Position* internalPosition = circuitBlockData->getConnectionIdToPosition(portId.value());
		if (!internalPosition) {
			logError("Internal position for port ID {} not found in CircuitBlockData for type {}", "Evaluator::getConnectionPoint", portId.value(), blockType);
			return std::nullopt;
		}
		circuitPortDependencies.insert({ circuitId, portId.value() });
		return getConnectionPoint(node->getId(), *internalPosition, direction, circuitPortDependencies, circuitNodeDependencies);
	}
	return EvalConnectionPoint(node->getId(), portId.value());
}

void Evaluator::edit_moveBlock(SimPauseGuard& pauseGuard, eval_circuit_id_t evalCircuitId, DiffCache& diffCache, Position curPosition, Rotation curRotation, Position newPosition, Rotation newRotation) {
	#ifdef TRACY_PROFILER
		ZoneScoped;
	#endif
	EvalCircuit* evalCircuit = evalCircuitContainer.getCircuit(evalCircuitId);
	if (!evalCircuit) {
		logError("EvalCircuit with id {} not found", "Evaluator::edit_moveBlock", evalCircuitId);
		return;
	}
	// delete interCircuit connections that are dependent on the block being moved
	std::optional<CircuitNode> node = evalCircuit->getNode(curPosition);
	if (!node.has_value()) {
		logError("Node at position {} not found", "Evaluator::edit_moveBlock", curPosition.toString());
		return;
	}
	removeDependentInterCircuitConnections(pauseGuard, node.value());
	evalCircuit->moveNode(curPosition, newPosition);
	checkToCreateExternalConnections(pauseGuard, evalCircuitId, newPosition);
}

const EvalAddressTree Evaluator::buildAddressTree() const {
	return buildAddressTree(0);
}

const EvalAddressTree Evaluator::buildAddressTree(eval_circuit_id_t evalCircuitId) const {
	std::shared_lock lk(simMutex);
	EvalCircuit* evalCircuit = evalCircuitContainer.getCircuit(evalCircuitId);
	if (!evalCircuit) {
		logError("EvalCircuit with id {} not found", "Evaluator::buildAddressTree", evalCircuitId);
		return EvalAddressTree(0);
	}
	EvalAddressTree root = EvalAddressTree(evalCircuit->getCircuitId());
	evalCircuit->forEachNode([this, &root](Position pos, const CircuitNode& node) {
		if (node.isIC()) {
			root.addBranch(pos, buildAddressTree(node.getId()));
		}
	});
	return root;
}

std::optional<middle_id_t> Evaluator::getMiddleId(const eval_circuit_id_t startingPoint, const Address& address) const {
	eval_circuit_id_t evalCircuitId = evalCircuitContainer.traverseToTopLevelIC(startingPoint, address);
	circuit_id_t circuitId = evalCircuitContainer.getCircuitId(evalCircuitId).value_or(0);
	SharedCircuit circuit = circuitManager.getCircuit(circuitId);
	if (!circuit) {
		logError("Circuit with ID {} not found", "Evaluator::getMiddleId", circuitId);
		return std::nullopt;
	}
	const BlockContainer* blockContainer = circuit->getBlockContainer();
	if (!blockContainer) {
		logError("BlockContainer not found", "Evaluator::getMiddleId");
		return std::nullopt;
	}
	Position blockPosition = address.getPosition(address.size() - 1);
	const Block* block = blockContainer->getBlock(blockPosition);
	if (!block) {
		logError("Block not found at position {}", "Evaluator::getMiddleId", blockPosition.toString());
		return std::nullopt;
	}
	std::optional<CircuitNode> node = evalCircuitContainer.getNode(block->getPosition(), evalCircuitId);
	if (!node.has_value()) {
		logError("Node not found for address {}", "Evaluator::getMiddleId", address.toString());
		return std::nullopt;
	}
	return node->getId();
}

std::optional<middle_id_t> Evaluator::getMiddleId(const eval_circuit_id_t startingPoint, const Address& address, const BlockContainer* blockContainer) const {
	eval_circuit_id_t evalCircuitId = evalCircuitContainer.traverseToTopLevelIC(startingPoint, address);
	Position blockPosition = address.getPosition(address.size() - 1);
	const Block* block = blockContainer->getBlock(blockPosition);
	if (!block) {
		logError("Block not found at position {}", "Evaluator::getMiddleId", blockPosition.toString());
		return std::nullopt;
	}
	std::optional<CircuitNode> node = evalCircuitContainer.getNode(block->getPosition(), evalCircuitId);
	if (!node.has_value()) {
		logError("Node not found for address {}", "Evaluator::getMiddleId", address.toString());
		return std::nullopt;
	}
	return node->getId();
}

std::optional<middle_id_t> Evaluator::getMiddleId(const Address& address) const {
	return getMiddleId(0, address);
}

logic_state_t Evaluator::getState(const Address& address) {
	std::shared_lock lk(simMutex);
	std::optional<eval_circuit_id_t> evalCircuitIdOpt = evalCircuitContainer.traverseToTopLevelIC(address);
	if (!evalCircuitIdOpt.has_value()) {
		logError("Failed to traverse to top-level IC for address {}", "Evaluator::getState", address.toString());
		return logic_state_t::UNDEFINED;
	}
	
	eval_circuit_id_t evalCircuitId = evalCircuitIdOpt.value();
	EvalCircuit* evalCircuit = evalCircuitContainer.getCircuit(evalCircuitId);
	if (!evalCircuit) {
		logError("EvalCircuit with id {} not found", "Evaluator::getState", evalCircuitId);
		return logic_state_t::UNDEFINED;
	}
	
	circuit_id_t circuitId = evalCircuit->getCircuitId();
	SharedCircuit circuit = circuitManager.getCircuit(circuitId);
	if (!circuit) {
		logError("Circuit with ID {} not found", "Evaluator::getState", circuitId);
		return logic_state_t::UNDEFINED;
	}
	const BlockContainer* blockContainer = circuit->getBlockContainer();
	if (!blockContainer) {
		logError("BlockContainer not found", "Evaluator::getState");
		return logic_state_t::UNDEFINED;
	}
	
	std::optional<EvalConnectionPoint> connectionPointOpt = getConnectionPoint(evalCircuitId, blockContainer, address.getPosition(address.size() - 1), Direction::OUT);
	if (!connectionPointOpt.has_value()) {
		logError("Connection point not found for address {}", "Evaluator::getState", address.toString());
		return logic_state_t::UNDEFINED;
	}
	return evalSimulator.getState(connectionPointOpt.value());
}

void Evaluator::setState(const Address& address, logic_state_t state) {
	std::unique_lock lk(simMutex);
	std::optional<eval_circuit_id_t> evalCircuitIdOpt = evalCircuitContainer.traverseToTopLevelIC(address);
	if (!evalCircuitIdOpt.has_value()) {
		logError("Failed to traverse to top-level IC for address {}", "Evaluator::setState", address.toString());
		return;
	}
	
	eval_circuit_id_t evalCircuitId = evalCircuitIdOpt.value();
	EvalCircuit* evalCircuit = evalCircuitContainer.getCircuit(evalCircuitId);
	if (!evalCircuit) {
		logError("EvalCircuit with id {} not found", "Evaluator::setState", evalCircuitId);
		return;
	}
	
	circuit_id_t circuitId = evalCircuit->getCircuitId();
	SharedCircuit circuit = circuitManager.getCircuit(circuitId);
	if (!circuit) {
		logError("Circuit with ID {} not found", "Evaluator::setState", circuitId);
		return;
	}
	const BlockContainer* blockContainer = circuit->getBlockContainer();
	if (!blockContainer) {
		logError("BlockContainer not found", "Evaluator::setState");
		return;
	}
	
	std::optional<EvalConnectionPoint> connectionPointOpt = getConnectionPoint(evalCircuitId, blockContainer, address.getPosition(address.size() - 1), Direction::OUT);
	if (connectionPointOpt.has_value()) {
		evalSimulator.setState(connectionPointOpt.value(), state);
		return;
	}
	std::optional<EvalConnectionPoint> connectionPointOptIn = getConnectionPoint(evalCircuitId, blockContainer, address.getPosition(address.size() - 1), Direction::IN);
	if (connectionPointOptIn.has_value()) {
		evalSimulator.setState(connectionPointOptIn.value(), state);
		return;
	}
	std::optional<middle_id_t> middleIdOpt = getMiddleId(evalCircuitId, address, blockContainer);
	if (middleIdOpt.has_value()) {
		EvalConnectionPoint connectionPoint(middleIdOpt.value(), 0);
		evalSimulator.setState(connectionPoint, state);
		return;
	}
	logError("Failed to get connection point for address {}", "Evaluator::setState", address.toString());
}

std::vector<logic_state_t> Evaluator::getBulkStates(const std::vector<Address>& addresses, const Address& addressOrigin) {
	#ifdef TRACY_PROFILER
		ZoneScoped;
	#endif
	if (addresses.empty()) {
		return {};
	}
	std::shared_lock lk(simMutex);
	eval_circuit_id_t startingPoint = evalCircuitContainer.traverseToTopLevelIC(addressOrigin);
	
	// Pre-compute all evalCircuitIds to minimize traversal calls
	std::vector<eval_circuit_id_t> evalCircuitIds;
	evalCircuitIds.reserve(addresses.size());
	for (const Address& addr : addresses) {
		evalCircuitIds.push_back(evalCircuitContainer.traverseToTopLevelIC(startingPoint, addr));
	}
	
	// Cache all required resources for each unique eval_circuit_id
	struct CircuitCacheEntry {
		EvalCircuit* evalCircuit = nullptr;
		const BlockContainer* blockContainer = nullptr;
		circuit_id_t circuitId = 0;
	};
	std::unordered_map<eval_circuit_id_t, CircuitCacheEntry> circuitCache;
	
	// Pre-populate cache for all unique evalCircuitIds
	for (eval_circuit_id_t evalCircuitId : evalCircuitIds) {
		if (circuitCache.find(evalCircuitId) == circuitCache.end()) {
			CircuitCacheEntry entry;
			entry.evalCircuit = evalCircuitContainer.getCircuit(evalCircuitId);
			if (entry.evalCircuit) {
				entry.circuitId = evalCircuitContainer.getCircuitId(evalCircuitId).value_or(0);
				SharedCircuit circuit = circuitManager.getCircuit(entry.circuitId);
				if (circuit) {
					entry.blockContainer = circuit->getBlockContainer();
				}
			}
			circuitCache[evalCircuitId] = entry;
		}
	}
	
	std::vector<EvalConnectionPoint> connectionPoints;
	connectionPoints.reserve(addresses.size());
	
	for (size_t i = 0; i < addresses.size(); ++i) {
		const Address& addr = addresses[i];
		eval_circuit_id_t evalCircuitId = evalCircuitIds[i];

		const CircuitCacheEntry& cacheEntry = circuitCache[evalCircuitId];
		if (!cacheEntry.blockContainer || !cacheEntry.evalCircuit) {
			logError("Failed to get cached resources for evalCircuitId {}", "Evaluator::getBulkStates", evalCircuitId);
			connectionPoints.emplace_back(0, 0);
			continue;
		}
		
		Position portPosition = addr.getPosition(addr.size() - 1);

		std::optional<EvalConnectionPoint> connectionPoint = getConnectionPoint(evalCircuitId, cacheEntry.blockContainer, portPosition, Direction::OUT);
		if (connectionPoint.has_value()) {
			connectionPoints.push_back(connectionPoint.value());
			continue;
		}
		
		// Inline optimized getConnectionPoint logic to avoid function call overhead
		const Block* block = cacheEntry.blockContainer->getBlock(portPosition);
		if (!block) {
			connectionPoints.emplace_back(0, 0);
			continue;
		}

		Position blockPosition = block->getPosition();
		std::optional<CircuitNode> node = cacheEntry.evalCircuit->getNode(blockPosition);
		if (!node.has_value()) {
			connectionPoints.emplace_back(0, 0);
			continue;
		}

		if (node->isIC()) {
			connectionPoints.emplace_back(0, 0);
			continue;
		}

		connectionPoints.emplace_back(node->getId(), 0);
	}

	return evalSimulator.getStates(connectionPoints);
}

void Evaluator::checkToCreateExternalConnections(SimPauseGuard& pauseGuard, eval_circuit_id_t evalCircuitId, Position position) {
	#ifdef TRACY_PROFILER
		ZoneScoped;
	#endif
	// logInfo("Checking to create external connections for evalCircuitId {} at position {}", "Evaluator::checkToCreateExternalConnections", evalCircuitId, position.toString());
	EvalCircuit* evalCircuit = evalCircuitContainer.getCircuit(evalCircuitId);
	if (!evalCircuit) {
		logError("EvalCircuit with id {} not found", "Evaluator::checkToCreateExternalConnections", evalCircuitId);
		return;
	}
	if (evalCircuit->isRoot()) {
		// logInfo("EvalCircuit is root, no external connections to create", "Evaluator::checkToCreateExternalConnections");
		return;
	}
	std::optional<CircuitNode> node = evalCircuit->getNode(position);
	if (!node.has_value()) {
		logError("Node at position {} not found", "Evaluator::checkToCreateExternalConnections", position.toString());
		return;
	}
	// logInfo("Node found: {}", "Evaluator::checkToCreateExternalConnections", node->toString());
	SharedCircuit circuit = circuitManager.getCircuit(evalCircuit->getCircuitId());
	if (!circuit) {
		logError("Circuit with id {} not found", "Evaluator::makeEditInPlace", evalCircuit->getCircuitId());
		return;
	}
	const BlockContainer* blockContainer = circuit->getBlockContainer();
	if (!blockContainer) {
		logError("BlockContainer not found", "Evaluator::makeEditInPlace");
		return;
	}
	const Block* block = blockContainer->getBlock(position);
	if (!block) {
		logError("Block not found at position {}", "Evaluator::checkToCreateExternalConnections", position.toString());
		return;
	}

	// Get block data to iterate through all connections
	const BlockData* blockData = blockDataManager.getBlockData(block->type());
	if (!blockData) {
		logError("BlockData not found for block type {}", "Evaluator::checkToCreateExternalConnections", static_cast<int>(block->type()));
		return;
	}

	// Iterate through all connections of the block
	struct ConnectionData {
		Position portPosition;
		connection_end_id_t connectionId;
		Direction direction;
	};
	std::vector<ConnectionData> connectionDataList;
	if (blockData->isDefaultData()) {
		// logInfo("Block type {} is default data", "Evaluator::checkToCreateExternalConnections", static_cast<int>(block->type()));
		connectionDataList.push_back({ position, 0, Direction::IN });
		connectionDataList.push_back({ position, 1, Direction::OUT });
	} else {
	const auto& connections = blockData->getConnections();
	// logInfo("Found {} connections for block type {}", "Evaluator::checkToCreateExternalConnections", connections.size(), static_cast<int>(block->type()));
	for (const auto& [connectionId, connectionOffset] : connections) {
		// Check if the connection is valid
		Vector portOffset = connectionOffset.first;
		Position portPosition = block->getPosition() + portOffset;
		// Determine direction (input or output)
		Direction direction = block->isConnectionInput(connectionId) ? Direction::IN : Direction::OUT;
		// logInfo("Connection {} at position {} with direction {}", "Evaluator::checkToCreateExternalConnections", connectionId, portPosition.toString(), (direction == Direction::IN ? "IN" : "OUT"));
		connectionDataList.push_back({ portPosition, connectionId, direction });
	}
	if (block->type() == BlockType::SWITCH || block->type() == BlockType::BUTTON || block->type() == BlockType::TICK_BUTTON) {
		connectionDataList.push_back({ position, 1, Direction::IN });
	}
	if (block->type() == BlockType::LIGHT) {
		connectionDataList.push_back({ position, 0, Direction::OUT });
	}}
	for (const auto& connectionData : connectionDataList) {
		Position portPosition = connectionData.portPosition;
		Direction direction = connectionData.direction;

		// logInfo("Checking connection at position {} with direction {}", "Evaluator::checkToCreateExternalConnections", portPosition.toString(), (direction == Direction::IN ? "IN" : "OUT"));

		std::set<CircuitPortDependency> circuitPortDependencies;
		std::set<CircuitNode> circuitNodeDependencies;
		circuitNodeDependencies.insert(node.value());
		std::optional<EvalConnectionPoint> connectionPoint = getConnectionPoint(
			evalCircuitId, blockContainer, portPosition, direction, circuitPortDependencies, circuitNodeDependencies);

		if (connectionPoint.has_value()) {
			traceOutwardsIC(
				pauseGuard,
				evalCircuitId,
				position,
				direction,
				connectionPoint.value(),
				circuitPortDependencies,
				circuitNodeDependencies
			);
		} else {
			logError("Connection point not found for position {}", "Evaluator::checkToCreateExternalConnections", portPosition.toString());
		}
	}
}

void Evaluator::traceOutwardsIC(
	SimPauseGuard& pauseGuard,
	eval_circuit_id_t evalCircuitId,
	Position position,
	Direction direction,
	const EvalConnectionPoint& targetConnectionPoint,
	std::set<CircuitPortDependency>& circuitPortDependencies,
	std::set<CircuitNode>& circuitNodeDependencies
) {
	#ifdef TRACY_PROFILER
		ZoneScoped;
	#endif
	EvalCircuit* evalCircuit = evalCircuitContainer.getCircuit(evalCircuitId);
	if (!evalCircuit) {
		logError("EvalCircuit with id {} not found", "Evaluator::traceOutwardsIC", evalCircuitId);
		return;
	}
	if (evalCircuit->isRoot()) {
		return;
	}
	circuit_id_t circuitId = evalCircuit->getCircuitId();
	CircuitBlockData* circuitBlockData = circuitBlockDataManager.getCircuitBlockData(circuitId);
	if (!circuitBlockData) {
		logError("CircuitBlockData for circuit ID {} not found", "Evaluator::traceOutwardsIC", circuitId);
		return;
	}
	BidirectionalMultiSecondKeyMap<connection_end_id_t, Position>::constIteratorPairT2 connectionIds = circuitBlockData->getConnectionPositionToId(position);
	// go through all the connection_end_ids
	eval_circuit_id_t parentEvalCircuitId = evalCircuit->getParentEvalId();
	EvalCircuit* parentEvalCircuit = evalCircuitContainer.getCircuit(parentEvalCircuitId);
	if (!parentEvalCircuit) {
		logError("Parent EvalCircuit with id {} not found", "Evaluator::traceOutwardsIC", parentEvalCircuitId);
		return;
	}

	SharedCircuit parentCircuit = circuitManager.getCircuit(parentEvalCircuit->getCircuitId());
	if (!parentCircuit) {
		logError("Circuit with id {} not found", "Evaluator::traceOutwardsIC", parentEvalCircuit->getCircuitId());
		return;
	}
	const BlockContainer* parentCircuitBlockContainer = parentCircuit->getBlockContainer();
	if (!parentCircuitBlockContainer) {
		logError("BlockContainer not found", "Evaluator::traceOutwardsIC");
		return;
	}
	std::optional<Position> parentCircuitPositionOpt = parentEvalCircuit->getPosition(CircuitNode::fromIC(evalCircuitId));
	if (!parentCircuitPositionOpt.has_value()) {
		logError("Parent circuit position not found for evalCircuitId {}", "Evaluator::traceOutwardsIC", evalCircuitId);
		return;
	}
	const Block* parentCircuitBlock = parentCircuitBlockContainer->getBlock(parentCircuitPositionOpt.value());
	if (!parentCircuitBlock) {
		logError("Block not found at position {}", "Evaluator::traceOutwardsIC", parentCircuitPositionOpt.value().toString());
		return;
	}

	circuitNodeDependencies.insert(CircuitNode::fromIC(evalCircuitId));

	for (auto iter = connectionIds.first; iter != connectionIds.second; ++iter) {
		connection_end_id_t connectionEndId = iter->second;
		if (parentCircuitBlock->isConnectionInput(connectionEndId) != (direction == Direction::IN)) {
			continue;
		}
		std::pair<Position, bool> connectionPosition = parentCircuitBlock->getConnectionPosition(connectionEndId);
		if (!connectionPosition.second) {
			logError("Connection position not found at connectionEndId {}", "Evaluator::traceOutwardsIC", connectionEndId);
			continue;
		}
		Position connectionPos = connectionPosition.first;
		const std::unordered_set<ConnectionEnd>* connectionEnds = (direction == Direction::IN) ? 
			parentCircuitBlock->getInputConnections(connectionPos) : parentCircuitBlock->getOutputConnections(connectionPos);
		if (!connectionEnds) {
			logError("Connection ends not found at position {}", "Evaluator::traceOutwardsIC", connectionPos.toString());
			continue;
		}

		circuitPortDependencies.insert({ circuitId, connectionEndId });

		for (const ConnectionEnd& connectionEnd : *connectionEnds) {
			const Block* targetBlock = parentCircuitBlockContainer->getBlock(connectionEnd.getBlockId());
			if (!targetBlock) {
				logError("Target block not found for connectionEnd {}", "Evaluator::traceOutwardsIC", connectionEnd.toString());
				continue;
			}
			std::pair<Position, bool> targetConnectionPointPositionOpt = targetBlock->getConnectionPosition(connectionEnd.getConnectionId());
			if (!targetConnectionPointPositionOpt.second) {
				logError("Target connection position not found for connectionEnd {}", "Evaluator::traceOutwardsIC", connectionEnd.toString());
				continue;
			}
			Position targetConnectionPointPosition = targetConnectionPointPositionOpt.first;
			std::set<CircuitPortDependency> circuitPortDependenciesCopy = circuitPortDependencies;
			std::set<CircuitNode> circuitNodeDependenciesCopy = circuitNodeDependencies;
			// get connection point
			std::optional<EvalConnectionPoint> connectionPoint = getConnectionPoint(
				parentEvalCircuitId, parentCircuitBlockContainer, targetConnectionPointPosition, !direction, circuitPortDependenciesCopy, circuitNodeDependenciesCopy);
			if (!connectionPoint.has_value()) {
				continue;
			}
			if (connectionPoint->gateId == targetConnectionPoint.gateId && direction == Direction::OUT) {
				// this is a safety against a gate wiring to itself twice if it connects to itself in a round-about way
				continue;
			}
			// If we reached here, we found a valid connection point
			EvalConnection evalConnection(connectionPoint.value(), targetConnectionPoint);
			evalSimulator.makeConnection(pauseGuard, evalConnection);
			interCircuitConnections.push_back({
				evalConnection,
				circuitPortDependenciesCopy,
				circuitNodeDependenciesCopy
			});

			traceOutwardsIC(
				pauseGuard,
				parentEvalCircuitId,
				targetConnectionPointPosition,
				direction,
				targetConnectionPoint,
				circuitPortDependencies,
				circuitNodeDependencies
			);
		}

		circuitPortDependencies.erase({ circuitId, connectionEndId });
	}

	circuitNodeDependencies.erase(CircuitNode::fromIC(evalCircuitId));
}