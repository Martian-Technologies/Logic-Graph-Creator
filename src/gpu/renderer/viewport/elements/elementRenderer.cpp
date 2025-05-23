#include "elementRenderer.h"

#include "computerAPI/directoryManager.h"
#include "computerAPI/fileLoader.h"
#include "gpu/abstractions/vulkanShader.h"

void ElementRenderer::init(VulkanDevice* device, VkRenderPass& renderPass) {
	this->device = device;
	
	// block preview
	VkShaderModule blockPreviewVertShader = createShaderModule(device->getDevice(), readFileAsBytes(DirectoryManager::getResourceDirectory() / "shaders/blockPreview.vert.spv"));
	VkShaderModule blockPreviewFragShader = createShaderModule(device->getDevice(), readFileAsBytes(DirectoryManager::getResourceDirectory() / "shaders/blockPreview.frag.spv"));

	PipelineInformation blockPreviewPipelineInfo{};
	blockPreviewPipelineInfo.vertShader = blockPreviewVertShader;
	blockPreviewPipelineInfo.fragShader = blockPreviewFragShader;
	blockPreviewPipelineInfo.renderPass = renderPass;
	blockPreviewPipelineInfo.pushConstants.push_back({VK_SHADER_STAGE_VERTEX_BIT, offsetof(BlockPreviewPushConstant, alpha)});
	blockPreviewPipelineInfo.pushConstants.push_back({VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(float)});
	blockPreviewPipelineInfo.descriptorSets.push_back(device->getBlockTextureManager()->getDescriptorLayout());
	blockPreviewPipeline.init(device, blockPreviewPipelineInfo);
	
	destroyShaderModule(device->getDevice(), blockPreviewVertShader);
	destroyShaderModule(device->getDevice(), blockPreviewFragShader);
}

void ElementRenderer::cleanup() {
	blockPreviewPipeline.cleanup();
}


const float PREVIEW_OPACITY = 0.5f;

void ElementRenderer::render(Frame& frame, const glm::mat4& viewMatrix, std::vector<BlockPreviewRenderData> blockPreviews) {
	if (!blockPreviews.empty()) {
		vkCmdBindPipeline(frame.mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, blockPreviewPipeline.getHandle());
		vkCmdBindDescriptorSets(frame.mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, blockPreviewPipeline.getLayout(), 0, 1, &device->getBlockTextureManager()->getTexture().descriptor, 0, nullptr);
	
		BlockPreviewPushConstant blockPreviewConstant;
		blockPreviewConstant.mvp = viewMatrix;
		Vec2 uvCellSize = device->getBlockTextureManager()->getTileset().getCellUVSize();
		blockPreviewConstant.uvCellSizeX = uvCellSize.x;
		blockPreviewConstant.uvCellSizeY = uvCellSize.y;
		for (const BlockPreviewRenderData& preview : blockPreviews){
			blockPreviewConstant.position = glm::vec2(preview.position.x, preview.position.y);
			blockPreviewConstant.size = glm::vec2(preview.size.dx, preview.size.dy);
			blockPreviewConstant.rotation = preview.rotation;
			blockPreviewConstant.uvOffsetX = device->getBlockTextureManager()->getTileset().getTopLeftUV(preview.type + 1, 0).x;
			blockPreviewConstant.alpha = PREVIEW_OPACITY;

			blockPreviewPipeline.cmdPushConstants(frame.mainCommandBuffer, &blockPreviewConstant);
			vkCmdDraw(frame.mainCommandBuffer, 6, 1, 0, 0);
		}
	}
}
