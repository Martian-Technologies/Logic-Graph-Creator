#include "vulkanChunkRenderer.h"

#include "gpu/vulkanShader.h"
#include "computerAPI/fileLoader.h"
#include "computerAPI/directoryManager.h"

VulkanChunkRenderer::VulkanChunkRenderer(VkRenderPass& renderPass) {
	// load shaders
	VkShaderModule blockVertShader = createShaderModule(readFileAsBytes(DirectoryManager::getResourceDirectory() / "shaders/block.vert.spv"));
	VkShaderModule blockFragShader = createShaderModule(readFileAsBytes(DirectoryManager::getResourceDirectory() / "shaders/block.frag.spv"));
	VkShaderModule wireVertShader = createShaderModule(readFileAsBytes(DirectoryManager::getResourceDirectory() / "shaders/wire.vert.spv"));
	VkShaderModule wireFragShader = createShaderModule(readFileAsBytes(DirectoryManager::getResourceDirectory() / "shaders/wire.frag.spv"));

	// create graphic pipelines
	PipelineInformation blockPipelineInfo{};
	blockPipelineInfo.vertShader = blockVertShader;
	blockPipelineInfo.fragShader = blockFragShader;
	blockPipelineInfo.renderPass = renderPass;
	blockPipelineInfo.vertexBindingDescriptions = BlockVertex::getBindingDescriptions();
	blockPipelineInfo.vertexAttributeDescriptions = BlockVertex::getAttributeDescriptions();
	blockPipelineInfo.pushConstantSize = sizeof(ViewPushConstants);
	blockPipeline = std::make_unique<Pipeline>(blockPipelineInfo);
	
	PipelineInformation wirePipelineInfo{};
	wirePipelineInfo.vertShader = wireVertShader;
	wirePipelineInfo.fragShader = wireFragShader;
	wirePipelineInfo.renderPass = renderPass;
	wirePipelineInfo.vertexBindingDescriptions = WireVertex::getBindingDescriptions();
	wirePipelineInfo.vertexAttributeDescriptions = WireVertex::getAttributeDescriptions();
	wirePipelineInfo.pushConstantSize = sizeof(ViewPushConstants);
	wirePipeline = std::make_unique<Pipeline>(wirePipelineInfo);

	// destroy shader modules since we won't be recreating pipelines
	destroyShaderModule(blockVertShader);
	destroyShaderModule(blockFragShader);
	destroyShaderModule(wireVertShader);
	destroyShaderModule(wireFragShader);
}

VulkanChunkRenderer::~VulkanChunkRenderer() {
	// temp way to delete all the buffers
	// chunker.setCircuit(nullptr);
}


void VulkanChunkRenderer::setCircuit(Circuit* circuit) {
	chunker.setCircuit(circuit);
}

void VulkanChunkRenderer::updateCircuit(DifferenceSharedPtr diff) {
	chunker.updateCircuit(diff);
}

void VulkanChunkRenderer::render(VulkanFrameData& frame, VkViewport& viewport, const glm::mat4& viewMatrix, const std::pair<FPosition, FPosition>& viewBounds) {
	std::vector<std::shared_ptr<VulkanChunkAllocation>> chunks = chunker.getAllocations(viewBounds.first.snap(), viewBounds.second.snap());

	// save chunk data to frame
	frame.getChunkAllocations().insert(frame.getChunkAllocations().begin(), chunks.begin(), chunks.end());

	// shared push constants
	ViewPushConstants pushConstants{};
	pushConstants.mvp = viewMatrix;

	// shared dynamic state
	VkRect2D scissor{};
	scissor.offset = {static_cast<int32_t>(viewport.x), static_cast<int32_t>(viewport.y)};
	scissor.extent = {static_cast<uint32_t>(viewport.width), static_cast<uint32_t>(viewport.height)};
	vkCmdSetScissor(frame.getMainCommandBuffer(), 0, 1, &scissor);
	vkCmdSetViewport(frame.getMainCommandBuffer(), 0, 1, &viewport);
	
	// block drawing pass
	{
		// bind render pipeline
		vkCmdBindPipeline(frame.getMainCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, blockPipeline->getHandle());
		
		// bind push constants
		vkCmdPushConstants(frame.getMainCommandBuffer(), blockPipeline->getLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ViewPushConstants), &pushConstants);

		for (std::shared_ptr<VulkanChunkAllocation> chunk : chunks) {
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

	// wire drawing pass
	{
		// bind render pipeline
		vkCmdBindPipeline(frame.getMainCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, wirePipeline->getHandle());
		
		// bind push constants
		vkCmdPushConstants(frame.getMainCommandBuffer(), wirePipeline->getLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ViewPushConstants), &pushConstants);

		for (std::shared_ptr<VulkanChunkAllocation> chunk : chunks) {
			if (chunk->getWireBuffer().has_value()) {
				
				// bind vertex buffers
				VkBuffer vertexBuffers[] = { chunk->getWireBuffer()->buffer };
				VkDeviceSize offsets[] = { 0 };
				vkCmdBindVertexBuffers(frame.getMainCommandBuffer(), 0, 1, vertexBuffers, offsets);

				// draw
				vkCmdDraw(frame.getMainCommandBuffer(), static_cast<uint32_t>(chunk->getNumWireVertices()), 1, 0, 0);
			}
		}
	}
}
