#include "vulkanChunker.h"

#include "../sharedLogic/logicRenderingUtils.h"

const int CHUNK_SIZE = 25;
cord_t getChunk(cord_t in) {
	return std::floor( (float)in / (float)CHUNK_SIZE ) * (int)CHUNK_SIZE;
}
Position getChunk(Position in) {
	in.x = getChunk(in.x);
	in.y = getChunk(in.y);
	return in;
}

#include "gpu/renderer/viewport/blockTextureManager.h"

// VulkanChunkAllocation
// =========================================================================================================

VulkanChunkAllocation::VulkanChunkAllocation(VulkanDevice* device, RenderedBlocks& blocks, RenderedWires& wires)
{
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
			}
			else {
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

void ChunkChain::updateAllocation(VulkanDevice* device) {
	if (!blocks.empty() || !wires.empty()) { // if we have data to upload
		// allocate new date
		std::shared_ptr<VulkanChunkAllocation> newAllocation = std::make_unique<VulkanChunkAllocation>(device, blocks, wires);
		// replace currently allocating data
		if (currentlyAllocating.has_value()) {
			gbJail.push_back(currentlyAllocating.value());
		}
		currentlyAllocating = newAllocation;

	}
	else { // if we have no data to upload
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

std::optional<std::shared_ptr<VulkanChunkAllocation>> ChunkChain::getAllocation() {
	// if the buffer has finished allocating, replace the newest with it
	if (currentlyAllocating.has_value() && currentlyAllocating.value()->isAllocationComplete()) {
		newestAllocation = currentlyAllocating;
		currentlyAllocating.reset();
	}

	annihilateOrphanGBs();

	// get the newest allocation if there is one
	return newestAllocation;
}

void ChunkChain::annihilateOrphanGBs() {
	// erase all GBs that are complete and not pointed to
	auto itr = gbJail.begin();
	while (itr != gbJail.end()) {
		if ((*itr)->isAllocationComplete()) {
			itr = gbJail.erase(itr);
		}
		else itr++;
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
			chunks[chunk].getBlocksForUpdating().erase(position);
			chunksToUpdate.insert(chunk);
			
			break;
		}
		case Difference::ModificationType::PLACE_BLOCK:
		{
			const auto& [position, rotation, blockType] = std::get<Difference::block_modification_t>(modificationData);

			// Add block
			Position chunk = getChunk(position);
			Vector blockSize = blockDataManager->getBlockSize(blockType, rotation);
			chunks[chunk].getBlocksForUpdating()[position] = RenderedBlock(blockType, rotation, blockSize.dx, blockSize.dy);
			chunksToUpdate.insert(chunk);
			
			break;
		}
		case Difference::ModificationType::REMOVED_CONNECTION:
		{
			const auto& [outputBlockPosition, outputPosition, inputBlockPosition, inputPosition] = std::get<Difference::connection_modification_t>(modificationData);

			// TODO - technically removing connections could use a faster algorithm since it doesn't need to know the input/output points of each chunk
			Rotation outputRotation = chunks[getChunk(outputBlockPosition)].getBlocksForUpdating()[outputBlockPosition].rotation;
			Rotation inputRotation = chunks[getChunk(inputBlockPosition)].getBlocksForUpdating()[inputBlockPosition].rotation;
			updateChunksOverConnection(outputPosition, outputRotation, inputPosition, inputRotation, false, chunksToUpdate);

			// remove from block connection registry
			blockToConnections[inputBlockPosition].erase({outputPosition, inputPosition});
			blockToConnections[outputBlockPosition].erase({outputPosition, inputPosition});
			
			break;
		}
		case Difference::ModificationType::CREATED_CONNECTION:
		{
			const auto& [outputBlockPosition, outputPosition, inputBlockPosition, inputPosition] = std::get<Difference::connection_modification_t>(modificationData);

			Rotation outputRotation = chunks[getChunk(outputBlockPosition)].getBlocksForUpdating()[outputBlockPosition].rotation;
			Rotation inputRotation = chunks[getChunk(inputBlockPosition)].getBlocksForUpdating()[inputBlockPosition].rotation;
			updateChunksOverConnection(outputPosition, outputRotation, inputPosition, inputRotation, true, chunksToUpdate);

			// add to block connection registry
			blockToConnections[inputBlockPosition][{outputPosition, inputPosition}] = {outputBlockPosition, true};
			blockToConnections[outputBlockPosition][{outputPosition, inputPosition}] = {inputBlockPosition, false};
			
			break;
		}
		case Difference::ModificationType::MOVE_BLOCK:
		{
			const auto& [curPosition, newPosition] = std::get<Difference::move_modification_t>(modificationData);
			if (curPosition == newPosition) continue;

			// MOVE BLOCK
			
			// get chunk
			Position curChunk = getChunk(curPosition);
			Position newChunk = getChunk(newPosition);
			chunksToUpdate.insert(curChunk);
			chunksToUpdate.insert(newChunk);

			// find original block
			auto& curBlocksForUpdating = chunks[curChunk].getBlocksForUpdating();
			auto itr = curBlocksForUpdating.find(curPosition);
			if (itr == curBlocksForUpdating.end()) {
				logError("Renderer could not find block to move.", "Vulkan");
				continue;
			}

			// add block to new position
			auto& newBlocksForUpdating = chunks[newChunk].getBlocksForUpdating();
			newBlocksForUpdating[newPosition] = itr->second;

			// remove block from original chunk
			curBlocksForUpdating.erase(itr);

			// MOVE CONNECTIONS
			Rotation rotation = chunks[getChunk(newPosition)].getBlocksForUpdating()[newPosition].rotation;
			Vector moveVector = newPosition - curPosition;

			// Move all input connections
			for (const auto& end : blockToConnections[curPosition]) {
				if (end.second.otherBlock != curPosition) {
					Rotation otherRotation = chunks[getChunk(end.second.otherBlock)].getBlocksForUpdating()[end.second.otherBlock].rotation;

					// remove old wire
					blockToConnections[end.second.otherBlock].erase(end.first); // remove other's entry
					if (end.second.isInput) { updateChunksOverConnection(end.first.first, otherRotation, end.first.second, rotation, false, chunksToUpdate); }
					else { updateChunksOverConnection(end.first.first, rotation, end.first.second, otherRotation, false, chunksToUpdate); }

					// calculate new wire
					std::pair<Position, Position> newConnection;
					if (end.second.isInput) { newConnection = {end.first.first, end.first.second + moveVector}; }
					else { newConnection = {end.first.first + moveVector, end.first.second}; }

					// add new wire to registry
					blockToConnections[newPosition][newConnection] = { end.second.otherBlock, end.second.isInput };
					blockToConnections[end.second.otherBlock][newConnection] = { newPosition, !end.second.isInput };

					// add new wire to chunks
					if (end.second.isInput) { updateChunksOverConnection(newConnection.first, otherRotation, newConnection.second, rotation, true, chunksToUpdate); }
					else { updateChunksOverConnection(newConnection.first, rotation, newConnection.second, otherRotation, true, chunksToUpdate); }
				}
				else {
					// if we are a self connection

					// remove old wire (we don't need to remove from registry, because we do that for ourselves later)
					updateChunksOverConnection(end.first.first, rotation, end.first.second, rotation, false, chunksToUpdate);

					// calculate new wire
					std::pair<Position, Position> newConnection = { end.first.first + moveVector, end.first.second + moveVector };

					// add new wire to registry
					blockToConnections[newPosition][newConnection] = { newPosition, end.second.isInput };

					// add new wire to chunks
					updateChunksOverConnection(newConnection.first, rotation, newConnection.second, rotation, true, chunksToUpdate);
				}
				
				
			}
			blockToConnections.erase(curPosition); // remove all of our old entries
			
			break;
		}
		default:
			break;
		}
	}

	// reallocate all modified chunks
	for (const Position& chunk : chunksToUpdate) {
		chunks[chunk].updateAllocation(device);
	}
}

void VulkanChunker::updateChunksOverConnection(Position start, Rotation startRotation, Position end, Rotation endRotation, bool add, std::unordered_set<Position>& chunksToUpdate) {
	
	// TODO - handle same position

	// the Jamisonian algorithm for calculating (lines which aren't necessary inside of a chunk but still bound to them)'s input and output points of intersections with chunks on a grid
	// the JACLWANICBSBTIOPICG (copyright 2025, released under MIT license)
	
	// honestly it's a little jank and it may break if the offsets leave the block far enough
	// and it won't work at all for curved wires, we will prob redesign system for that

	// get state address for connection
	Address relativeAddress;
	relativeAddress = start; // TODO - use actual address
	
	// chunk positions
	Position currentChunk = getChunk(start);
	Position endChunk = getChunk(end);

	// start line position
	FVector startOffset = getOutputOffset(startRotation);
	f_cord_t currentX = start.x + startOffset.dx;
	f_cord_t currentY = start.y + startOffset.dy;

	// end line position
	FVector endOffset = getInputOffset(endRotation);
	f_cord_t endX = end.x + endOffset.dx;
	f_cord_t endY = end.y + endOffset.dy;

	// line change and slopes
	f_cord_t dx = endX - currentX;
	cord_t dirX = (dx > 0) ? 1 : -1;
	f_cord_t dy = endY - currentY;
	cord_t dirY = (dy > 0) ? 1 : -1;
	f_cord_t slope = dy/dx;
	f_cord_t iSlope = dx/dy;

	// calculate the distance to the next chunk border from starting X
	f_cord_t relativeChunkPercentX = (currentX / CHUNK_SIZE);
	relativeChunkPercentX -= (float)currentChunk.x / CHUNK_SIZE; // make it relative
	f_cord_t dstToNextChunkBorderX = (dirX > 0 ? 1.0f - relativeChunkPercentX : -relativeChunkPercentX) * CHUNK_SIZE;

	// calculate the distance to the next chunk border from starting Y
	f_cord_t relativeChunkPercentY = (currentY / CHUNK_SIZE);
	relativeChunkPercentY -= (float)currentChunk.y / CHUNK_SIZE; // make it relative
	f_cord_t dstToNextChunkBorderY = (dirY > 0 ? 1.0f - relativeChunkPercentY : -relativeChunkPercentY) * CHUNK_SIZE;

	// go along line connecting chunks until last one
	while (currentChunk != endChunk) {
		bool moveHorizontal;
		
		f_cord_t yChangeForHorizontal = dstToNextChunkBorderX * slope;
		f_cord_t xChangeForVertical = dstToNextChunkBorderY * iSlope;

		// figure out which direction we should move
		if (currentChunk.x == endChunk.x) { moveHorizontal = false; } // check if we have purely vertical to go
		else if (currentChunk.y == endChunk.y) { moveHorizontal = true; } // check if we have purely horizontal to go
		else {
			// compare movement distances, pick shortest
			
			// get line distance (squared) for moving to vertical (x) chunk edge
			f_cord_t horizontalTravelCost = (dstToNextChunkBorderX * dstToNextChunkBorderX) + (yChangeForHorizontal * yChangeForHorizontal);

			// get line distance (squared) for moving to horizontal (y) chunk edge
			f_cord_t verticalTravelCost = (dstToNextChunkBorderY * dstToNextChunkBorderY) + (xChangeForVertical * xChangeForVertical);

			moveHorizontal = horizontalTravelCost < verticalTravelCost;
		}

		// update connection
		f_cord_t newX = currentX;
		f_cord_t newY = currentY;
		if (moveHorizontal) {
			newX += dstToNextChunkBorderX;
			newY += yChangeForHorizontal;
		}
		else {
			newX += xChangeForVertical;
			newY += dstToNextChunkBorderY;
		}
		if (add) chunks[currentChunk].getWiresForUpdating()[{start, end}] = {{currentX, currentY}, {newX, newY}, relativeAddress};
		else chunks[currentChunk].getWiresForUpdating().erase({start, end});
		chunksToUpdate.insert(currentChunk);	
		
		// move in the desired direction
		if (moveHorizontal) {
			// update positions
			currentX = newX;
			currentY = newY;

			dstToNextChunkBorderX = CHUNK_SIZE * dirX;
			dstToNextChunkBorderY -= yChangeForHorizontal;

			currentChunk.x += CHUNK_SIZE * dirX;
				
		} else {
			// update positions
			currentX = newX;
			currentY = newY;

			dstToNextChunkBorderX -= xChangeForVertical;
			dstToNextChunkBorderY = CHUNK_SIZE * dirY;

			currentChunk.y += CHUNK_SIZE * dirY;
		}
	}

	// connect last chunk
	if (add) chunks[currentChunk].getWiresForUpdating()[{start, end}] = {{currentX, currentY}, {endX, endY}, relativeAddress};
	else chunks[currentChunk].getWiresForUpdating().erase({start, end});
	chunksToUpdate.insert(currentChunk);
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
			auto chunk = chunks.find({chunkX, chunkY});
			if (chunk != chunks.end()) {
				auto allocation = chunk->second.getAllocation();
				if (allocation.has_value()) seen.push_back(allocation.value());
			}
		}
	}

	return seen;
}
