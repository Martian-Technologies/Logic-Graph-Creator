#ifndef vulkanChunker_h
#define vulkanChunker_h

#include "backend/circuit/circuit.h"
#include "gpu/vulkanBuffer.h"

#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>

// TODO -
// Chunkchain should work (v^)
// Thread safety for chunker (v^)
// Chunk should handle differences elegantly
// Chunk objects actually get added to frame data somehow
// Chunk system should be abstracted
// Ease up on sync

// TODO - not sure if this should be stored, maybe it would be faster to just
// query from blockContainer sometimes. We also don't have to store the width
// and height

struct RenderedBlock {
	BlockType blockType;
	Rotation rotation;
	int realWidth;
	int realHeight;
};

class VulkanChunkAllocation {
public:
	VulkanChunkAllocation(const std::unordered_map<Position, RenderedBlock>& blocks);
	~VulkanChunkAllocation();

	inline const AllocatedBuffer& getBuffer() const { return buffer; }
	inline uint32_t getNumVertices() const { return numVertices; }

	inline bool isAllocationComplete() const { return true; }
	
private:
	AllocatedBuffer buffer;
	uint32_t numVertices;
};

class ChunkChain {
public:
	inline std::unordered_map<Position, RenderedBlock>& getBlocksForUpdating() { allocationDirty = true; return upToData; }
	void updateAllocation();
	
	std::optional<std::shared_ptr<VulkanChunkAllocation>> getAllocation();
	
private:
	void annihilateOrphanGBs();
	
private:
	std::unordered_map<Position, RenderedBlock> upToData; // up to date (block) data
	bool allocationDirty = false;

	std::optional<std::shared_ptr<VulkanChunkAllocation>> newestAllocation;
	std::optional<std::shared_ptr<VulkanChunkAllocation>> currentlyAllocating;
	std::vector<std::shared_ptr<VulkanChunkAllocation>> gbJail; // gay baby jail
};

class VulkanChunker {
public:
	void setCircuit(Circuit* circuit);
	void updateCircuit(DifferenceSharedPtr diff);
	
	std::vector<std::shared_ptr<VulkanChunkAllocation>> getAllocations(Position min, Position max);
	
private:
	Circuit* circuit = nullptr;

	std::unordered_map<Position, ChunkChain> chunks;
	std::mutex mux; // sync can be relaxed in the future
};

#endif
