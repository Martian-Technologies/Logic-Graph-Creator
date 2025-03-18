#ifndef vulkanChunker_h
#define vulkanChunker_h

#include "backend/circuit/circuit.h"
#include "gpu/vulkanBuffer.h"

#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>

// TODO -
// Chunkchain should work
// Thread safety for chunker
// Chunk should handle differences elegantly
// Chunk objects actually get added to frame data somehow
// Chunk system should be abstracted

// TODO - not sure if this should be stored, maybe it would be faster to just query from blockContainer
struct RenderedBlock {
	BlockType blockType;
	Position position;
	Rotation rotation;
	int realWidth;
	int realHeight;
};

class VulkanChunkAllocation {
public:
	VulkanChunkAllocation(const std::vector<RenderedBlock>& blocks, uint32_t numVertices);
	~VulkanChunkAllocation();

	inline const AllocatedBuffer& getBuffer() const { return buffer; }
	inline uint32_t getNumVertices() const { return numVertices; }
	
private:
	AllocatedBuffer buffer;
	uint32_t numVertices;
};

class ChunkChain {
public:
	std::shared_ptr<VulkanChunkAllocation> getAllocation();

	inline std::vector<RenderedBlock>& getBlocks() { return upToData; }
	void updateAllocation();
	
private:
	void clean();
	void checkForFinished();
	
private:
	std::vector<RenderedBlock> upToData; // funny joke?
	
	std::deque<std::shared_ptr<VulkanChunkAllocation>> allocationChain;
	std::vector<std::shared_ptr<VulkanChunkAllocation>> uploadingAllocations;
};

class VulkanChunker {
public:
	void setCircuit(Circuit* circuit);
	void updateCircuit(DifferenceSharedPtr diff);
	
	std::vector<std::shared_ptr<VulkanChunkAllocation>> getChunks(Position min, Position max);
	
private:
	Circuit* circuit = nullptr;

	std::unordered_map<Position, ChunkChain> chunks;
};

#endif
