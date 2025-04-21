#include "gridRenderer.h"

#include "computerAPI/fileLoader.h"
#include "computerAPI/directoryManager.h"
#include "gpu/vulkanShader.h"

GridRenderer::GridRenderer(VkRenderPass& renderPass) {
	// create shaders
	VkShaderModule gridVertShader = createShaderModule(readFileAsBytes(DirectoryManager::getResourceDirectory() / "shaders/grid.vert.spv"));
	VkShaderModule gridFragShader = createShaderModule(readFileAsBytes(DirectoryManager::getResourceDirectory() / "shaders/grid.frag.spv"));

	// create pipeline
	PipelineInformation gridPipelineInfo{};
	gridPipelineInfo.vertShader = gridVertShader;
	gridPipelineInfo.fragShader = gridFragShader;
	gridPipelineInfo.renderPass = renderPass;
	gridPipelineInfo.pushConstants.push_back({gridFadeOffset, iMvpOffset, VK_SHADER_STAGE_VERTEX_BIT});
	gridPipelineInfo.pushConstants.push_back({sizeof(float), gridFadeOffset, VK_SHADER_STAGE_FRAGMENT_BIT});
	gridPipeline = std::make_unique<Pipeline>(gridPipelineInfo);

	destroyShaderModule(gridVertShader);
	destroyShaderModule(gridFragShader);
}

constexpr float gridFadeOutDistance = 160.0f;
constexpr float gridFadeOutWidth = 60.0f;

void GridRenderer::render(VulkanFrameData& frame, const glm::mat4& viewMatrix, float viewScale) {
	float gridFade = std::clamp(1.0f - ((viewScale - gridFadeOutDistance) * (1.0f / gridFadeOutWidth)), 0.0f, 1.0f);
	// invert the view matrix to get the right coordinates for the grid in the shader
	GridPushConstants pushConstants { glm::inverse(viewMatrix), gridFade};

	// bind pipeline
	vkCmdBindPipeline(frame.getMainCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, gridPipeline->getHandle());
		
	// bind push constants
	vkCmdPushConstants(frame.getMainCommandBuffer(), gridPipeline->getLayout(), VK_SHADER_STAGE_VERTEX_BIT, iMvpOffset, gridFadeOffset, &pushConstants.iMvp);
	vkCmdPushConstants(frame.getMainCommandBuffer(), gridPipeline->getLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, gridFadeOffset, sizeof(float), &pushConstants.gridFade);

	vkCmdDraw(frame.getMainCommandBuffer(), 6, 1, 0, 0);
}
