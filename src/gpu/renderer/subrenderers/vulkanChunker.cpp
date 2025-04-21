#include "vulkanChunker.h"

#include "vulkanChunkRenderer.h"
#include "gpu/vulkanInstance.h"
#include "backend/circuitView/renderer/tileSet.h"

const int CHUNK_SIZE = 25;
cord_t getChunk(cord_t in) {
	return std::floor( (float)in / (float)CHUNK_SIZE ) * (int)CHUNK_SIZE;
}
Position getChunk(Position in) {
	in.x = getChunk(in.x);
	in.y = getChunk(in.y);
	return in;
}

// VulkanChunkAllocation
// =========================================================================================================

TileSetInfo blockTileSet(256, 15, 4);

VulkanChunkAllocation::VulkanChunkAllocation(RenderedBlocks& blocks, RenderedWires& wires) {
	// TODO - should pre-allocate buffers with size and pool them
	// TODO - maybe should use smaller size coordinates with one big offset
	
	// Generate block vertices
	if (blocks.size() > 0) {
		std::vector<BlockVertex> blockVertices;
		blockVertices.reserve(blocks.size() * 6);
		for (const auto& block : blocks) {
			Position blockPosition = block.first;
			Vec2 uvOrigin = blockTileSet.getTopLeftUV(block.second.blockType + 1, 0);
			Vec2 uvSize = blockTileSet.getCellUVSize();

			glm::vec2 topLeftUV = {uvOrigin.x, uvOrigin.y};
			glm::vec2 topRightUV = {uvOrigin.x + uvSize.x, uvOrigin.y};
			glm::vec2 bottomLeftUV = {uvOrigin.x, uvOrigin.y + uvSize.y};
			glm::vec2 bottomRightUV = {uvOrigin.x + uvSize.x, uvOrigin.y + uvSize.y};
			
			BlockVertex v1 = {{blockPosition.x + block.second.realWidth, blockPosition.y + block.second.realHeight}, bottomRightUV};
			BlockVertex v2 = {{blockPosition.x, blockPosition.y + block.second.realHeight}, bottomLeftUV};
			BlockVertex v3 = {{blockPosition.x, blockPosition.y}, topLeftUV};
			BlockVertex v4 = {{blockPosition.x, blockPosition.y}, topLeftUV};
			BlockVertex v5 = {{blockPosition.x + block.second.realWidth, blockPosition.y}, topRightUV};
			BlockVertex v6 = {{blockPosition.x + block.second.realWidth, blockPosition.y + block.second.realHeight}, bottomRightUV};
			blockVertices.push_back(v1);
			blockVertices.push_back(v2);
			blockVertices.push_back(v3);
			blockVertices.push_back(v4);
			blockVertices.push_back(v5);
			blockVertices.push_back(v6);
		}
		
		// upload block vertices
		numBlockVertices = blockVertices.size();
		size_t blockBufferSize = sizeof(BlockVertex) * numBlockVertices;
		blockBuffer = createBuffer(blockBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
		vmaCopyMemoryToAllocation(VulkanInstance::get().getAllocator(), blockVertices.data(), blockBuffer->allocation, 0, blockBufferSize);
	}

	
	// Generate wire vertices
	if (wires.size() > 0) {
		std::vector<WireVertex> wireVertices;
		wireVertices.reserve(wires.size() * 6);
		for (const auto& wire : wires) {

			constexpr float WIRE_WIDTH = 0.03f;
			
			// get normalized direction
			FVector dir = wire.second.end - wire.second.start;
			dir /= dir.length();

			// direction rotated clockwise (I'm thinking right as in a vector pointed in (1, 1) rotated to (1, -1))
			FVector right(-dir.dy, dir.dx);
			right *= WIRE_WIDTH; // apply thickness of wire

			// reused position
			FPosition vertexPos;

			// first triangle
			vertexPos = wire.second.start + right;
			wireVertices.emplace_back(glm::vec2(vertexPos.x, vertexPos.y));
			vertexPos = wire.second.start - right;
			wireVertices.emplace_back(glm::vec2(vertexPos.x, vertexPos.y));
			vertexPos = wire.second.end + right;
			wireVertices.emplace_back(glm::vec2(vertexPos.x, vertexPos.y));
			// second triangle
			vertexPos = wire.second.start - right;
			wireVertices.emplace_back(glm::vec2(vertexPos.x, vertexPos.y));
			vertexPos = wire.second.end - right;
			wireVertices.emplace_back(glm::vec2(vertexPos.x, vertexPos.y));
			vertexPos = wire.second.end + right;
			wireVertices.emplace_back(glm::vec2(vertexPos.x, vertexPos.y));
		}
		
		// upload wire vertices
		numWireVertices = wireVertices.size();
		size_t wireBufferSize = sizeof(WireVertex) * numWireVertices;
		wireBuffer = createBuffer(wireBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
		vmaCopyMemoryToAllocation(VulkanInstance::get().getAllocator(), wireVertices.data(), wireBuffer->allocation, 0, wireBufferSize);
	}
}

VulkanChunkAllocation::~VulkanChunkAllocation() {
	if (blockBuffer.has_value()) destroyBuffer(blockBuffer.value());
	if (wireBuffer.has_value()) destroyBuffer(wireBuffer.value());
}

// ChunkChain
// =========================================================================================================

void ChunkChain::updateAllocation() {
	if (!blocks.empty() || !wires.empty()) { // if we have data to upload
		// allocate new date
		std::shared_ptr<VulkanChunkAllocation> newAllocation = std::make_unique<VulkanChunkAllocation>(blocks, wires);
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
			Rotation outputRotation = circuit->getBlockContainer()->getBlock(outputBlockPosition)->getRotation();
			Rotation inputRotation = circuit->getBlockContainer()->getBlock(inputBlockPosition)->getRotation();
			updateChunksOverConnection(outputPosition, outputRotation, inputPosition, inputRotation, false, chunksToUpdate);
			
			break;
		}
		case Difference::ModificationType::CREATED_CONNECTION:
		{
			const auto& [outputBlockPosition, outputPosition, inputBlockPosition, inputPosition] = std::get<Difference::connection_modification_t>(modificationData);

			Rotation outputRotation = circuit->getBlockContainer()->getBlock(outputBlockPosition)->getRotation();
			Rotation inputRotation = circuit->getBlockContainer()->getBlock(inputBlockPosition)->getRotation();
			updateChunksOverConnection(outputPosition, outputRotation, inputPosition, inputRotation, true, chunksToUpdate);
			
			break;
		}
		case Difference::ModificationType::MOVE_BLOCK:
		{
			const auto& [curPosition, newPosition] = std::get<Difference::move_modification_t>(modificationData);
			if (curPosition == newPosition) continue;

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
			
			break;
		}
		default:
			break;
		}
	}

	// reallocate all modified chunks
	for (const Position& chunk : chunksToUpdate) {
		chunks[chunk].updateAllocation();
	}
}

