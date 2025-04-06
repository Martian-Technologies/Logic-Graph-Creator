#ifndef subrenderer_h
#define subrenderer_h

#include <vulkan/vulkan.h>
#include <glm/ext/matrix_float4x4.hpp>

struct VulkanFrameData;

struct SubrendererInfo {
	VulkanFrameData& frame;
	glm::mat4 pixelViewMat;
	std::pair<uint32_t, uint32_t> windowSize;
	VkExtent2D windowExtent;
};

#endif
