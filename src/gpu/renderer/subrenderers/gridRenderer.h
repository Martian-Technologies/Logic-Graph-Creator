#ifndef gridRenderer_h
#define gridRenderer_h

#include "gpu/renderer/vulkanFrame.h"
#include "gpu/renderer/vulkanPipeline.h"

struct GridPushConstants {
	glm::mat4 iMvp;
	float gridFade;
};

class GridRenderer {
public:
	GridRenderer(VkRenderPass& renderPass);

	void render(VulkanFrameData& frame, const glm::mat4& viewMatrix, float viewScale);

private:
	std::unique_ptr<Pipeline> gridPipeline = nullptr;
};

#endif
