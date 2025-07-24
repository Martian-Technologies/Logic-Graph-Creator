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
		logError("Circuit with ID {} not found in evaluator constructor", "Evaluator", circuitId);
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
		logError("EvalCircuit with id {} not found in Evaluator", "Evaluator::makeEditInPlace", evalCircuitId);
		return;
	}
	SharedCircuit circuit = circuitManager.getCircuit(circuitId.value());
	if (!circuit) {
		logError("Circuit with id {} not found in Evaluator", "Evaluator::makeEditInPlace", circuitId.value());
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
	logWarning("not implemented yet", "Evaluator::edit_removeBlock");
}

void Evaluator::edit_placeBlock(SimPauseGuard& pauseGuard, eval_circuit_id_t evalCircuitId, DiffCache& diffCache, Position position, Rotation rotation, BlockType type) {
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
		logError("Unsupported BlockType {} in Evaluator::edit_placeBlock", "Evaluator::edit_placeBlock", type);
		return;
	}
	middle_id_t gateId = middleIdProvider.getNewId();
	evalSimulator.addGate(pauseGuard, gateType, gateId);
	EvalCircuit* evalCircuit = evalCircuitContainer.getCircuit(evalCircuitId);
	if (!evalCircuit) {
		logError("EvalCircuit with id {} not found in Evaluator", "Evaluator::edit_placeBlock", evalCircuitId);
		return;
	}
	CircuitNode node = CircuitNode::fromMiddle(gateId);
	evalCircuit->setNode(position, node);
}

void Evaluator::edit_removeConnection(SimPauseGuard& pauseGuard, eval_circuit_id_t evalCircuitId, DiffCache& diffCache, Position outputBlockPosition, Position outputPosition, Position inputBlockPosition, Position inputPosition) {
	logWarning("not implemented yet", "Evaluator::edit_removeConnection");
}

void Evaluator::edit_createConnection(SimPauseGuard& pauseGuard, eval_circuit_id_t evalCircuitId, DiffCache& diffCache, Position outputBlockPosition, Position outputPosition, Position inputBlockPosition, Position inputPosition) {
	logWarning("not implemented yet", "Evaluator::edit_createConnection");
}

void Evaluator::edit_moveBlock(SimPauseGuard& pauseGuard, eval_circuit_id_t evalCircuitId, DiffCache& diffCache, Position curPosition, Rotation curRotation, Position newPosition, Rotation newRotation) {
	logWarning("not implemented yet", "Evaluator::edit_moveBlock");
}

const EvalAddressTree Evaluator::buildAddressTree() const {
	return buildAddressTree(0);
}

const EvalAddressTree Evaluator::buildAddressTree(eval_circuit_id_t evalCircuitId) const {
	EvalCircuit* evalCircuit = evalCircuitContainer.getCircuit(evalCircuitId);
	if (!evalCircuit) {
		logError("EvalCircuit with id {} not found in Evaluator", "Evaluator::buildAddressTree", evalCircuitId);
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
	std::optional<CircuitNode> node = evalCircuitContainer.traverse(startingPoint, address);
	if (!node.has_value()) {
		logError("Invalid address {} in Evaluator::getMiddleId", "Evaluator::getMiddleId", address.toString());
		return std::nullopt;
	}
	if (node->isIC()) {
		logError("Address {} does not point to an IC in Evaluator::getMiddleId", "Evaluator::getMiddleId", address.toString());
		return std::nullopt;
	}
	return node->getId();
}

std::optional<middle_id_t> Evaluator::getMiddleId(const Address& address) const {
	return getMiddleId(0, address);
}

logic_state_t Evaluator::getState(const Address& address) {
	std::optional<middle_id_t> middleIdOpt = getMiddleId(address);
	if (!middleIdOpt.has_value()) {
		logError("Failed to get middle ID for address {} in Evaluator::getState", "Evaluator::getState", address.toString());
		return logic_state_t::UNDEFINED; // or some other default state
	}
	logic_state_t state = evalSimulator.getState(middleIdOpt.value());
	return state;
}

void Evaluator::setState(const Address& address, logic_state_t state) {
	std::optional<middle_id_t> middleIdOpt = getMiddleId(address);
	if (!middleIdOpt.has_value()) {
		logError("Failed to get middle ID for address {} in Evaluator::setState", "Evaluator::setState", address.toString());
		return;
	}
	evalSimulator.setState(middleIdOpt.value(), state);
}

std::vector<logic_state_t> Evaluator::getBulkStates(const std::vector<Address>& addresses, const Address& addressOrigin) {
	if (addresses.empty()) {
		return {};
	}
	eval_circuit_id_t startingPoint = 0;
	if (addressOrigin.size() != 0) {
		std::optional<CircuitNode> originNode = evalCircuitContainer.traverse(addressOrigin);
		if (!originNode.has_value() || !originNode->isIC()) {
			logError("Invalid address origin {} in Evaluator::getBulkStates", "Evaluator::getBulkStates", addressOrigin.toString());
			return std::vector<logic_state_t>(addresses.size(), logic_state_t::UNDEFINED);
		}
		startingPoint = originNode->getId();
	}
	std::vector<middle_id_t> middleIds;
	middleIds.reserve(addresses.size());
	for (const Address& addr : addresses) {
		std::optional<middle_id_t> middleIdOpt = getMiddleId(startingPoint, addr);
		if (middleIdOpt.has_value()) {
			middleIds.push_back(middleIdOpt.value());
		} else {
			middleIds.push_back(0); // or some other invalid ID
		}
	}
	return evalSimulator.getStates(middleIds);
}