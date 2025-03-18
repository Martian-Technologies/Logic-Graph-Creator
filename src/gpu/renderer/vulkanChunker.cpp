#include "vulkanChunker.h"

#include "gpu/renderer/vulkanBlockRenderer.h"
#include "gpu/vulkanManager.h"

// VulkanChunkAllocation
// =========================================================================================================

VulkanChunkAllocation::VulkanChunkAllocation(const std::vector<RenderedBlock>& blocks) {
	// TODO - should pre-allocate buffers with size and pool them
	// TODO - should abstract this function
	
	// Generate vertices
	std::vector<BlockVertex> vertices;
	vertices.reserve(blocks.size() * 3);
	for (const auto& block : blocks) {
		Position blockPosition = block.position;
		BlockVertex v1 = {{blockPosition.x + block.realWidth, blockPosition.y + block.realHeight}, {0.0f, 0.0f, 1.0f}};
		BlockVertex v2 = {{blockPosition.x, blockPosition.y + block.realHeight}, {0.0f, 1.0f, 0.0f}};
		BlockVertex v3 = {{blockPosition.x, blockPosition.y}, {1.0f, 0.0f, 0.0f}};
		vertices.push_back(v1);
		vertices.push_back(v2);
		vertices.push_back(v3);
	}

	// upload vertices to buffer
	numVertices = vertices.size();
	size_t vertexBufferSize = sizeof(BlockVertex) * numVertices;
	buffer = createBuffer(vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO);
	vmaCopyMemoryToAllocation(Vulkan::getAllocator(), vertices.data(), buffer.allocation, 0, vertexBufferSize);
}

VulkanChunkAllocation::~VulkanChunkAllocation() {
	destroyBuffer(buffer);
}

// ChunkChain
// =========================================================================================================

void ChunkChain::updateAllocation() {
	std::shared_ptr<VulkanChunkAllocation> newAllocation = std::make_unique<VulkanChunkAllocation>(upToData);
	
	// replace currently allocating
	if (currentlyAllocating.has_value()) {
		gbJail.push_back(currentlyAllocating.value());
	}
	currentlyAllocating = newAllocation;
	allocationDirty = false;
}

std::optional<std::shared_ptr<VulkanChunkAllocation>> ChunkChain::getAllocation() {
	// if the buffer has finished allocating, replace the newest with it
	if (currentlyAllocating.has_value() && currentlyAllocating.value()->isAllocationComplete()) {
		if (newestAllocation.has_value()) gbJail.push_back(newestAllocation.value());
		newestAllocation = currentlyAllocating;
		currentlyAllocating.reset();
	}

	annihilateOrphanGBs();

	// get the newest allocation if there is one
	return newestAllocation;
}

void ChunkChain::annihilateOrphanGBs() {
	return;
	// erase all GBs that are complete and not pointed to
	auto itr = gbJail.begin();
	while (itr != gbJail.end()) {
		if (itr->use_count() <= 1 && (*itr)->isAllocationComplete()) {
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
	this->circuit = circuit;
	
	// remove all existing chunking data
	chunks.clear();

	if (circuit) {
		// partition blocks into chunks
		for (const auto& block : *(circuit->getBlockContainer())) {
			chunks[getChunk(block.second.getPosition())].getBlocksForUpdating().push_back({block.second.type(), block.second.getPosition(), block.second.getRotation(), block.second.width(), block.second.height()});
		}

		// allocate vulkan buffer for all chunks
		// TODO - this should be improved to happen deferred
		for (auto& chunk : chunks) {
			chunk.second.updateAllocation();
		}
	}
}

void VulkanChunker::updateCircuit(DifferenceSharedPtr diff) {
	// TODO - very temp (rebuild whole thing (defeats the purpose))
	setCircuit(circuit);
}

std::vector<std::shared_ptr<VulkanChunkAllocation>> VulkanChunker::getAllocations(Position min, Position max) {
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
