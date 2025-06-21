#include "evaluator.h"

Evaluator::Evaluator(evaluator_id_t evaluatorId, CircuitManager& circuitManager, circuit_id_t circuitId, DataUpdateEventManager* dataUpdateEventManager)
	: evaluatorId(evaluatorId), paused(true),
	targetTickrate(0),
	usingTickrate(false),
	circuitManager(circuitManager),
	circuitLattice(),
	receiver(dataUpdateEventManager) {
	setTickrate(40 * 60);
	const auto circuit = circuitManager.getCircuit(circuitId);
	circuitIds.push_back(circuitId);
	const auto blockContainer = circuit->getBlockContainer();
	const Difference difference = blockContainer->getCreationDifference();
	// receiver.linkFunction("blockDataRemoveConnection", std::bind(&Evaluator::removeCircuitIO, this, std::placeholders::_1));

	makeEdit(std::make_shared<Difference>(difference), circuitId);
}

void Evaluator::makeEdit(DifferenceSharedPtr difference, circuit_id_t circuitId) {
	logWarning("implement simulator locking", "Evaluator::makeEdit");
	DiffCache diffCache(circuitManager);
	for (int i = 0; i < circuitIds.size(); i++) {
		circuit_id_t currentCircuitId = circuitIds.at(i);
		if (currentCircuitId == circuitId) {
			makeEditInPlace(i, difference, diffCache);
		}
	}
}

void Evaluator::makeEditInPlace(lattice_coord_t layerIndex, DifferenceSharedPtr difference, DiffCache& diffCache) {
	SharedCircuit circuit = circuitManager.getCircuit(circuitIds.at(layerIndex));
	if (!circuit) {
		logError("Circuit with id {} not found in Evaluator", "Evaluator::makeEditInPlace", circuitIds.at(layerIndex));
		return;
	}
	const std::vector<Difference::Modification>& modifications = difference->getModifications();
	for (const Difference::Modification& modification : modifications) {
		const auto& [modificationType, modificationData] = modification;
		switch (modificationType) {
			case Difference::ModificationType::REMOVED_BLOCK: {
				const auto& [position, rotation, blockType] = std::get<Difference::block_modification_t>(modificationData);
				edit_removeBlock(layerIndex, position, rotation, blockType);
				break;
			}
			case Difference::ModificationType::PLACE_BLOCK: {
				const auto& [position, rotation, blockType] = std::get<Difference::block_modification_t>(modificationData);
				edit_placeBlock(layerIndex, position, rotation, blockType);
				break;
			}
			case Difference::ModificationType::MOVE_BLOCK: {
				const auto& [curPosition, curRotation, newPosition, newRotation] = std::get<Difference::move_modification_t>(modificationData);
				edit_moveBlock(layerIndex, curPosition, curRotation, newPosition, newRotation);
				break;
			}
			case Difference::ModificationType::REMOVED_CONNECTION: {
				const auto& [outputBlockPosition, outputPosition, inputBlockPosition, inputPosition] = std::get<Difference::connection_modification_t>(modificationData);
				edit_removeConnection(layerIndex, outputBlockPosition, outputPosition, inputBlockPosition, inputPosition);
				break;
			}
			case Difference::ModificationType::CREATED_CONNECTION: {
				const auto& [outputBlockPosition, outputPosition, inputBlockPosition, inputPosition] = std::get<Difference::connection_modification_t>(modificationData);
				edit_createConnection(layerIndex, outputBlockPosition, outputPosition, inputBlockPosition, inputPosition);
				break;
			}
			case Difference::ModificationType::SET_DATA: {
				const auto& [position, newData, oldData] = std::get<Difference::data_modification_t>(modificationData);
				edit_setData(layerIndex, position, newData, oldData);
				break;
			}
		}
	}
}