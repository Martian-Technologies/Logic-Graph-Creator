#ifndef vulkanChunker_h
#define vulkanChunker_h

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "backend/address.h"
#include "backend/circuit/circuit.h"
#include "gpu/abstractions/vulkanBuffer.h"
#include "gpu/abstractions/vulkanDescriptor.h"
#include "gpu/helper/nBuffer.h"

// ====================================================================================================================

struct BlockInstance {
	glm::vec2 pos;
	uint32_t sizeX;
	uint32_t sizeY;
	float texX;
	uint32_t rotation;

	inline static std::vector<VkVertexInputBindingDescription> getBindingDescriptions() {
		std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
		bindingDescriptions[0].binding = 0;
		bindingDescriptions[0].stride = sizeof(BlockInstance);
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

        return bindingDescriptions;
    }

	inline static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(4);

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(BlockInstance, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32_UINT;
		attributeDescriptions[1].offset = offsetof(BlockInstance, sizeX);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(BlockInstance, texX);

		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32_UINT;
		attributeDescriptions[3].offset = offsetof(BlockInstance, rotation);

		return attributeDescriptions;
	}
};

struct WireInstance {
	glm::vec2 pointA;
	glm::vec2 pointB;
	uint32_t stateIndex;

	inline static std::vector<VkVertexInputBindingDescription> getBindingDescriptions() {
		std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
		bindingDescriptions[0].binding = 0;
		bindingDescriptions[0].stride = sizeof(WireInstance);
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

        return bindingDescriptions;
    }

	inline static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(3);

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(WireInstance, pointA);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(WireInstance, pointB);
		
		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32_UINT;
		attributeDescriptions[2].offset = offsetof(WireInstance, stateIndex);

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
	VulkanChunkAllocation(VulkanDevice* device, RenderedBlocks& blocks, RenderedWires& wires);
	~VulkanChunkAllocation();

	inline const std::optional<AllocatedBuffer>& getBlockBuffer() const { return blockBuffer; }
	inline uint32_t getNumBlockInstances() const { return numBlockInstances; }
	
	inline const std::optional<AllocatedBuffer>& getWireBuffer() const { return wireBuffer; }
	inline uint32_t getNumWireInstances() const { return numWireInstances; }

	inline std::optional<NBuffer>& getStateBuffer() { return stateBuffer; }

	inline std::vector<Address>& getRelativeAddresses() { return relativeAdresses; }

	inline bool isAllocationComplete() const { return true; }
	
private:
	std::optional<AllocatedBuffer> blockBuffer;
	uint32_t numBlockInstances;

	std::optional<AllocatedBuffer> wireBuffer;
	uint32_t numWireInstances;

	std::optional<NBuffer> stateBuffer;
	VkDescriptorBufferInfo stateDescriptorBufferInfo;
	
	std::vector<Address> relativeAdresses;
};

// ====================================================================================================================

class ChunkChain {
public:
	inline RenderedBlocks& getBlocksForUpdating() { allocationDirty = true; return blocks; }
	inline RenderedWires& getWiresForUpdating() { allocationDirty = true; return wires; }
	void updateAllocation(VulkanDevice* device);
	
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

struct ChunkerConnectionEnd {
	Position otherBlock;
	bool isInput;
};

class VulkanChunker {
public:
	VulkanChunker(VulkanDevice* device);
	
	void setCircuit(Circuit* circuit);
	void updateCircuit(DifferenceSharedPtr diff);
	
	std::vector<std::shared_ptr<VulkanChunkAllocation>> getAllocations(Position min, Position max);

private:
	void updateCircuit(Difference* diff);
	void updateChunksOverConnection(Position start, Rotation startRotation, Position end, Rotation endRotation, bool add, std::unordered_set<Position>& chunksToUpdate);
	FVector getOutputOffset(Rotation rotation);
	FVector getInputOffset(Rotation rotation);
	
private:
	std::unordered_map<Position, ChunkChain> chunks;
	std::unordered_map<Position, std::unordered_map<std::pair<Position, Position>, ChunkerConnectionEnd, WireHash>> blockToConnections;
	std::mutex mux; // sync can be relaxed in the future

	// refs
	Circuit* circuit = nullptr;
	VulkanDevice* device = nullptr;
};

#endif
