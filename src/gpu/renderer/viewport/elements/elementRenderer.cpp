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
	blockPreviewPipelineInfo.pushConstants.push_back({VK_SHADER_STAGE_VERTEX_BIT, sizeof(BlockPreviewPushConstant)});
	blockPreviewPipelineInfo.descriptorSets.push_back(device->getBlockTextureManager()->getDescriptorLayout());
	blockPreviewPipeline.init(device, blockPreviewPipelineInfo);
	
	destroyShaderModule(device->getDevice(), blockPreviewVertShader);
	destroyShaderModule(device->getDevice(), blockPreviewFragShader);

	// box selection
	VkShaderModule boxSelectionVertShader = createShaderModule(device->getDevice(), readFileAsBytes(DirectoryManager::getResourceDirectory() / "shaders/boxSelection.vert.spv"));
	VkShaderModule boxSelectionFragShader = createShaderModule(device->getDevice(), readFileAsBytes(DirectoryManager::getResourceDirectory() / "shaders/boxSelection.frag.spv"));

	PipelineInformation boxSelectionPipelineInfo{};
	boxSelectionPipelineInfo.vertShader = boxSelectionVertShader;
	boxSelectionPipelineInfo.fragShader = boxSelectionFragShader;
	boxSelectionPipelineInfo.renderPass = renderPass;
	boxSelectionPipelineInfo.pushConstants.push_back({VK_SHADER_STAGE_VERTEX_BIT, offsetof(BoxSelectionPushConstant, inverted)});
	boxSelectionPipelineInfo.pushConstants.push_back({VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(uint32_t)});
	boxSelectionPipeline.init(device, boxSelectionPipelineInfo);
	
	destroyShaderModule(device->getDevice(), boxSelectionVertShader);
	destroyShaderModule(device->getDevice(), boxSelectionFragShader);

	// connection preview
	VkShaderModule connectionPreviewVertShader = createShaderModule(device->getDevice(), readFileAsBytes(DirectoryManager::getResourceDirectory() / "shaders/connectionPreview.vert.spv"));
	VkShaderModule connectionPreviewFragShader = createShaderModule(device->getDevice(), readFileAsBytes(DirectoryManager::getResourceDirectory() / "shaders/connectionPreview.frag.spv"));

	PipelineInformation connectionPreviewPipelineInfo{};
	connectionPreviewPipelineInfo.vertShader = connectionPreviewVertShader;
	connectionPreviewPipelineInfo.fragShader = connectionPreviewFragShader;
	connectionPreviewPipelineInfo.renderPass = renderPass;
	connectionPreviewPipelineInfo.pushConstants.push_back({VK_SHADER_STAGE_VERTEX_BIT, sizeof(ConnectionPreviewPushConstant)});
	connectionPreviewPipeline.init(device, connectionPreviewPipelineInfo);
	
	destroyShaderModule(device->getDevice(), connectionPreviewVertShader);
	destroyShaderModule(device->getDevice(), connectionPreviewFragShader);
}

void ElementRenderer::cleanup() {
	connectionPreviewPipeline.cleanup();
	boxSelectionPipeline.cleanup();
	blockPreviewPipeline.cleanup();
}

void ElementRenderer::render(Frame& frame, const glm::mat4& viewMatrix,
							 const std::vector<BlockPreviewRenderData>& blockPreviews,
							 const std::vector<BoxSelectionRenderData>& boxSelections,
							 const std::vector<ConnectionPreviewRenderData>& connectionPreviews) {
	
	// Block previews
	if (!blockPreviews.empty()) {
		vkCmdBindPipeline(frame.mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, blockPreviewPipeline.getHandle());
		vkCmdBindDescriptorSets(frame.mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, blockPreviewPipeline.getLayout(), 0, 1, &device->getBlockTextureManager()->getTexture().descriptor, 0, nullptr);
	
		BlockPreviewPushConstant blockPreviewConstant;
		blockPreviewConstant.mvp = viewMatrix;
		Vec2 uvCellSize = device->getBlockTextureManager()->getTileset().getCellUVSize();
		blockPreviewConstant.uvCellSizeX = uvCellSize.x;
		blockPreviewConstant.uvCellSizeY = uvCellSize.y;
		for (const BlockPreviewRenderData& preview : blockPreviews){
			blockPreviewConstant.position = preview.position;
			blockPreviewConstant.size = preview.size;
			blockPreviewConstant.rotation = preview.rotation;
			blockPreviewConstant.uvOffsetX = device->getBlockTextureManager()->getTileset().getTopLeftUV(preview.type + 1, 0).x;

			blockPreviewPipeline.cmdPushConstants(frame.mainCommandBuffer, &blockPreviewConstant);
			vkCmdDraw(frame.mainCommandBuffer, 6, 1, 0, 0);
		}
	}

	// Box selections
	if (!boxSelections.empty()) {
		vkCmdBindPipeline(frame.mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, boxSelectionPipeline.getHandle());
	
		BoxSelectionPushConstant boxSelectionConstant;
		boxSelectionConstant.mvp = viewMatrix;
		
		for (const BoxSelectionRenderData& boxSelection : boxSelections){
			boxSelectionConstant.position = boxSelection.topLeft;
			boxSelectionConstant.size = boxSelection.size;
			boxSelectionConstant.inverted = boxSelection.inverted;

			boxSelectionPipeline.cmdPushConstants(frame.mainCommandBuffer, &boxSelectionConstant);
			vkCmdDraw(frame.mainCommandBuffer, 6, 1, 0, 0);
		}
	}

	// Connection previews
	if (!connectionPreviews.empty()) {
		vkCmdBindPipeline(frame.mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, connectionPreviewPipeline.getHandle());
	
		ConnectionPreviewPushConstant connectionPreviewConstant;
		connectionPreviewConstant.mvp = viewMatrix;
		
		for (const ConnectionPreviewRenderData& connectionPreview : connectionPreviews){
			connectionPreviewConstant.pointA = connectionPreview.pointA;
			connectionPreviewConstant.pointB = connectionPreview.pointB;

			connectionPreviewPipeline.cmdPushConstants(frame.mainCommandBuffer, &connectionPreviewConstant);
			vkCmdDraw(frame.mainCommandBuffer, 6, 1, 0, 0);
		}
	}
}
