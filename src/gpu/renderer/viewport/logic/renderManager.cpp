#include "renderManager.h"

#include "sharedLogic/logicRenderingUtils.h"
#include "backend/evaluator/evaluator.h"
#include "backend/circuit/circuit.h"

CircuitRenderManager::CircuitRenderManager(Circuit* circuit) : circuit(circuit) {
	circuit->connectListener(this, [this](DifferenceSharedPtr diff, circuit_id_t circuitId) {if (circuitId == this->circuit->getCircuitId()) addDifference(diff); });
}

CircuitRenderManager::~CircuitRenderManager() {
	circuit->disconnectListener(this);
}

void CircuitRenderManager::connect(CircuitRenderer* circuitRenderer) {
	addDifference(circuit->getBlockContainer()->getCreationDifferenceShared(), std::set<CircuitRenderer*>{ circuitRenderer });
	connectedRenderers.insert(circuitRenderer);
}

void CircuitRenderManager::disconnect(CircuitRenderer* circuitRenderer) {
	connectedRenderers.erase(circuitRenderer);
}

void CircuitRenderManager::addDifference(DifferenceSharedPtr diff) {
	addDifference(diff, connectedRenderers);
}

void CircuitRenderManager::addDifference(DifferenceSharedPtr diff, const std::set<CircuitRenderer*>& renderers) {
#ifdef TRACY_PROFILER
	ZoneScoped;
#endif
	if (diff->clearsAll()) {
		renderedBlocks.clear();
		for (CircuitRenderer* renderer : renderers) {
			renderer->reset();
		}
		return;
	}
	
	for (CircuitRenderer* renderer : renderers) {
		renderer->startMakingEdits();
	}

	const BlockDataManager* blockDataManager = circuit->getBlockContainer()->getBlockDataManager();
	for (const auto& modification : diff->getModifications()) {
		const auto& [modificationType, modificationData] = modification;

		
			switch (modificationType) {
			case Difference::ModificationType::PLACE_BLOCK:
			{
				const auto& [position, rotation, blockType] = std::get<Difference::block_modification_t>(modificationData);

				for (CircuitRenderer* renderer : renderers) {
					Position statePosition = Position(1000000, 1000000);
					if (blockType < BlockType::CUSTOM) {
						if (blockType == BlockType::TRISTATE_BUFFER) statePosition = position + rotateVectorWithArea(Vector(0, 1), Vector(1, 2), rotation);
						else statePosition = position;
					}
					renderer->addBlock(blockType, position, blockDataManager->getBlockSize(blockType, rotation), rotation, statePosition);
				}
				renderedBlocks.emplace(position, RenderedBlock(blockType, rotation));
				break;
			}
			case Difference::ModificationType::REMOVED_BLOCK:
			{
				const auto& [position, rotation, blockType] = std::get<Difference::block_modification_t>(modificationData);

				for (CircuitRenderer* renderer : renderers) {
					renderer->removeBlock(position);
				}
				auto iter = renderedBlocks.find(position);
				if (iter == renderedBlocks.end()) {
					logError("Could not find block at {} to remove.", "CircuitRenderManager", position);
					continue;
				}
				if (iter->second.connectionsToOtherBlock.size() != 0) {
					logError("Trying to remove block at {} that still has {} connections.", "CircuitRenderManager", position, iter->second.connectionsToOtherBlock.size());
					continue;
				}
				renderedBlocks.erase(iter);
				break;
			}
			case Difference::ModificationType::CREATED_CONNECTION:
			{
				const auto& [outputBlockPosition, outputPosition, inputBlockPosition, inputPosition] = std::get<Difference::connection_modification_t>(modificationData);

				// uses position of output and input CELLS fed into offset function
				std::pair<Position, Position> newConnection = { outputPosition, inputPosition };

				auto outputIter = renderedBlocks.find(outputBlockPosition);
				if (outputIter == renderedBlocks.end()) {
					logError("Could not find block at {} to add output connection to.", "CircuitRenderManager", outputBlockPosition);
				}
				outputIter->second.connectionsToOtherBlock.emplace(newConnection, inputBlockPosition);

				// only need both if it is a different block
				if (outputBlockPosition != inputBlockPosition) {
					auto inputIter = renderedBlocks.find(inputBlockPosition);
					if (inputIter == renderedBlocks.end()) {
						logError("Could not find block at {} to add input connection to.", "CircuitRenderManager", inputBlockPosition);
					}
					inputIter->second.connectionsToOtherBlock.emplace(newConnection, outputBlockPosition);
					for (CircuitRenderer* renderer : renderers) {
						renderer->addWire(newConnection, { getOutputOffset(outputIter->second.type, outputIter->second.rotation), getInputOffset(inputIter->second.type, inputIter->second.rotation) });
					}
				} else {
					for (CircuitRenderer* renderer : renderers) {
						renderer->addWire(newConnection, { getOutputOffset(outputIter->second.type, outputIter->second.rotation), getInputOffset(outputIter->second.type, outputIter->second.rotation) });
					}
				}
				break;
			}
			case Difference::ModificationType::REMOVED_CONNECTION:
			{
				const auto& [outputBlockPosition, outputPosition, inputBlockPosition, inputPosition] = std::get<Difference::connection_modification_t>(modificationData);
				for (CircuitRenderer* renderer : renderers) {
					renderer->removeWire({ outputPosition, inputPosition });
				}

				auto outputIter = renderedBlocks.find(outputBlockPosition);
				if (outputIter == renderedBlocks.end()) {
					logError("Could not find block at {} to remove output connection from.", "CircuitRenderManager", outputBlockPosition);
				}
				outputIter->second.connectionsToOtherBlock.erase({ outputPosition, inputPosition });

				// only need both if it is a different block
				if (outputBlockPosition != inputBlockPosition) {
					auto inputIter = renderedBlocks.find(inputBlockPosition);
					if (inputIter == renderedBlocks.end()) {
						logError("Could not find block at {} to remove input connection from.", "CircuitRenderManager", inputBlockPosition);
					}
					inputIter->second.connectionsToOtherBlock.erase({ outputPosition, inputPosition });
				}
				break;
			}
			case Difference::ModificationType::MOVE_BLOCK:
			{
				const auto& [curPosition, curRotation, newPosition, newRotation] = std::get<Difference::move_modification_t>(modificationData);

				if (curPosition == newPosition && curRotation == newRotation) continue;

				auto iter = renderedBlocks.find(curPosition);
				if (iter == renderedBlocks.end()) {
					logError("Could not find block at {} to move.", "CircuitRenderManager", curPosition);
					continue;
				}

				if (curPosition != newPosition) {
					auto newIter = renderedBlocks.emplace(newPosition, std::move(iter->second)).first;
					renderedBlocks.erase(iter);
					iter = newIter;
				}

				iter->second.rotation = newRotation;

				// MOVE BLOCK
				for (CircuitRenderer* renderer : renderers) {
					renderer->moveBlock(curPosition, newPosition, newRotation, blockDataManager->getBlockSize(iter->second.type, newRotation));
				}

				Vector blockSize = blockDataManager->getBlockSize(iter->second.type, curRotation);
				Rotation rotationAmount = subRotations(newRotation, curRotation);

				// MOVE CONNECTIONS
				std::unordered_map<std::pair<Position, Position>, Position> oldConnectionsToOtherBlock = std::move(iter->second.connectionsToOtherBlock);
				iter->second.connectionsToOtherBlock.clear();

				for (auto& [posPair, otherBlockPos] : oldConnectionsToOtherBlock) {
					for (CircuitRenderer* renderer : renderers) {
						renderer->removeWire(posPair);
					}
					if (otherBlockPos == curPosition) {
						Position outputPos = newPosition + rotateVectorWithArea(posPair.first - curPosition, blockSize, rotationAmount);
						Position inputPos = newPosition + rotateVectorWithArea(posPair.second - curPosition, blockSize, rotationAmount);
						for (CircuitRenderer* renderer : renderers) {
							renderer->addWire({ outputPos, inputPos }, { getOutputOffset(iter->second.type, newRotation), getInputOffset(iter->second.type, newRotation) });
						}
						iter->second.connectionsToOtherBlock.emplace(std::make_pair(outputPos, inputPos), newPosition);
					} else {
						auto otherIter = renderedBlocks.find(otherBlockPos);
						if (otherIter == renderedBlocks.end()) {
							logError("Could not find block at {} to move connection.", "CircuitRenderManager", otherBlockPos);
							continue;
						}
						otherIter->second.connectionsToOtherBlock.erase(posPair);
						bool isInput = posPair.second.withinArea(curPosition, curPosition + blockSize - Vector(1));
						if (isInput) {
							Position inputPos = newPosition + rotateVectorWithArea(posPair.second - curPosition, blockSize, rotationAmount);
							for (CircuitRenderer* renderer : renderers) {
								renderer->addWire({ posPair.first, inputPos }, { getOutputOffset(otherIter->second.type, otherIter->second.rotation), getInputOffset(iter->second.type, newRotation) });
							}
							iter->second.connectionsToOtherBlock.emplace(std::make_pair(posPair.first, inputPos), otherBlockPos);
							otherIter->second.connectionsToOtherBlock.emplace(std::make_pair(posPair.first, inputPos), newPosition);
						} else {
							Position outputPos = newPosition + rotateVectorWithArea(posPair.first - curPosition, blockSize, rotationAmount);
							for (CircuitRenderer* renderer : renderers) {
								renderer->addWire({ outputPos, posPair.second }, { getOutputOffset(iter->second.type, newRotation), getInputOffset(otherIter->second.type, otherIter->second.rotation) });
							}
							iter->second.connectionsToOtherBlock.emplace(std::make_pair(outputPos, posPair.second), otherBlockPos);
							otherIter->second.connectionsToOtherBlock.emplace(std::make_pair(outputPos, posPair.second), newPosition);
						}
					}
				}
			}
		}
	}


	for (CircuitRenderer* renderer : renderers) {
		renderer->stopMakingEdits();
	}
}
