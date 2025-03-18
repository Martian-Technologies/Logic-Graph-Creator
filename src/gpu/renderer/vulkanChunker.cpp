#include "vulkanChunker.h"

#include "gpu/renderer/vulkanBlockRenderer.h"
#include "gpu/vulkanManager.h"

// VulkanChunkAllocation
// =========================================================================================================

// ChunkChain
// =========================================================================================================

void ChunkChain::clean() {
	// iterates from back and finds first that is in used
	auto itr = allocationChain.rbegin();
	while (itr != allocationChain.rend() - 1) {
		if (itr->use_count() > 1) break;
		itr++;
	}
	// converts back to a forward iterator (which increments) and erases all from the end
	allocationChain.erase(itr.base(), allocationChain.end());
}

std::shared_ptr<VulkanChunkAllocation> ChunkChain::getAllocation() {
	return nullptr;
}

void ChunkChain::updateAllocation() {
	
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
		// TODO - this should be improved to happen deferred
		for (const auto& block : *(circuit->getBlockContainer())) {
			chunks[getChunk(block.second.getPosition())].getBlocks().push_back({block.second.type(), block.second.getPosition(), block.second.getRotation()});
		}

		for (auto& chunk : chunks) {
			chunk.second.updateAllocation();
		}
	}
}

/*void VulkanChunker::buildChunk(Chunk& chunk) {
	// TODO - should pre-allocate buffers with size and pool them
	// TODO - should abstract this function
	
	// Generate vertices
	std::vector<BlockVertex> vertices;
	vertices.reserve(chunk.blocks.size() * 3);
	for (const auto& block : chunk.blocks) {
		Position blockPosition = block.position;
		BlockVertex v1 = {{blockPosition.x + block.realWidth, blockPosition.y + block.realHeight}, {0.0f, 0.0f, 1.0f}};
		BlockVertex v2 = {{blockPosition.x, blockPosition.y + block.realHeight}, {0.0f, 1.0f, 0.0f}};
		BlockVertex v3 = {{blockPosition.x, blockPosition.y}, {1.0f, 0.0f, 0.0f}};
		vertices.push_back(v1);
		vertices.push_back(v2);
		vertices.push_back(v3);
	}

	// upload vertices to buffer
	size_t vertexBufferSize = sizeof(BlockVertex) * vertices.size();
	chunk.buffer = createBuffer(vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO);
	vmaCopyMemoryToAllocation(Vulkan::getAllocator(), vertices.data(), chunk.buffer.allocation, 0, vertexBufferSize);
	chunk.numVertices = vertices.size();
	}*/

void VulkanChunker::updateCircuit(DifferenceSharedPtr diff) {
	// TODO - very temp (rebuild whole thing (defeats the purpose))
	setCircuit(circuit);
}

std::vector<std::shared_ptr<VulkanChunkAllocation>> VulkanChunker::getChunks(Position min, Position max) {
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
				if (allocation != nullptr) seen.push_back(allocation);
			}
		}
	}

	return seen;
}
