#include "evaluator.h"

Evaluator::Evaluator(evaluator_id_t evaluatorId, CircuitManager& circuitManager, circuit_id_t circuitId, DataUpdateEventManager* dataUpdateEventManager)
	: evaluatorId(evaluatorId),
	circuitManager(circuitManager),
	evalCircuitContainer(),
	receiver(dataUpdateEventManager),
	evalConfig(),
	middleIdProvider(),
	simulatorOptimizer(middleIdProvider) {
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
	logWarning("implement simulator locking", "Evaluator::makeEdit");
	DiffCache diffCache(circuitManager);
	for (eval_circuit_id_t evalCircuitId = 0; evalCircuitId < evalCircuitContainer.size(); evalCircuitId++) {
		if (evalCircuitContainer.getCircuitId(evalCircuitId) == circuitId) {
			makeEditInPlace(evalCircuitId, difference, diffCache);
		}
	}
}

void Evaluator::makeEditInPlace(eval_circuit_id_t evalCircuitId, DifferenceSharedPtr difference, DiffCache& diffCache) {
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
				edit_removeBlock(evalCircuitId, diffCache, position, rotation, blockType);
				break;
			}
			case Difference::ModificationType::PLACE_BLOCK: {
				const auto& [position, rotation, blockType] = std::get<Difference::block_modification_t>(modificationData);
				edit_placeBlock(evalCircuitId, diffCache, position, rotation, blockType);
				break;
			}
			case Difference::ModificationType::MOVE_BLOCK: {
				const auto& [curPosition, curRotation, newPosition, newRotation] = std::get<Difference::move_modification_t>(modificationData);
				edit_moveBlock(evalCircuitId, diffCache, curPosition, curRotation, newPosition, newRotation);
				break;
			}
			case Difference::ModificationType::REMOVED_CONNECTION: {
				const auto& [outputBlockPosition, outputPosition, inputBlockPosition, inputPosition] = std::get<Difference::connection_modification_t>(modificationData);
				edit_removeConnection(evalCircuitId, diffCache, outputBlockPosition, outputPosition, inputBlockPosition, inputPosition);
				break;
			}
			case Difference::ModificationType::CREATED_CONNECTION: {
				const auto& [outputBlockPosition, outputPosition, inputBlockPosition, inputPosition] = std::get<Difference::connection_modification_t>(modificationData);
				edit_createConnection(evalCircuitId, diffCache, outputBlockPosition, outputPosition, inputBlockPosition, inputPosition);
				break;
			}
		}
	}
}

void Evaluator::edit_removeBlock(eval_circuit_id_t evalCircuitId, DiffCache& diffCache, Position position, Rotation rotation, BlockType type) {
	logWarning("not implemented yet", "Evaluator::edit_removeBlock");
}

void Evaluator::edit_placeBlock(eval_circuit_id_t evalCircuitId, DiffCache& diffCache, Position position, Rotation rotation, BlockType type) {
	logWarning("not implemented yet", "Evaluator::edit_placeBlock");
}

void Evaluator::edit_removeConnection(eval_circuit_id_t evalCircuitId, DiffCache& diffCache, Position outputBlockPosition, Position outputPosition, Position inputBlockPosition, Position inputPosition) {
	logWarning("not implemented yet", "Evaluator::edit_removeConnection");
}

void Evaluator::edit_createConnection(eval_circuit_id_t evalCircuitId, DiffCache& diffCache, Position outputBlockPosition, Position outputPosition, Position inputBlockPosition, Position inputPosition) {
	logWarning("not implemented yet", "Evaluator::edit_createConnection");
}

void Evaluator::edit_moveBlock(eval_circuit_id_t evalCircuitId, DiffCache& diffCache, Position curPosition, Rotation curRotation, Position newPosition, Rotation newRotation) {
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