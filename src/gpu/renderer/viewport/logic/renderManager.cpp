#include "renderManager.h"

#ifdef TRACY_PROFILER
#include <tracy/Tracy.hpp>
#endif

#include "sharedLogic/logicRenderingUtils.h"
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
			const auto& [position, orientation, blockType] = std::get<Difference::block_modification_t>(modificationData);

			for (CircuitRenderer* renderer : renderers) {
				Position statePosition = Position(1000000, 1000000);
				if (blockType < BlockType::CUSTOM) {
					if (blockType == BlockType::TRISTATE_BUFFER) statePosition = position + orientation.transformVectorWithArea(Vector(0, 1), Size(1, 2));
					else statePosition = position;
				}
				renderer->addBlock(blockType, position, blockDataManager->getBlockSize(blockType, orientation), orientation, statePosition);
			}
			renderedBlocks.emplace(position, RenderedBlock(blockType, orientation));
			break;
		}
		case Difference::ModificationType::REMOVED_BLOCK:
		{
			const auto& [position, orientation, blockType] = std::get<Difference::block_modification_t>(modificationData);

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
					renderer->addWire(newConnection, { getOutputOffset(outputIter->second.type, outputIter->second.orientation), getInputOffset(inputIter->second.type, inputIter->second.orientation) });
				}
			} else {
				for (CircuitRenderer* renderer : renderers) {
					renderer->addWire(newConnection, { getOutputOffset(outputIter->second.type, outputIter->second.orientation), getInputOffset(outputIter->second.type, outputIter->second.orientation) });
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
			const auto& [curPosition, curOrientation, newPosition, newOrientation] = std::get<Difference::move_modification_t>(modificationData);

			if (curPosition == newPosition && curOrientation == newOrientation) continue;

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

			iter->second.orientation = newOrientation;

			// MOVE BLOCK
			for (CircuitRenderer* renderer : renderers) {
				renderer->moveBlock(curPosition, newPosition, newOrientation, blockDataManager->getBlockSize(iter->second.type, newOrientation));
			}

			Size blockSize = blockDataManager->getBlockSize(iter->second.type, curOrientation);
			Orientation transformAmount = newOrientation.relativeTo(curOrientation);

			// MOVE CONNECTIONS
			std::unordered_map<std::pair<Position, Position>, Position> oldConnectionsToOtherBlock = std::move(iter->second.connectionsToOtherBlock);
			iter->second.connectionsToOtherBlock.clear();

			for (auto& [posPair, otherBlockPos] : oldConnectionsToOtherBlock) {
				for (CircuitRenderer* renderer : renderers) {
					renderer->removeWire(posPair);
				}
				if (otherBlockPos == curPosition) {
					Position outputPos = newPosition + transformAmount.transformVectorWithArea(posPair.first - curPosition, blockSize);
					Position inputPos = newPosition + transformAmount.transformVectorWithArea(posPair.second - curPosition, blockSize);
					for (CircuitRenderer* renderer : renderers) {
						renderer->addWire({ outputPos, inputPos }, { getOutputOffset(iter->second.type, newOrientation), getInputOffset(iter->second.type, newOrientation) });
					}
					iter->second.connectionsToOtherBlock.emplace(std::make_pair(outputPos, inputPos), newPosition);
				} else {
					auto otherIter = renderedBlocks.find(otherBlockPos);
					if (otherIter == renderedBlocks.end()) {
						logError("Could not find block at {} to move connection.", "CircuitRenderManager", otherBlockPos);
						continue;
					}
					otherIter->second.connectionsToOtherBlock.erase(posPair);
					bool isInput = posPair.second.withinArea(curPosition, curPosition + blockSize.getLargestVectorInArea());
					if (isInput) {
						Position inputPos = newPosition + transformAmount.transformVectorWithArea(posPair.second - curPosition, blockSize);
						for (CircuitRenderer* renderer : renderers) {
							renderer->addWire({ posPair.first, inputPos }, { getOutputOffset(otherIter->second.type, otherIter->second.orientation), getInputOffset(iter->second.type, newOrientation) });
						}
						iter->second.connectionsToOtherBlock.emplace(std::make_pair(posPair.first, inputPos), otherBlockPos);
						otherIter->second.connectionsToOtherBlock.emplace(std::make_pair(posPair.first, inputPos), newPosition);
					} else {
						Position outputPos = newPosition + transformAmount.transformVectorWithArea(posPair.first - curPosition, blockSize);
						for (CircuitRenderer* renderer : renderers) {
							renderer->addWire({ outputPos, posPair.second }, { getOutputOffset(iter->second.type, newOrientation), getInputOffset(otherIter->second.type, otherIter->second.orientation) });
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
