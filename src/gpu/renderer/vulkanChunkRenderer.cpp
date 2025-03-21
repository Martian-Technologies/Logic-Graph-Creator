#include "vulkanChunkRenderer.h"

#include "gpu/vulkanUtil.h"
#include "computerAPI/fileLoader.h"
#include "computerAPI/directoryManager.h"

void VulkanChunkRenderer::initialize(VkRenderPass& renderPass) {
	// load shaders
	blockVertShader = createShaderModule(readFileAsBytes(DirectoryManager::getResourceDirectory() / "shaders/block.vert.spv"));
	blockFragShader = createShaderModule(readFileAsBytes(DirectoryManager::getResourceDirectory() / "shaders/block.frag.spv"));

	// create graphic pipeline
	blockPipeline = createPipeline(blockVertShader, blockFragShader, BlockVertex::getBindingDescriptions(), BlockVertex::getAttributeDescriptions(), renderPass);
}

void VulkanChunkRenderer::destroy() {
	// temp way to delete all the buffers
	chunker.setCircuit(nullptr);
	
	destroyShaderModule(blockVertShader);
	destroyShaderModule(blockFragShader);
	destroyPipeline(blockPipeline);
}


void VulkanChunkRenderer::setCircuit(Circuit* circuit) {
	chunker.setCircuit(circuit);
}

void VulkanChunkRenderer::updateCircuit(DifferenceSharedPtr diff) {
	chunker.updateCircuit(diff);
}

void VulkanChunkRenderer::render(VulkanFrameData& frame, VkExtent2D& renderExtent, const glm::mat4& viewMatrix, const std::pair<FPosition, FPosition>& viewBounds) {
	// bind render pipeline
	vkCmdBindPipeline(frame.getMainCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, blockPipeline.handle);
		
	// bind push constants
	ViewPushConstants pushConstants{};
	pushConstants.mvp = viewMatrix;
	vkCmdPushConstants(frame.getMainCommandBuffer(), blockPipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ViewPushConstants), &pushConstants);

	// set dynamic state
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(renderExtent.width);
	viewport.height = static_cast<float>(renderExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(frame.getMainCommandBuffer(), 0, 1, &viewport);
	VkRect2D scissor{};
	scissor.offset = {0, 0};
	scissor.extent = renderExtent;
	vkCmdSetScissor(frame.getMainCommandBuffer(), 0, 1, &scissor);

	for (std::shared_ptr<VulkanChunkAllocation> chunk : chunker.getAllocations(viewBounds.first.snap(), viewBounds.second.snap())) {
		// save chunk data to frame
		frame.getChunkAllocations().push_back(chunk);

		if (chunk->getBlockBuffer().has_value()) {
			// bind vertex buffers
			VkBuffer vertexBuffers[] = { chunk->getBlockBuffer()->buffer };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(frame.getMainCommandBuffer(), 0, 1, vertexBuffers, offsets);

			// draw
			vkCmdDraw(frame.getMainCommandBuffer(), static_cast<uint32_t>(chunk->getNumBlockVertices()), 1, 0, 0);
		}
	}
}
