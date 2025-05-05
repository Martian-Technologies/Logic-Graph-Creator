#ifndef vulkanChunker_h
#define vulkanChunker_h

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "backend/address.h"
#include "backend/circuit/circuit.h"
#include "gpu/abstractions/vulkanBuffer.h"
#include "gpu/abstractions/vulkanDescriptor.h"

// ====================================================================================================================

struct BlockVertex {
	glm::vec2 pos;
	glm::vec2 tex;

	inline static std::vector<VkVertexInputBindingDescription> getBindingDescriptions() {
		std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
		bindingDescriptions[0].binding = 0;
		bindingDescriptions[0].stride = sizeof(BlockVertex);
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescriptions;
    }

	inline static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(BlockVertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(BlockVertex, tex);

		return attributeDescriptions;
	}
};

struct WireVertex {
	glm::vec2 pos;
	uint32_t stateIndex;

	inline static std::vector<VkVertexInputBindingDescription> getBindingDescriptions() {
		std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
		bindingDescriptions[0].binding = 0;
		bindingDescriptions[0].stride = sizeof(WireVertex);
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescriptions;
    }

	inline static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(WireVertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32_UINT;
		attributeDescriptions[1].offset = offsetof(WireVertex, stateIndex);

		return attributeDescriptions;
	}
};

// ====================================================================================================================
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
	Address relativeStateAddress;
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

	inline const std::optional<AllocatedBuffer>& getStateBuffer() const { return stateBuffer; }
	inline VkDescriptorBufferInfo& getStateDescriptorBufferInfo() { return stateDescriptorBufferInfo; }

	inline bool isAllocationComplete() const { return true; }
	
private:
	std::optional<AllocatedBuffer> blockBuffer;
	uint32_t numBlockVertices;

	std::optional<AllocatedBuffer> wireBuffer;
	uint32_t numWireVertices;

	std::optional<AllocatedBuffer> stateBuffer;
	VkDescriptorBufferInfo stateDescriptorBufferInfo;
	
	std::vector<Address> relativeAdresses;
};

// ====================================================================================================================

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

// ====================================================================================================================

class VulkanChunker {
public:
	void setCircuit(Circuit* circuit);
	void updateCircuit(DifferenceSharedPtr diff);
	
	std::vector<std::shared_ptr<VulkanChunkAllocation>> getAllocations(Position min, Position max);

private:
	void updateCircuit(Difference* diff);
	void updateChunksOverConnection(Position start, Rotation startRotation, Position end, Rotation endRotation, bool add, std::unordered_set<Position>& chunksToUpdate);
	FVector getOutputOffset(Rotation rotation);
	FVector getInputOffset(Rotation rotation);
	
private:
	Circuit* circuit = nullptr;

	std::unordered_map<Position, ChunkChain> chunks;
	std::mutex mux; // sync can be relaxed in the future
};

#endif
