#ifndef gridRenderer_h
#define gridRenderer_h

#include "gpu/renderer/vulkanFrame.h"

class GridRenderer {
public:
	GridRenderer(VkRenderPass& renderPass, VkDescriptorSetLayout viewLayout);
	~GridRenderer();

	void render(VulkanFrameData& frame, VkExtent2D windowExtent, VkDescriptorSet viewDataSet);
};

#endif
