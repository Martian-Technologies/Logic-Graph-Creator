#include "evaluator.h"

Evaluator::Evaluator(evaluator_id_t evaluatorId, CircuitManager& circuitManager, circuit_id_t circuitId, DataUpdateEventManager* dataUpdateEventManager)
	: evaluatorId(evaluatorId),
	circuitManager(circuitManager),
	evalCircuitContainer(),
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
	evalCircuitContainer.addCircuit(circuitId);
	const auto blockContainer = circuit->getBlockContainer();
	const Difference difference = blockContainer->getCreationDifference();
	// receiver.linkFunction("blockDataRemoveConnection", std::bind(&Evaluator::removeCircuitIO, this, std::placeholders::_1));

	makeEdit(std::make_shared<Difference>(difference), circuitId);
}

void Evaluator::makeEdit(DifferenceSharedPtr difference, circuit_id_t circuitId) {
	logInfo("_________________________________________________________________________________________");
	logInfo("Applying edit to Evaluator with ID {} for Circuit ID {}", "Evaluator::makeEdit", evaluatorId, circuitId);
	SimPauseGuard pauseGuard = evalSimulator.beginEdit();
	DiffCache diffCache(circuitManager);
	for (eval_circuit_id_t evalCircuitId = 0; evalCircuitId < evalCircuitContainer.size(); evalCircuitId++) {
		if (evalCircuitContainer.getCircuitId(evalCircuitId) == circuitId) {
			makeEditInPlace(pauseGuard, evalCircuitId, difference, diffCache);
		}
	}
	evalSimulator.endEdit(pauseGuard);
}

void Evaluator::makeEditInPlace(SimPauseGuard& pauseGuard, eval_circuit_id_t evalCircuitId, DifferenceSharedPtr difference, DiffCache& diffCache) {
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
				edit_removeConnection(pauseGuard, evalCircuitId, diffCache, outputBlockPosition, outputPosition, inputBlockPosition, inputPosition);
				break;
			}
			case Difference::ModificationType::CREATED_CONNECTION: {
				const auto& [outputBlockPosition, outputPosition, inputBlockPosition, inputPosition] = std::get<Difference::connection_modification_t>(modificationData);
				edit_createConnection(pauseGuard, evalCircuitId, diffCache, outputBlockPosition, outputPosition, inputBlockPosition, inputPosition);
				break;
			}
		}
	}
}

void Evaluator::edit_removeBlock(SimPauseGuard& pauseGuard, eval_circuit_id_t evalCircuitId, DiffCache& diffCache, Position position, Rotation rotation, BlockType type) {
	logInfo("Removing block of type {} at position {}", "Evaluator::edit_removeBlock", type, position.toString());
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
		logError("Cannot remove IC nodes yet", "Evaluator::edit_removeBlock", position.toString());
		return;
	}
	evalSimulator.removeGate(pauseGuard, node->getId());
	middleIdProvider.releaseId(node->getId());
	logInfo("Released middle ID {}", "Evaluator::edit_removeBlock", node->getId());
	evalCircuit->removeNode(position);
}

void Evaluator::edit_placeBlock(SimPauseGuard& pauseGuard, eval_circuit_id_t evalCircuitId, DiffCache& diffCache, Position position, Rotation rotation, BlockType type) {
	logInfo("Placing block of type {} at position {}", "Evaluator::edit_placeBlock", type, position.toString());
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
		logError("Unsupported BlockType {}", "Evaluator::edit_placeBlock", type);
		return;
	}
	middle_id_t gateId = middleIdProvider.getNewId();
	logInfo("Using middle ID {} for new gate", "Evaluator::edit_placeBlock", gateId);
	evalSimulator.addGate(pauseGuard, gateType, gateId);
	EvalCircuit* evalCircuit = evalCircuitContainer.getCircuit(evalCircuitId);
	if (!evalCircuit) {
		logError("EvalCircuit with id {} not found", "Evaluator::edit_placeBlock", evalCircuitId);
		return;
	}
	CircuitNode node = CircuitNode::fromMiddle(gateId);
	evalCircuit->setNode(position, node);
}