constexpr float edgeDistance = 0.48f;
constexpr float sideShift = 0.25f;

FVector VulkanChunker::getOutputOffset(Rotation rotation) {
	FVector offset = { 0.5, 0.5 };
	
	switch (rotation) {
	case Rotation::ZERO: offset += { edgeDistance, sideShift }; break;
	case Rotation::NINETY: offset += { -sideShift, edgeDistance }; break;
	case Rotation::ONE_EIGHTY: offset += { -edgeDistance, -sideShift }; break;
	case Rotation::TWO_SEVENTY: offset += { sideShift, -edgeDistance }; break;
	}
	
	return offset;
}

FVector VulkanChunker::getInputOffset(Rotation rotation) {
	FVector offset = { 0.5, 0.5 };
	
	switch (rotation) {
	case Rotation::ZERO: offset += { -edgeDistance, -sideShift }; break;
	case Rotation::NINETY: offset += { sideShift, -edgeDistance }; break;
	case Rotation::ONE_EIGHTY: offset += { edgeDistance, sideShift }; break;
	case Rotation::TWO_SEVENTY: offset += { -sideShift, edgeDistance }; break;
	}
	
	return offset;
}

void VulkanChunker::updateChunksOverConnection(Position start, Rotation startRotation, Position end, Rotation endRotation, bool add, std::unordered_set<Position>& chunksToUpdate) {
	
	// TODO - handle same position

	// the Jamisonian algorithm for calculating (lines which aren't necessary inside of a chunk but still bound to them)'s input and output points of intersections with chunks on a grid
	// the JACLWANICBSBTIOPICG (copyright 2025, released under MIT license)
	
	// honestly it's a little jank and it may break if the offsets leave the block far enough
	// and it won't work at all for curved wires, we will prob redesign system for that
	
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

		// move in the desired direction
		if (moveHorizontal) {
			// get next position and connect chunk
			f_cord_t newX = currentX + dstToNextChunkBorderX;
			f_cord_t newY = currentY + yChangeForHorizontal;

			if (add) chunks[currentChunk].getWiresForUpdating()[{start, end}] = {{currentX, currentY}, {newX, newY}};
			else chunks[currentChunk].getWiresForUpdating().erase({start, end});
			chunksToUpdate.insert(currentChunk);

			// update positions
			currentX = newX;
			currentY = newY;

			dstToNextChunkBorderX = CHUNK_SIZE * dirX;
			dstToNextChunkBorderY -= yChangeForHorizontal;

			currentChunk.x += CHUNK_SIZE * dirX;
				
		} else {
			// get next position and connect chunk
			f_cord_t newX = currentX + xChangeForVertical;
			f_cord_t newY = currentY + dstToNextChunkBorderY;

			if (add) chunks[currentChunk].getWiresForUpdating()[{start, end}] = {{currentX, currentY}, {newX, newY}};
			else chunks[currentChunk].getWiresForUpdating().erase({start, end});
			chunksToUpdate.insert(currentChunk);

			// update positions
			currentX = newX;
			currentY = newY;

			dstToNextChunkBorderX -= xChangeForVertical;
			dstToNextChunkBorderY = CHUNK_SIZE * dirY;

			currentChunk.y += CHUNK_SIZE * dirY;
		}
	}

	// connect last chunk
	if (add) chunks[currentChunk].getWiresForUpdating()[{start, end}] = {{currentX, currentY}, {endX, endY}};
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
