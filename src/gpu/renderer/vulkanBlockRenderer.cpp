#include "vulkanBlockRenderer.h"

#include "gpu/vulkanUtil.h"
#include "computerAPI/fileLoader.h"

void VulkanBlockRenderer::initialize(VkRenderPass& renderPass) {
	vertShader = createShaderModule(readFileAsBytes(":/shaders/shader.vert.spv"));
	fragShader = createShaderModule(readFileAsBytes(":/shaders/shader.frag.spv"));
	pipeline = createPipeline(vertShader, fragShader, BlockVertex::getBindingDescriptions(), BlockVertex::getAttributeDescriptions(), renderPass);
}

void VulkanBlockRenderer::setCircuit(Circuit* circuit) {
	vertexBufferRing.setCircuit(circuit);
}

void VulkanBlockRenderer::updateCircuit(DifferenceSharedPtr diff) {
	vertexBufferRing.updateCircuit(diff);
}

void VulkanBlockRenderer::render(VkCommandBuffer& commandBuffer, VkExtent2D& renderExtent, const glm::mat4& viewMatrix) {
	// only draw with pipeline if we have a circuit (vertex buffer)
	if (vertexBufferRing.hasCircuit()) {
		// bind render pipeline
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.handle);
		
		// bind push constants
		ViewPushConstants pushConstants{};
		pushConstants.mvp = viewMatrix;
		vkCmdPushConstants(commandBuffer, pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ViewPushConstants), &pushConstants);

		// set dynamic state
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(renderExtent.width);
		viewport.height = static_cast<float>(renderExtent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		VkRect2D scissor{};
		scissor.offset = {0, 0};
		scissor.extent = renderExtent;
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		// bind vertex buffers
		const CircuitBuffer& circuitBuffer = vertexBufferRing.getAvaiableBuffer();
		VkBuffer vertexBuffers[] = { circuitBuffer.blockBuffer.buffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

		// draw
		vkCmdDraw(commandBuffer, static_cast<uint32_t>(circuitBuffer.numBlockVertices), 1, 0, 0);
	}
}

void VulkanBlockRenderer::destroy() {
	vertexBufferRing.destroy();
	destroyShaderModule(vertShader);
	destroyShaderModule(fragShader);
	destroyPipeline(pipeline);
}
