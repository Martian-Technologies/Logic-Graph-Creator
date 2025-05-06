#ifndef gridRenderer_h
#define gridRenderer_h

#include "gpu/renderer/frameManager.h"
#include "gpu/abstractions/vulkanPipeline.h"

struct GridPushConstants {
	glm::mat4 iMvp;
	float gridFade;
};

class GridRenderer {
public:
	void init(VulkanDevice* device, VkRenderPass& renderPass);
	void cleanup();

	void render(Frame& frame, const glm::mat4& viewMatrix, float viewScale);

private:
	Pipeline pipeline;

	// push constant data
	size_t iMvpOffset = 0;
	size_t gridFadeOffset = offsetof(GridPushConstants, gridFade);
};

#endif
