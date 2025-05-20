#include "gridRenderer.h"

#include "computerAPI/fileLoader.h"
#include "computerAPI/directoryManager.h"
#include "gpu/abstractions/vulkanShader.h"

void GridRenderer::init(VulkanDevice* device, VkRenderPass& renderPass) {
	// create shaders
	VkShaderModule gridVertShader = createShaderModule(device->getDevice(), readFileAsBytes(DirectoryManager::getResourceDirectory() / "shaders/grid.vert.spv"));
	VkShaderModule gridFragShader = createShaderModule(device->getDevice(), readFileAsBytes(DirectoryManager::getResourceDirectory() / "shaders/grid.frag.spv"));

	// create pipeline
	PipelineInformation gridPipelineInfo{};
	gridPipelineInfo.vertShader = gridVertShader;
	gridPipelineInfo.fragShader = gridFragShader;
	gridPipelineInfo.renderPass = renderPass;
	gridPipelineInfo.pushConstants.push_back({VK_SHADER_STAGE_VERTEX_BIT, gridFadeOffset});
	gridPipelineInfo.pushConstants.push_back({VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(GridPushConstants) - gridFadeOffset});
	pipeline.init(device, gridPipelineInfo);

	destroyShaderModule(device->getDevice(), gridVertShader);
	destroyShaderModule(device->getDevice(), gridFragShader);
}

void GridRenderer::cleanup() {
	pipeline.cleanup();
}

constexpr float gridFadeOutDistance = 160.0f;
constexpr float gridFadeOutWidth = 60.0f;

void GridRenderer::render(Frame& frame, const glm::mat4& viewMatrix, float viewScale, bool hasCircuit) {
	// calculate grid fade num
	float gridFade = std::clamp(1.0f - ((viewScale - gridFadeOutDistance) * (1.0f / gridFadeOutWidth)), 0.0f, 1.0f);
	// invert the view matrix to get the right coordinates for the grid in the shader
	GridPushConstants pushConstants { glm::inverse(viewMatrix), gridFade, hasCircuit ? 1.0f : 0.0f };

	// bind pipeline
	vkCmdBindPipeline(frame.mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.getHandle());
		
	// bind push constants
	pipeline.cmdPushConstants(frame.mainCommandBuffer, &pushConstants);

	vkCmdDraw(frame.mainCommandBuffer, 6, 1, 0, 0);
}
