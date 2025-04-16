#include "vulkanChunker.h"

#include "vulkanChunkRenderer.h"
#include "gpu/vulkanInstance.h"

const unsigned int CHUNK_SIZE = 25;
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

VulkanChunkAllocation::VulkanChunkAllocation(RenderedBlocks& blocks, RenderedWires& wires) {
	// TODO - should pre-allocate buffers with size and pool them
	// TODO - maybe should use smaller size coordinates with one big offset
	
	// Generate block vertices
	if (blocks.size() > 0) {
		std::vector<BlockVertex> blockVertices;
		blockVertices.reserve(blocks.size() * 6);
		for (const auto& block : blocks) {
			Position blockPosition = block.first;
			BlockVertex v1 = {{blockPosition.x + block.second.realWidth, blockPosition.y + block.second.realHeight}, {0.0f, 0.0f, 1.0f}};
			BlockVertex v2 = {{blockPosition.x, blockPosition.y + block.second.realHeight}, {0.0f, 1.0f, 0.0f}};
			BlockVertex v3 = {{blockPosition.x, blockPosition.y}, {1.0f, 0.0f, 0.0f}};
			BlockVertex v4 = {{blockPosition.x, blockPosition.y}, {1.0f, 0.0f, 0.0f}};
			BlockVertex v5 = {{blockPosition.x + block.second.realWidth, blockPosition.y}, {1.0f, 0.0f, 0.0f}};
			BlockVertex v6 = {{blockPosition.x + block.second.realWidth, blockPosition.y + block.second.realHeight}, {1.0f, 0.0f, 0.0f}};
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
			updateChunksOverConnection(outputPosition, inputPosition, false, chunksToUpdate);
			
			break;
		}
		case Difference::ModificationType::CREATED_CONNECTION:
		{
			const auto& [outputBlockPosition, outputPosition, inputBlockPosition, inputPosition] = std::get<Difference::connection_modification_t>(modificationData);

			updateChunksOverConnection(outputPosition, inputPosition, true, chunksToUpdate);
			
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

void VulkanChunker::updateChunksOverConnection(Position start, Position end, bool add, std::unordered_set<Position>& chunksToUpdate) {
	
	// TODO - MAKE SURE not at same position better
	if (start == end) return;
	
	// chunk position (lines can technically be outside of chunk)
	Position currentChunk = getChunk(start);
	Position endChunk = getChunk(end);

	// start line position
	f_cord_t currentX = start.x;
	f_cord_t currentY = start.y;

	// line change and slopes
	f_cord_t dx = (float)end.x - (float)start.x;
	cord_t dirX = (dx > 0) ? 1 : -1;
	f_cord_t dy = (float)end.y - (float)start.y;
	cord_t dirY = (dy > 0) ? 1 : -1;
	f_cord_t slope = dy/dx;
	f_cord_t iSlope = dx/dy;

	f_cord_t relativeChunkX = (currentX / CHUNK_SIZE);
	relativeChunkX -= std::floor(relativeChunkX);
	f_cord_t dstToNextChunkBorderX = (dirX > 0 ? 1.0f - relativeChunkX : -relativeChunkX) * CHUNK_SIZE;
	
	f_cord_t relativeChunkY = (currentY / CHUNK_SIZE);
	relativeChunkY -= std::floor(relativeChunkY);
	f_cord_t dstToNextChunkBorderY = (dirY > 0 ? 1.0f - relativeChunkY : -relativeChunkY - 1.0f) * CHUNK_SIZE;

	// go along line connecting chunks until last one
	int itrs = 0;
	while (currentChunk != endChunk) {
		// get line distance (squared) for moving to vertical (x) chunk edge
		f_cord_t yChange = dstToNextChunkBorderX * slope;
		f_cord_t xTravelCost = (dstToNextChunkBorderX * dstToNextChunkBorderX) + (yChange * yChange);

		// get line distance (squared) for moving to horizontal (y) chunk edge
		f_cord_t xChange = dstToNextChunkBorderY * iSlope;
		f_cord_t yTravelCost = (dstToNextChunkBorderY * dstToNextChunkBorderY) + (xChange * xChange);

		if (xTravelCost < yTravelCost) {
			// get next position and connect chunk
			f_cord_t newX = currentX + dstToNextChunkBorderX;
			f_cord_t newY = currentY + yChange;

			if (add) chunks[currentChunk].getWiresForUpdating()[{start, end}] = {{currentX, currentY}, {newX, newY}};
			else chunks[currentChunk].getWiresForUpdating().erase({start, end});
			chunksToUpdate.insert(currentChunk);

			// update positions
			currentX = newX;
			currentY = newY;

			dstToNextChunkBorderX = CHUNK_SIZE * dirX;
			dstToNextChunkBorderY -= yChange;

			currentChunk.x += CHUNK_SIZE * dirX;
				
		} else {
			// get next position and connect chunk
			f_cord_t newX = currentX + xChange;
			f_cord_t newY = currentY + dstToNextChunkBorderY;

			if (add) chunks[currentChunk].getWiresForUpdating()[{start, end}] = {{currentX, currentY}, {newX, newY}};
			else chunks[currentChunk].getWiresForUpdating().erase({start, end});
			chunksToUpdate.insert(currentChunk);

			// update positions
			currentX = newX;
			currentY = newY;

			dstToNextChunkBorderX -= xChange;
			dstToNextChunkBorderY = CHUNK_SIZE * dirY;

			currentChunk.y += CHUNK_SIZE * dirY;
		}

		++itrs;
		logInfo(itrs);
	}

	// connect last chunk
	if (add) chunks[currentChunk].getWiresForUpdating()[{start, end}] = {{currentX, currentY}, {(float)end.x, (float)end.y}};
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
