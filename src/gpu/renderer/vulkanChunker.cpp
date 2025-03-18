#include "vulkanChunker.h"

#include "gpu/vulkanManager.h"

const unsigned int CHUNK_SIZE = 25;

Position getChunk(Position in) {
	in.x = std::floor(in.x / (float)CHUNK_SIZE) * (int)CHUNK_SIZE;
	in.y = std::floor(in.y / (float)CHUNK_SIZE) * (int)CHUNK_SIZE;
	return in;
}

void VulkanChunker::setCircuit(Circuit* circuit) {
	this->circuit = circuit;
	
	// clear existing chunk buffers
	for (auto& chunk : chunks) {
		destroyBuffer(chunk.second.buffer);
	}
	chunks.clear();

	if (circuit) {
		// partition blocks into chunks
		// TODO - this should be improved to happen deferred
		for (const auto& block : *(circuit->getBlockContainer())) {
			chunks[getChunk(block.second.getPosition())].blocks.push_back({block.second.type(), block.second.getPosition(), block.second.getRotation(), block.second.width(), block.second.height()});
		}

		for (auto& chunk : chunks) {
			buildChunk(chunk.second);
		}
	}
}

void VulkanChunker::buildChunk(Chunk& chunk) {
	// TODO - should pre-allocate buffers with size and pool them
	
	// Generate vertices
	std::vector<Vertex> vertices;
	vertices.reserve(chunk.blocks.size() * 3);
	for (const auto& block : chunk.blocks) {
		Position blockPosition = block.position;
		Vertex v1 = {{blockPosition.x + block.realWidth, blockPosition.y + block.realHeight}, {0.0f, 0.0f, 1.0f}};
		Vertex v2 = {{blockPosition.x, blockPosition.y + block.realHeight}, {0.0f, 1.0f, 0.0f}};
		Vertex v3 = {{blockPosition.x, blockPosition.y}, {1.0f, 0.0f, 0.0f}};
		vertices.push_back(v1);
		vertices.push_back(v2);
		vertices.push_back(v3);
	}

	// upload vertices to buffer
	size_t vertexBufferSize = sizeof(Vertex) * vertices.size();
	chunk.buffer = createBuffer(vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO);
	vmaCopyMemoryToAllocation(Vulkan::getAllocator(), vertices.data(), chunk.buffer.allocation, 0, vertexBufferSize);
	chunk.numVertices = vertices.size();
}

void VulkanChunker::updateCircuit(DifferenceSharedPtr diff) {
	// TODO - very temp (rebuild whole thing (defeats the purpose))
	setCircuit(circuit);
}

std::vector<Chunk> VulkanChunker::getChunks(Position min, Position max) {
	// get chunk bounds with padding for large blocks (this will technically goof if there are blocks larger than chunk size)
	min = getChunk(min) - Vector(CHUNK_SIZE, CHUNK_SIZE);
	max = getChunk(max) + Vector(CHUNK_SIZE, CHUNK_SIZE);

	std::vector<Chunk> seen;
	for (cord_t chunkX = min.x; chunkX <= max.x; chunkX += CHUNK_SIZE) {
		for (cord_t chunkY = min.y; chunkY <= max.y; chunkY += CHUNK_SIZE) {
			auto chunk = chunks.find({chunkX, chunkY});
			if (chunk != chunks.end()) seen.push_back(chunk->second);
		}
	}

	return seen;
}

void VulkanChunker::destroy() {
	for (auto& chunk : chunks) {
		destroyBuffer(chunk.second.buffer);
	}
	chunks.clear();
}
