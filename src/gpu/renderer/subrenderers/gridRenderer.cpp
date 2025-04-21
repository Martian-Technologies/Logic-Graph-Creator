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
	gridPipelineInfo.pushConstantSize = sizeof(GridPushConstants);
	gridPipeline = std::make_unique<Pipeline>(gridPipelineInfo);

	destroyShaderModule(gridVertShader);
	destroyShaderModule(gridFragShader);
}

void GridRenderer::render(VulkanFrameData& frame, const glm::mat4& viewMatrix) {
	// invert the view matrix to get the right coordinates for the grid in the shader
	GridPushConstants pushConstants { glm::inverse(viewMatrix) };

	// bind pipeline
	vkCmdBindPipeline(frame.getMainCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, gridPipeline->getHandle());
		
	// bind push constants
	vkCmdPushConstants(frame.getMainCommandBuffer(), gridPipeline->getLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GridPushConstants), &pushConstants);

	vkCmdDraw(frame.getMainCommandBuffer(), 6, 1, 0, 0);
}
