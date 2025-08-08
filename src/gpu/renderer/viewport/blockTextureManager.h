#ifndef blockTextureManager_h
#define blockTextureManager_h

#include "backend/circuitView/renderer/tileSet.h"
#include "gpu/abstractions/vulkanDescriptor.h"
#include "gpu/abstractions/vulkanImage.h"
#include "gpu/vulkanDevice.h"

struct BlockTexture {
	VkDescriptorSet descriptor;
	VkSampler sampler;
	AllocatedImage image;
};

class BlockTextureManager {
public:
	void init(VulkanDevice* device);
	void cleanup();

	inline VkDescriptorSetLayout getDescriptorLayout() { return descriptorLayout; }
	inline BlockTexture& getTexture() { return mainTexture; }
	inline TileSetInfo& getTileset() { return mainTileSet; }

private:
	VulkanDevice* device;

	DescriptorAllocator descriptorAllocator;
	VkDescriptorSetLayout descriptorLayout;

	BlockTexture mainTexture;
	TileSetInfo mainTileSet = TileSetInfo(256, 15, 4);
};

#endif
