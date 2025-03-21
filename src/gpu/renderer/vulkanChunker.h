#ifndef vulkanChunker_h
#define vulkanChunker_h

#include "backend/circuit/circuit.h"
#include "gpu/vulkanBuffer.h"

#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>

// TODO - not sure if this should be stored, maybe it would be faster to just
// query from blockContainer sometimes. We also don't have to store the width
// and height
struct RenderedBlock {
	BlockType blockType;
	Rotation rotation;
	int realWidth;
	int realHeight;
};

typedef std::pair<Position, Position> RenderedWire;

class VulkanChunkAllocation {
public:
	VulkanChunkAllocation(const std::unordered_map<Position, RenderedBlock>& blocks, std::set<RenderedWire>& wires);
	~VulkanChunkAllocation();

	inline const std::optional<AllocatedBuffer>& getBlockBuffer() const { return blockBuffer; }
	inline uint32_t getNumBlockVertices() const { return numBlockVertices; }

	inline const std::optional<AllocatedBuffer>& getWireBuffer() const { return wireBuffer; }
	inline uint32_t getNumWireVertices() const { return numWireVertices; }

	inline bool isAllocationComplete() const { return true; }
	
private:
	std::optional<AllocatedBuffer> blockBuffer;
	uint32_t numBlockVertices;

	std::optional<AllocatedBuffer> wireBuffer;
	uint32_t numWireVertices;
};

class ChunkChain {
public:
	inline std::unordered_map<Position, RenderedBlock>& getBlocksForUpdating() { allocationDirty = true; return blocks; }
	void updateAllocation();
	
	std::optional<std::shared_ptr<VulkanChunkAllocation>> getAllocation();
	
private:
	void annihilateOrphanGBs();
	
private:
	std::unordered_map<Position, RenderedBlock> blocks;
	std::set<RenderedWire> wires;
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
