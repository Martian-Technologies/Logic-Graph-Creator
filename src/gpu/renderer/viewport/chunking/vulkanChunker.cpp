#include "vulkanChunker.h"

#include "../sharedLogic/logicRenderingUtils.h"
#include "backend/position/position.h"
#include "logging/logging.h"

const int CHUNK_SIZE = 64;
cord_t getChunk(cord_t in) {
	return std::floor((float)in / (float)CHUNK_SIZE) * (int)CHUNK_SIZE;
}
Position getChunk(Position in) {
	in.x = getChunk(in.x);
	in.y = getChunk(in.y);
	return in;
}

#include "gpu/renderer/viewport/blockTextureManager.h"

// VulkanChunkAllocation
// =========================================================================================================

VulkanChunkAllocation::VulkanChunkAllocation(VulkanDevice* device, RenderedBlocks& blocks, RenderedWires& wires) {
	// TODO - should pre-allocate buffers with size and pool them
	// TODO - maybe should use smaller size coordinates with one big offset

	// Maps "state position" to a position in the address list so that multiple objects can index the same array
	std::unordered_map<Position, size_t> posToAddressIdx;

	// Generate block instances
	if (blocks.size() > 0) {
		std::vector<BlockInstance> blockInstances;
		blockInstances.reserve(blocks.size());
		for (const auto& block : blocks) {
			Position blockPosition = block.first;
			Vec2 uvOrigin = device->getBlockTextureManager()->getTileset().getTopLeftUV(block.second.blockType + 1, 0);

			BlockInstance instance;
			instance.pos = glm::vec2(blockPosition.x, blockPosition.y);
			instance.sizeX = block.second.realWidth;
			instance.sizeY = block.second.realHeight;
			instance.rotation = block.second.rotation;
			instance.texX = uvOrigin.x;

			blockInstances.push_back(instance);

			// blocks are added to state array
			posToAddressIdx[blockPosition] = relativeAdresses.size();
			relativeAdresses.push_back(blockPosition);
		}

		// upload block vertices
		numBlockInstances = blockInstances.size();
		size_t blockBufferSize = sizeof(BlockInstance) * numBlockInstances;
		blockBuffer = createBuffer(device, blockBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
		vmaCopyMemoryToAllocation(device->getAllocator(), blockInstances.data(), blockBuffer->allocation, 0, blockBufferSize);
	}

	// Generate wire vertices
	if (wires.size() > 0) {
		std::vector<WireInstance> wireInstances;
		wireInstances.reserve(wires.size());
		for (const auto& wire : wires) {

			// get wire's index in state buffer
			size_t stateIdx;
			auto itr = posToAddressIdx.find(wire.first.first);
			// check if wire state position is already in the state array
			if (itr != posToAddressIdx.end()) {
				stateIdx = itr->second;
			} else {
				// add address to state buffer
				stateIdx = relativeAdresses.size();
				relativeAdresses.push_back(wire.second.relativeStateAddress);
				posToAddressIdx[wire.first.first] = stateIdx;
			}

			WireInstance instance;
			instance.pointA = glm::vec2(wire.second.start.x, wire.second.start.y);
			instance.pointB = glm::vec2(wire.second.end.x, wire.second.end.y);
			instance.stateIndex = stateIdx;

			wireInstances.push_back(instance);
		}

		// upload wire vertices
		numWireInstances = wireInstances.size();
		size_t wireBufferSize = sizeof(WireInstance) * numWireInstances;
		wireBuffer = createBuffer(device, wireBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
		vmaCopyMemoryToAllocation(device->getAllocator(), wireInstances.data(), wireBuffer->allocation, 0, wireBufferSize);
	}

	if (!relativeAdresses.empty()) {
		// Create state buffer
		size_t stateBufferSize = relativeAdresses.size() * sizeof(logic_state_t);
		stateBuffer.emplace();
		stateBuffer->init(device, stateBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
		std::vector<logic_state_t> defaultStates(relativeAdresses.size(), logic_state_t::HIGH);
	}
}

VulkanChunkAllocation::~VulkanChunkAllocation() {
	if (blockBuffer.has_value()) destroyBuffer(blockBuffer.value());
	if (wireBuffer.has_value()) destroyBuffer(wireBuffer.value());
	if (stateBuffer.has_value()) stateBuffer->cleanup();
}

// ChunkChain
// =========================================================================================================

void Chunk::rebuildAllocation(VulkanDevice* device) {
	if (!blocks.empty() || !wires.empty()) { // if we have data to upload
		// allocate new date
		std::shared_ptr<VulkanChunkAllocation> newAllocation = std::make_unique<VulkanChunkAllocation>(device, blocks, wires);
		// replace currently allocating data
		if (currentlyAllocating.has_value()) {
			gbJail.push_back(currentlyAllocating.value());
		}
		currentlyAllocating = newAllocation;

	} else { // if we have no data to upload
		// drop currently allocating, send to gay baby jail
		if (currentlyAllocating.has_value()) {
			gbJail.push_back(currentlyAllocating.value());
		}
		currentlyAllocating.reset();

		// drop newest allocation
		newestAllocation.reset();
	}

	allocationDirty = false;
}

std::optional<std::shared_ptr<VulkanChunkAllocation>> Chunk::getAllocation() {
	// if the buffer has finished allocating, replace the newest with it
	if (currentlyAllocating.has_value() && currentlyAllocating.value()->isAllocationComplete()) {
		newestAllocation = currentlyAllocating;
		currentlyAllocating.reset();
	}

	annihilateOrphanGBs();

	// get the newest allocation if there is one
	return newestAllocation;
}

void Chunk::annihilateOrphanGBs() {
	// erase all GBs that are complete and not pointed to
	auto itr = gbJail.begin();
	while (itr != gbJail.end()) {
		if ((*itr)->isAllocationComplete()) {
			itr = gbJail.erase(itr);
		} else itr++;
	}
}

// VulkanChunker
// =========================================================================================================

VulkanChunker::VulkanChunker(VulkanDevice* device)
	: device(device) {

}

void VulkanChunker::setCircuit(Circuit* circuit) {
	std::lock_guard<std::mutex> lock(mux);

	this->circuit = circuit;

	// remove all existing chunking data
	chunks.clear();

	if (circuit) {
		// TODO - this should be improved to happen deferred
		Difference creationDiff = circuit->getBlockContainer()->getCreationDifference();
		updateCircuit(&creationDiff);
	}
}

void VulkanChunker::updateCircuit(DifferenceSharedPtr diff) {
	std::lock_guard<std::mutex> lock(mux);

	updateCircuit(diff.get());
}

void VulkanChunker::updateCircuit(Difference* diff) {
	std::unordered_set<Position> chunksToUpdate;

	for (const auto& modification : diff->getModifications()) {
		const auto& [modificationType, modificationData] = modification;
		const BlockDataManager* blockDataManager = circuit->getBlockContainer()->getBlockDataManager();

		switch (modificationType) {
		case Difference::ModificationType::REMOVED_BLOCK:
		{
			const auto& [position, rotation, blockType] = std::get<Difference::block_modification_t>(modificationData);

			// Remove block
			Position chunk = getChunk(position);
			chunks[chunk].getRenderedBlocks().erase(position);
			chunksToUpdate.insert(chunk);

			break;
		}
		case Difference::ModificationType::PLACE_BLOCK:
		{
			const auto& [position, rotation, blockType] = std::get<Difference::block_modification_t>(modificationData);

			// Add block
			Position chunk = getChunk(position);
			Vector blockSize = blockDataManager->getBlockSize(blockType, rotation);
			chunks[chunk].getRenderedBlocks()[position] = RenderedBlock(blockType, rotation, blockSize.dx, blockSize.dy);
			chunksToUpdate.insert(chunk);

			break;
		}
		case Difference::ModificationType::REMOVED_CONNECTION:
		{
			const auto& [outputBlockPosition, outputPosition, inputBlockPosition, inputPosition] = std::get<Difference::connection_modification_t>(modificationData);

			// TODO - technically removing connections could use a faster algorithm since it doesn't need to know the input/output points of each chunk
			Rotation outputRotation = chunks[getChunk(outputBlockPosition)].getRenderedBlocks()[outputBlockPosition].rotation;
			Rotation inputRotation = chunks[getChunk(inputBlockPosition)].getRenderedBlocks()[inputBlockPosition].rotation;
			updateWireOverChunks(outputPosition, outputRotation, inputPosition, inputRotation, false, chunksToUpdate);

			// remove from block connection registry
			blockToConnections[inputBlockPosition].erase({ outputPosition, inputPosition });
			blockToConnections[outputBlockPosition].erase({ outputPosition, inputPosition });

			break;
		}
		case Difference::ModificationType::CREATED_CONNECTION:
		{
			const auto& [outputBlockPosition, outputPosition, inputBlockPosition, inputPosition] = std::get<Difference::connection_modification_t>(modificationData);

			Rotation outputRotation = chunks[getChunk(outputBlockPosition)].getRenderedBlocks()[outputBlockPosition].rotation;
			Rotation inputRotation = chunks[getChunk(inputBlockPosition)].getRenderedBlocks()[inputBlockPosition].rotation;
			updateWireOverChunks(outputPosition, outputRotation, inputPosition, inputRotation, true, chunksToUpdate);

			// add to block connection registry
			blockToConnections[inputBlockPosition][{outputPosition, inputPosition}] = { outputBlockPosition, true };
			blockToConnections[outputBlockPosition][{outputPosition, inputPosition}] = { inputBlockPosition, false };

			break;
		}
		case Difference::ModificationType::MOVE_BLOCK:
		{
			const auto& [curPosition, curRotation, newPosition, newRotation] = std::get<Difference::move_modification_t>(modificationData);

			if (curPosition == newPosition) continue;

			// MOVE BLOCK

			// get chunk
			Position curChunk = getChunk(curPosition);
			Position newChunk = getChunk(newPosition);
			chunksToUpdate.insert(curChunk);
			chunksToUpdate.insert(newChunk);

			// find original block
			auto& curBlocksForUpdating = chunks[curChunk].getRenderedBlocks();
			auto itr = curBlocksForUpdating.find(curPosition);
			if (itr == curBlocksForUpdating.end()) {
				logError("Renderer could not find block to move.", "Vulkan");
				continue;
			}

			// add block to new position
			auto& newBlocksForUpdating = chunks[newChunk].getRenderedBlocks();
			itr->second.rotation = newRotation;
			newBlocksForUpdating[newPosition] = itr->second;
			BlockType blockType = itr->second.blockType;
			// remove block from original chunk
			curBlocksForUpdating.erase(itr);

			// MOVE CONNECTIONS
			Rotation rotation = chunks[getChunk(newPosition)].getRenderedBlocks()[newPosition].rotation;
			Vector moveVector = newPosition - curPosition;

			// Move all input connections
			for (const auto& end : blockToConnections[curPosition]) {
				if (end.second.otherBlock != curPosition) {
					Rotation otherRotation = chunks[getChunk(end.second.otherBlock)].getRenderedBlocks()[end.second.otherBlock].rotation;

					// remove old wire
					blockToConnections[end.second.otherBlock].erase(end.first); // remove other's entry
					if (end.second.isInput) updateWireOverChunks(end.first.first, otherRotation, end.first.second, rotation, false, chunksToUpdate);
					else updateWireOverChunks(end.first.first, rotation, end.first.second, otherRotation, false, chunksToUpdate);

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
					if (end.second.isInput) updateWireOverChunks(newConnection.first, otherRotation, newConnection.second, rotation, true, chunksToUpdate);
					else updateWireOverChunks(newConnection.first, rotation, newConnection.second, otherRotation, true, chunksToUpdate);
				} else {
					// if we are a self connection

					// remove old wire (we don't need to remove from registry, because we do that for ourselves later)
					updateWireOverChunks(end.first.first, rotation, end.first.second, rotation, false, chunksToUpdate);

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
					updateWireOverChunks(newConnection.first, rotation, newConnection.second, rotation, true, chunksToUpdate);
				}


			}
			blockToConnections.erase(curPosition); // remove all of our old entries

			break;
		}
		default:
			break;
		}
	}

	// rebuild all modified chunks
	for (const Position& chunk : chunksToUpdate) {
		chunks[chunk].rebuildAllocation(device);
	}
}

std::vector<ChunkIntersection> VulkanChunker::getChunkIntersections(FPosition start, FPosition end) {
	// the JACLWANICBSBTIOPICG (copyright 2025, released under MIT license) also known as DDA (or Ben is lying)
	// Thank you One Lone Coder and lodev

	std::vector<ChunkIntersection> intersections;

	FVector diff = end - start;
	float distance = diff.length();
	FVector dir = diff / distance;

	Position chunk = getChunk(start.snap());

	//length of ray from one x or y-side to next x or y-side
	FVector rayUnitStepSize = FVector( sqrt(1 + (dir.dy / dir.dx) * (dir.dy / dir.dx)), sqrt(1 + (dir.dx / dir.dy) * (dir.dx / dir.dy)) ) * CHUNK_SIZE;

	// starting conditions
	FVector rayLength1D;
	Vector step;
	if (dir.dx < 0)
	{
		step.dx = -CHUNK_SIZE;
		rayLength1D.dx = ((start.x - float(chunk.x)) / float(CHUNK_SIZE)) * rayUnitStepSize.dx;
	} else {
		step.dx = CHUNK_SIZE;
		rayLength1D.dx = ((float(chunk.x + CHUNK_SIZE) - start.x) / float(CHUNK_SIZE)) * rayUnitStepSize.dx;
	}
	if (dir.dy < 0)
	{
		step.dy = -CHUNK_SIZE;
		rayLength1D.dy = ((start.y - float(chunk.y)) / float(CHUNK_SIZE)) * rayUnitStepSize.dy;
	} else {
		step.dy = CHUNK_SIZE;
		rayLength1D.dy = ((float(chunk.y + CHUNK_SIZE) - start.y) / float(CHUNK_SIZE)) * rayUnitStepSize.dy;
	}

	FPosition currentPos = start;
	float currentDistance = 0.0f;
	while (currentDistance < distance) {

		Position oldChunk = chunk;

		// decide which direction we walk
		if (rayLength1D.dx < rayLength1D.dy)
		{
			chunk.x += step.dx;
			currentDistance = rayLength1D.dx;
			rayLength1D.dx += rayUnitStepSize.dx;

		} else if (rayLength1D.dx > rayLength1D.dy) {
			chunk.y += step.dy;
			currentDistance = rayLength1D.dy;
			rayLength1D.dy += rayUnitStepSize.dy;
		} else {
			chunk += step;
			currentDistance = rayLength1D.dx;
			rayLength1D.dx += rayUnitStepSize.dx;
			rayLength1D.dy += rayUnitStepSize.dy;
		}

		// clamp overshoot
		if (currentDistance > distance) {
			currentDistance = distance;
		}

		// add point at current distance
		FPosition newPos = start + dir * currentDistance;
		intersections.push_back({oldChunk, currentPos, newPos});
		currentPos = newPos;
	}

	return intersections;
}

void VulkanChunker::updateWireOverChunks(Position start, Rotation startRotation, Position end, Rotation endRotation, bool add, std::unordered_set<Position>& chunksToUpdate) {

	// TODO - handle same position
	std::pair<Position, Position> wire = { start, end };

	// get state address for connection
	Address relativeAddress = start; // TODO - use actual address

	// get start line position
	FPosition startF = start.free() + getOutputOffset(startRotation);
	FPosition endF = end.free() + getInputOffset(endRotation);

	// update all wires in intersecting chunks
	if (add) {
		for (const ChunkIntersection& intersection : getChunkIntersections(startF, endF)) {
			chunks[intersection.chunk].getRenderedWires()[wire] = { intersection.start, intersection.end, relativeAddress };
			wireToChunks[{start, end}].push_back(intersection.chunk);
			chunksToUpdate.insert(intersection.chunk);
		}
	} else {
		auto itr = wireToChunks.find({start, end});
		for (Position p : itr->second) {
			chunks[p].getRenderedWires().erase(wire);
			chunksToUpdate.insert(p);
		}
		wireToChunks.erase(itr);
	}
}

std::vector<std::shared_ptr<VulkanChunkAllocation>> VulkanChunker::getAllocations(Position min, Position max) {
	std::lock_guard<std::mutex> lock(mux);

	// get chunk bounds with padding for large blocks (this will technically goof if there are blocks larger than chunk size)
	min = getChunk(min) - Vector(CHUNK_SIZE, CHUNK_SIZE);
	max = getChunk(max) + Vector(CHUNK_SIZE, CHUNK_SIZE);

	// go through each chunk in view and collect it if it exists and has an allocation
	std::vector<std::shared_ptr<VulkanChunkAllocation>> seen;
	for (cord_t chunkX = min.x; chunkX <= max.x; chunkX += CHUNK_SIZE) {
		for (cord_t chunkY = min.y; chunkY <= max.y; chunkY += CHUNK_SIZE) {
			auto chunk = chunks.find({ chunkX, chunkY });
			if (chunk != chunks.end()) {
				auto allocation = chunk->second.getAllocation();
				if (allocation.has_value()) seen.push_back(allocation.value());
			}
		}
	}

	return seen;
}
