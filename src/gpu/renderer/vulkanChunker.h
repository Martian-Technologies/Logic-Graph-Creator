#ifndef vulkanChunker_h
#define vulkanChunker_h

#include "backend/circuit/circuit.h"
#include "gpu/vulkanBuffer.h"

#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>

// TODO - not sure if this should be stored
struct RenderedBlock {
	BlockType blockType;
	Position position;
	Rotation rotation;
	int realWidth;
	int realHeight;
};

struct Chunk {
	AllocatedBuffer buffer;
	uint32_t numVertices;

	std::vector<RenderedBlock> blocks;
};

class VulkanChunker {
public:
	void setCircuit(Circuit* circuit);
	void updateCircuit(DifferenceSharedPtr diff);
	void destroy();

	// TODO - this should not copy chunks
	std::vector<Chunk> getChunks(Position min, Position max);

private:
	void buildChunk(Chunk& chunk);
	
private:
	Circuit* circuit = nullptr;

	// TODO - maybe chunks should be a pointer
	std::unordered_map<Position, Chunk> chunks;
};

#endif
