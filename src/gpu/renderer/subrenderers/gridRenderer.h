#ifndef gridRenderer_h
#define gridRenderer_h

#include "gpu/renderer/frameManager.h"
#include "gpu/renderer/vulkanPipeline.h"

struct GridPushConstants {
	glm::mat4 iMvp;
	float gridFade;
};

class GridRenderer {
public:
	GridRenderer(VkRenderPass& renderPass);

	void render(Frame& frame, const glm::mat4& viewMatrix, float viewScale);

private:
	std::unique_ptr<Pipeline> gridPipeline = nullptr;

	// push constant data
	size_t iMvpOffset = 0;
	size_t gridFadeOffset = offsetof(GridPushConstants, gridFade);
};

#endif