void Evaluator::edit_removeConnection(SimPauseGuard& pauseGuard, eval_circuit_id_t evalCircuitId, DiffCache& diffCache, Position outputBlockPosition, Position outputPosition, Position inputBlockPosition, Position inputPosition) {
	logInfo("Removing connection from output {} at {} to input {} at {}. evalCircuitId: {}", "Evaluator::edit_removeConnection", outputBlockPosition.toString(), outputPosition.toString(), inputBlockPosition.toString(), inputPosition.toString(), evalCircuitId);
	std::optional<EvalConnectionPoint> outputPoint = getConnectionPoint(evalCircuitId, outputPosition, Direction::OUT);
	if (!outputPoint.has_value()) {
		logError("Output connection point not found for position {}", "Evaluator::edit_removeConnection", outputPosition.toString());
		return;
	}
	std::optional<EvalConnectionPoint> inputPoint = getConnectionPoint(evalCircuitId, inputPosition, Direction::IN);
	if (!inputPoint.has_value()) {
		logError("Input connection point not found for position {}", "Evaluator::edit_removeConnection", inputPosition.toString());
		return;
	}
	EvalConnection connection(outputPoint.value(), inputPoint.value());
	evalSimulator.removeConnection(pauseGuard, connection);
}

void Evaluator::edit_createConnection(SimPauseGuard& pauseGuard, eval_circuit_id_t evalCircuitId, DiffCache& diffCache, Position outputBlockPosition, Position outputPosition, Position inputBlockPosition, Position inputPosition) {
	logInfo("Creating connection from output {} at {} to input {} at {}", "Evaluator::edit_createConnection", outputBlockPosition.toString(), outputPosition.toString(), inputBlockPosition.toString(), inputPosition.toString());
	std::optional<EvalConnectionPoint> outputPoint = getConnectionPoint(evalCircuitId, outputPosition, Direction::OUT);
	if (!outputPoint.has_value()) {
		logError("Output connection point not found for position {}", "Evaluator::edit_createConnection", outputPosition.toString());
		return;
	}
	std::optional<EvalConnectionPoint> inputPoint = getConnectionPoint(evalCircuitId, inputPosition, Direction::IN);
	if (!inputPoint.has_value()) {
		logError("Input connection point not found for position {}", "Evaluator::edit_createConnection", inputPosition.toString());
		return;
	}
	EvalConnection connection(outputPoint.value(), inputPoint.value());
	evalSimulator.makeConnection(pauseGuard, connection);
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

std::optional<EvalConnectionPoint> Evaluator::getConnectionPoint(const eval_circuit_id_t evalCircuitId, const Position portPosition, Direction direction) const {
	EvalCircuit* evalCircuit = evalCircuitContainer.getCircuit(evalCircuitId);
	if (!evalCircuit) {
		// logError("EvalCircuit with id {} not found", "Evaluator::getConnectionPoint", evalCircuitId);
		return std::nullopt;
	}
	circuit_id_t circuitId = evalCircuit->getCircuitId();
	SharedCircuit circuit = circuitManager.getCircuit(circuitId);
	if (!circuit) {
		// logError("Circuit with ID {} not found", "Evaluator::getInputPortId", circuitId);
		return std::nullopt;
	}
	const BlockContainer* blockContainer = circuit->getBlockContainer();
	if (!blockContainer) {
		// logError("BlockContainer not found", "Evaluator::getInputPortId");
		return std::nullopt;
	}
	const Block* block = blockContainer->getBlock(portPosition);
	if (!block) {
		// logError("Block not found at position {}", "Evaluator::getInputPortId", portPosition.toString());
		return std::nullopt;
	}
	Position blockPosition = block->getPosition();
	std::optional<CircuitNode> node = evalCircuit->getNode(blockPosition);
	if (!node.has_value()) {
		// logError("Node not found at position {}", "Evaluator::getConnectionPoint", blockPosition.toString());
		return std::nullopt;
	}
	if (node->isIC()) {
		// logError("Cannot get connection point for IC at position {} yet", "Evaluator::getConnectionPoint", blockPosition.toString());
		return std::nullopt;
	}
	std::optional<connection_port_id_t> portId = getPortId(evalCircuit->getCircuitId(), blockPosition, portPosition, direction);
	if (!portId.has_value()) {
		// logError("Port not found at position {}", "Evaluator::getConnectionPoint", portPosition.toString());
		return std::nullopt;
	}
	return EvalConnectionPoint(node->getId(), portId.value());
}

void Evaluator::edit_moveBlock(SimPauseGuard& pauseGuard, eval_circuit_id_t evalCircuitId, DiffCache& diffCache, Position curPosition, Rotation curRotation, Position newPosition, Rotation newRotation) {
	EvalCircuit* evalCircuit = evalCircuitContainer.getCircuit(evalCircuitId);
	if (!evalCircuit) {
		logError("EvalCircuit with id {} not found", "Evaluator::edit_moveBlock", evalCircuitId);
		return;
	}
	evalCircuit->moveNode(curPosition, newPosition);
}

const EvalAddressTree Evaluator::buildAddressTree() const {
	return buildAddressTree(0);
}

const EvalAddressTree Evaluator::buildAddressTree(eval_circuit_id_t evalCircuitId) const {
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

std::optional<middle_id_t> Evaluator::getMiddleId(const Address& address) const {
	return getMiddleId(0, address);
}

logic_state_t Evaluator::getState(const Address& address) {
	std::optional<eval_circuit_id_t> evalCircuitIdOpt = evalCircuitContainer.traverseToTopLevelIC(address);
	if (!evalCircuitIdOpt.has_value()) {
		logError("Failed to traverse to top-level IC for address {}", "Evaluator::getState", address.toString());
		return logic_state_t::UNDEFINED; // or some other default state
	}
	std::optional<EvalConnectionPoint> connectionPointOpt = getConnectionPoint(evalCircuitIdOpt.value(), address.getPosition(address.size() - 1), Direction::OUT);
	if (!connectionPointOpt.has_value()) {
		logError("Connection point not found for address {}", "Evaluator::getState", address.toString());
		return logic_state_t::UNDEFINED; // or some other default state
	}
	return evalSimulator.getState(connectionPointOpt.value());
}

void Evaluator::setState(const Address& address, logic_state_t state) {
	std::optional<eval_circuit_id_t> evalCircuitIdOpt = evalCircuitContainer.traverseToTopLevelIC(address);
	if (!evalCircuitIdOpt.has_value()) {
		logError("Failed to traverse to top-level IC for address {}", "Evaluator::setState", address.toString());
		return;
	}
	std::optional<EvalConnectionPoint> connectionPointOpt = getConnectionPoint(evalCircuitIdOpt.value(), address.getPosition(address.size() - 1), Direction::OUT);
	if (connectionPointOpt.has_value()) {
		evalSimulator.setState(connectionPointOpt.value(), state);
		return;
	}
	std::optional<EvalConnectionPoint> connectionPointOptIn = getConnectionPoint(evalCircuitIdOpt.value(), address.getPosition(address.size() - 1), Direction::IN);
	if (connectionPointOptIn.has_value()) {
		evalSimulator.setState(connectionPointOptIn.value(), state);
		return;
	}
	std::optional<middle_id_t> middleIdOpt = getMiddleId(evalCircuitIdOpt.value(), address);
	if (middleIdOpt.has_value()) {
		EvalConnectionPoint connectionPoint(middleIdOpt.value(), 0);
		evalSimulator.setState(connectionPoint, state);
		return;
	}
	logError("Failed to get connection point for address {}", "Evaluator::setState", address.toString());
}

std::vector<logic_state_t> Evaluator::getBulkStates(const std::vector<Address>& addresses, const Address& addressOrigin) {
	if (addresses.empty()) {
		return {};
	}
	eval_circuit_id_t startingPoint = evalCircuitContainer.traverseToTopLevelIC(addressOrigin);
	std::vector<EvalConnectionPoint> connectionPoints;
	connectionPoints.reserve(addresses.size());
	for (const Address& addr : addresses) {
		eval_circuit_id_t evalCircuitId = evalCircuitContainer.traverseToTopLevelIC(startingPoint, addr);
		std::optional<EvalConnectionPoint> connectionPointOpt = getConnectionPoint(evalCircuitId, addr.getPosition(addr.size() - 1), Direction::OUT);
		if (connectionPointOpt.has_value()) {
			connectionPoints.push_back(connectionPointOpt.value());
		} else {
			std::optional<middle_id_t> middleIdOpt = getMiddleId(evalCircuitId, addr);
			if (middleIdOpt.has_value()) {
				connectionPoints.emplace_back(middleIdOpt.value(), 0);
			} else {
				logError("Failed to get connection point for address {}", "Evaluator::getBulkStates", addr.toString());
				connectionPoints.emplace_back(0, 0);
			}
		}
	}
	return evalSimulator.getStates(connectionPoints);
}