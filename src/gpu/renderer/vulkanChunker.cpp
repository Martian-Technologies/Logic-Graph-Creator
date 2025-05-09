#include "vulkanChunker.h"

#include "gpu/renderer/vulkanChunkRenderer.h"
#include "gpu/vulkanManager.h"

// VulkanChunkAllocation
// =========================================================================================================

VulkanChunkAllocation::VulkanChunkAllocation(const std::unordered_map<Position, RenderedBlock>& blocks, std::set<RenderedWire>& wires) {
	// TODO - should pre-allocate buffers with size and pool them
	// TODO - should abstract this function
	
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
		blockBuffer = createBuffer(blockBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO);
		vmaCopyMemoryToAllocation(Vulkan::getAllocator(), blockVertices.data(), blockBuffer->allocation, 0, blockBufferSize);
	}

	if (wires.size() > 0) {
		// Generate wire vertices
		std::vector<WireVertex> wireVertices;
		wireVertices.reserve(wires.size() * 6);
		for (const auto& wire : wires) {
		}
		// upload wire vertices
		numWireVertices = wireVertices.size();
		size_t wireBufferSize = sizeof(WireVertex) * numWireVertices;
		wireBuffer = createBuffer(wireBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO);
		vmaCopyMemoryToAllocation(Vulkan::getAllocator(), wireVertices.data(), wireBuffer->allocation, 0, wireBufferSize);
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

const unsigned int CHUNK_SIZE = 25;
Position getChunk(Position in) {
	in.x = std::floor(in.x / (float)CHUNK_SIZE) * (int)CHUNK_SIZE;
	in.y = std::floor(in.y / (float)CHUNK_SIZE) * (int)CHUNK_SIZE;
	return in;
}

void VulkanChunker::setCircuit(Circuit* circuit) {
	std::lock_guard<std::mutex> lock(mux);
	
	this->circuit = circuit;
	
	// remove all existing chunking data
	chunks.clear();

	if (circuit) {
		// partition blocks into chunks
		for (const auto& block : *(circuit->getBlockContainer())) {
			Position position = block.second.getPosition();
			chunks[getChunk(position)].getBlocksForUpdating()[position] = RenderedBlock(block.second.type(), block.second.getRotation(), block.second.size().dx, block.second.size().dy);
		}

		// allocate vulkan buffer for all chunks
		// TODO - this should be improved to happen deferred
		for (auto& chunk : chunks) {
			chunk.second.updateAllocation();
		}
	}
}

void VulkanChunker::updateCircuit(DifferenceSharedPtr diff) {
	std::lock_guard<std::mutex> lock(mux);

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
			
			break;
		}
		case Difference::ModificationType::CREATED_CONNECTION:
		{
			const auto& [outputBlockPosition, outputPosition, inputBlockPosition, inputPosition] = std::get<Difference::connection_modification_t>(modificationData);
			
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
	for (Position chunk : chunksToUpdate) {
		chunks[chunk].updateAllocation();
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
			auto chunk = chunks.find({chunkX, chunkY});
			if (chunk != chunks.end()) {
				auto allocation = chunk->second.getAllocation();
				if (allocation.has_value()) seen.push_back(allocation.value());
			}
		}
	}

	return seen;
}
