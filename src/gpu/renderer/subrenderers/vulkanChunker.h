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

struct RenderedWire {
	FPosition start;
	FPosition end;
};

struct WireHash {
    std::size_t operator () (const std::pair<Position,Position> &p) const {
        auto h1 = std::hash<Position>{}(p.first);
        auto h2 = std::hash<Position>{}(p.second);

		// temp hash
        return h1 ^ h2;
    }
};

typedef std::unordered_map<Position, RenderedBlock> RenderedBlocks;
typedef std::unordered_map<std::pair<Position, Position>, RenderedWire, WireHash> RenderedWires;

// TODO - maybe these should just be split into two different types
class VulkanChunkAllocation {
public:
	VulkanChunkAllocation(RenderedBlocks& blocks, RenderedWires& wires);
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
	inline RenderedBlocks& getBlocksForUpdating() { allocationDirty = true; return blocks; }
	inline RenderedWires& getWiresForUpdating() { allocationDirty = true; return wires; }
	void updateAllocation();
	
	std::optional<std::shared_ptr<VulkanChunkAllocation>> getAllocation();
	
private:
	void annihilateOrphanGBs();
	
private:
	RenderedBlocks blocks;
	RenderedWires wires;
	bool allocationDirty = false;

	std::optional<std::shared_ptr<VulkanChunkAllocation>> newestAllocation;
	std::optional<std::shared_ptr<VulkanChunkAllocation>> currentlyAllocating;
	std::vector<std::shared_ptr<VulkanChunkAllocation>> gbJail; // gay baby jail (deleted chunks mid allocation go here)
};

class VulkanChunker {
public:
	void setCircuit(Circuit* circuit);
	void updateCircuit(DifferenceSharedPtr diff);
	
	std::vector<std::shared_ptr<VulkanChunkAllocation>> getAllocations(Position min, Position max);

private:
	void getChunksOverConnection(Position p1, Position p2, std::vector<Position>& chunks);
	
private:
	Circuit* circuit = nullptr;

	std::unordered_map<Position, ChunkChain> chunks;
	std::mutex mux; // sync can be relaxed in the future
};

#endif
