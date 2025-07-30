#include "renderManager.h"

#include "backend/container/block/blockDefs.h"
#include "circuit.h"
#include "gpu/renderer/viewport/sharedLogic/logicRenderingUtils.h"

CircuitRenderManager::CircuitRenderManager(Circuit* circuit) : circuit(circuit) {

}

void CircuitRenderManager::getMeUpToSpeed(CircuitRenderer* renderer) {
	addDifference(circuit->getBlockContainer()->getCreationDifferenceShared(), std::set<CircuitRenderer*>{ renderer } );
}

void CircuitRenderManager::connect(CircuitRenderer* circuitRenderer) {
	connectedRenderers.insert(circuitRenderer);
}

void CircuitRenderManager::disconnect(CircuitRenderer* circuitRenderer) {
	connectedRenderers.erase(circuitRenderer);
}

void CircuitRenderManager::addDifference(DifferenceSharedPtr diff) {
	addDifference(diff, connectedRenderers);
}

void CircuitRenderManager::addDifference(DifferenceSharedPtr diff, const std::set<CircuitRenderer*>& renderers) {
	for (CircuitRenderer* renderer : renderers) {
		renderer->startMakingEdits();
	}

	const BlockDataManager* blockDataManager = circuit->getBlockContainer()->getBlockDataManager();
	for (const auto& modification : diff->getModifications()) {
		const auto& [modificationType, modificationData] = modification;

		for (CircuitRenderer* renderer : renderers) {
			switch (modificationType) {
			case Difference::ModificationType::REMOVED_BLOCK:
			{
				const auto& [position, rotation, blockType] = std::get<Difference::block_modification_t>(modificationData);
	
				renderer->removeBlock(position);
				blockToRotation.erase(position);
				blockToType.erase(position);
	
				break;
			}
			case Difference::ModificationType::PLACE_BLOCK:
			{
				const auto& [position, rotation, blockType] = std::get<Difference::block_modification_t>(modificationData);
	
				renderer->addBlock(blockType, position, blockDataManager->getBlockSize(blockType, rotation), rotation);
				blockToRotation[position] = rotation;
				blockToType[position] = blockType;
	
				break;
			}
			case Difference::ModificationType::REMOVED_CONNECTION:
			{
				const auto& [outputBlockPosition, outputPosition, inputBlockPosition, inputPosition] = std::get<Difference::connection_modification_t>(modificationData);
	
				renderer->removeWire({outputPosition, inputPosition});
	
				// remove from block connection registry
				blockToConnections[inputBlockPosition].erase({ outputPosition, inputPosition });
				blockToConnections[outputBlockPosition].erase({ outputPosition, inputPosition });
	
				break;
			}
			case Difference::ModificationType::CREATED_CONNECTION:
			{
				const auto& [outputBlockPosition, outputPosition, inputBlockPosition, inputPosition] = std::get<Difference::connection_modification_t>(modificationData);
	
				// uses position of output and input CELLS fed into offset function
				std::pair<Position, Position> newConnection = {outputPosition, inputPosition};
				renderer->addWire(newConnection, { getOutputOffset(outputPosition, circuit), getInputOffset(inputPosition, circuit) }, getAddressOfConnection(newConnection));
	
				// add to block connection registry
				blockToConnections[inputBlockPosition][{outputPosition, inputPosition}] = { outputBlockPosition, true };
				blockToConnections[outputBlockPosition][{outputPosition, inputPosition}] = { inputBlockPosition, false };
	
				break;
			}
			case Difference::ModificationType::MOVE_BLOCK:
			{
				const auto& [curPosition, curRotation, newPosition, newRotation] = std::get<Difference::move_modification_t>(modificationData);
	
				if (curPosition == newPosition) continue;

				// Update bookkeeping
				blockToRotation.erase(curPosition);
				blockToRotation[newPosition] = newRotation;
				BlockType blockType = blockToType[curPosition];
				blockToType.erase(curPosition);
				blockToType[newPosition] = blockType;
	
				// MOVE BLOCK
				renderer->moveBlock(curPosition, newPosition, newRotation, blockDataManager->getBlockSize(blockType, newRotation));
	
				// MOVE CONNECTIONS
				Vector moveVector = newPosition - curPosition;
	
				// Move all input connections
				for (const auto& end : blockToConnections[curPosition]) {
					if (end.second.otherBlock != curPosition) {
						Rotation otherRotation = blockToRotation[end.second.otherBlock];
	
						// remove old wire
						blockToConnections[end.second.otherBlock].erase(end.first); // remove other's entry
						renderer->removeWire(end.first);
	
						// calculate new wire
						std::pair<connection_end_id_t, bool> idSucPair = (
							end.second.isInput ?
							(circuit->getBlockContainer()->getBlockDataManager()->getInputConnectionId(blockType, curRotation, end.first.second - curPosition)) :
							(circuit->getBlockContainer()->getBlockDataManager()->getOutputConnectionId(blockType, curRotation, end.first.first - curPosition))
						);
						Position newEndPos;
						if (idSucPair.second) {
							newEndPos = newPosition + circuit->getBlockContainer()->getBlockDataManager()->getConnectionVector(blockType, newRotation, idSucPair.first).first;
						} else {
							logError("Can not find connection ID for wire move.", "Vulkan");
							newEndPos = (end.second.isInput ? end.first.second : end.first.first) + moveVector;
						}
						std::pair<Position, Position> newConnection;
						if (end.second.isInput) newConnection = {end.first.first, newEndPos};
						else newConnection = {newEndPos, end.first.second};
	
						// add new wire to registry
						blockToConnections[newPosition][newConnection] = { end.second.otherBlock, end.second.isInput };
						blockToConnections[end.second.otherBlock][newConnection] = { newPosition, !end.second.isInput };
	
						// add new wire to chunks
						std::pair<FVector, FVector> socketOffsets;
						if (end.second.isInput) socketOffsets = { getOutputOffset(otherRotation), getInputOffset(newRotation) };
						else socketOffsets = { getOutputOffset(newRotation), getInputOffset(otherRotation) };

						renderer->addWire(newConnection, socketOffsets, getAddressOfConnection(newConnection));

					} else {
						// if we are a self connection
	
						// remove old wire (we don't need to remove from registry, because we do that for ourselves later)
						renderer->removeWire(end.first);
	
						// calculate new wire
						std::pair<connection_end_id_t, bool> inputIdSucPair = circuit->getBlockContainer()->getBlockDataManager()->getInputConnectionId(blockType, curRotation, end.first.second - curPosition);
						std::pair<connection_end_id_t, bool> outputIdSucPair = circuit->getBlockContainer()->getBlockDataManager()->getOutputConnectionId(blockType, curRotation, end.first.first - curPosition);
						Position inputNewEndPos;
						if (inputIdSucPair.second) {
							inputNewEndPos = newPosition + circuit->getBlockContainer()->getBlockDataManager()->getConnectionVector(blockType, newRotation, inputIdSucPair.first).first;
						} else {
							logError("Can not find connection ID for wire move.", "Vulkan");
							inputNewEndPos = end.first.second + moveVector;
						}
						Position outputNewEndPos;
						if (outputIdSucPair.second) {
							outputNewEndPos = newPosition + circuit->getBlockContainer()->getBlockDataManager()->getConnectionVector(blockType, newRotation, outputIdSucPair.first).first;
						} else {
							logError("Can not find connection ID for wire move.", "Vulkan");
							outputNewEndPos = end.first.first + moveVector;
						}
	
						std::pair<Position, Position> newConnection = { outputNewEndPos, outputNewEndPos };
	
						// add new wire to registry
						blockToConnections[newPosition][newConnection] = { newPosition, end.second.isInput };
	
						// add new wire to chunks
						renderer->addWire(newConnection, { getOutputOffset(newRotation), getInputOffset(newRotation) }, getAddressOfConnection(newConnection));
					}
	
				}
				// dearest ben herman, since blocks moving might be in the same position, this will probably FUCK SHIT UP
				blockToConnections.erase(curPosition); // remove all of our old entries
	
				break;
			}
			default:
				break;
			}

		}

	}

	for (CircuitRenderer* renderer : renderers) {
		renderer->stopMakingEdits();
	}

}

// Dearest Ben Herman.. please insert the bullshit inside of this function
Address CircuitRenderManager::getAddressOfConnection(std::pair<Position, Position> connection) {
	return connection.first;
}
