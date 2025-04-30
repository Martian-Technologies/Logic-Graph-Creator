#include "vulkanChunkRenderer.h"

#include "gpu/vulkanShader.h"
#include "computerAPI/fileLoader.h"
#include "computerAPI/directoryManager.h"
#include "gpu/vulkanInstance.h"

#include <stb_image.h>

VulkanChunkRenderer::VulkanChunkRenderer(VkRenderPass& renderPass)
	: descriptorAllocator(100, {{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0.5f}, { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0.5f}}) {

	// ==================== TEXTURE setup =================================================

	// upload texture
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load((DirectoryManager::getResourceDirectory() / "logicTiles.png").string().c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	VkExtent3D size { (uint32_t)texWidth, (uint32_t)texHeight, 1};
	blockTexture = createImage(pixels, size, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
	stbi_image_free(pixels);

	// create layout and descriptor set
	DescriptorLayoutBuilder textureLayoutBuilder;
	textureLayoutBuilder.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	blockTextureDescriptorSetLayout = textureLayoutBuilder.build(VK_SHADER_STAGE_FRAGMENT_BIT);
	blockTextureDescriptor = descriptorAllocator.allocate(blockTextureDescriptorSetLayout);

	// create sampler
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	vkCreateSampler(VulkanInstance::get().getDevice(), &samplerInfo, nullptr, &blockTextureSampler);

	// write descriptor
	DescriptorWriter textureWriter;
	textureWriter.writeImage(0, blockTexture.imageView, blockTextureSampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	textureWriter.updateSet(VulkanInstance::get().getDevice(), blockTextureDescriptor);

	// ==================== STATE BUFFER setup =============================================
	// create layout and descriptor set
	DescriptorLayoutBuilder stateBufferBuilder;
	stateBufferBuilder.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	stateBufferDescriptorSetLayout = stateBufferBuilder.build(VK_SHADER_STAGE_VERTEX_BIT);

	// ==================== PIPELINE setup =================================================
	
	// load shaders
	VkShaderModule blockVertShader = createShaderModule(readFileAsBytes(DirectoryManager::getResourceDirectory() / "shaders/block.vert.spv"));
	VkShaderModule blockFragShader = createShaderModule(readFileAsBytes(DirectoryManager::getResourceDirectory() / "shaders/block.frag.spv"));
	VkShaderModule wireVertShader = createShaderModule(readFileAsBytes(DirectoryManager::getResourceDirectory() / "shaders/wire.vert.spv"));
	VkShaderModule wireFragShader = createShaderModule(readFileAsBytes(DirectoryManager::getResourceDirectory() / "shaders/wire.frag.spv"));
	
	PipelineInformation blockPipelineInfo{};
	blockPipelineInfo.vertShader = blockVertShader;
	blockPipelineInfo.fragShader = blockFragShader;
	blockPipelineInfo.renderPass = renderPass;
	blockPipelineInfo.vertexBindingDescriptions = BlockVertex::getBindingDescriptions();
	blockPipelineInfo.vertexAttributeDescriptions = BlockVertex::getAttributeDescriptions();
	blockPipelineInfo.pushConstants.push_back({sizeof(ChunkPushConstants), 0, VK_SHADER_STAGE_VERTEX_BIT});
	blockPipelineInfo.descriptorSets.push_back(stateBufferDescriptorSetLayout);
	blockPipelineInfo.descriptorSets.push_back(blockTextureDescriptorSetLayout);
	blockPipeline = std::make_unique<Pipeline>(blockPipelineInfo);
	
	PipelineInformation wirePipelineInfo{};
	wirePipelineInfo.vertShader = wireVertShader;
	wirePipelineInfo.fragShader = wireFragShader;
	wirePipelineInfo.renderPass = renderPass;
	wirePipelineInfo.vertexBindingDescriptions = WireVertex::getBindingDescriptions();
	wirePipelineInfo.vertexAttributeDescriptions = WireVertex::getAttributeDescriptions();
	wirePipelineInfo.pushConstants.push_back({sizeof(ChunkPushConstants), 0, VK_SHADER_STAGE_VERTEX_BIT});
	wirePipelineInfo.descriptorSets.push_back(stateBufferDescriptorSetLayout);
	wirePipeline = std::make_unique<Pipeline>(wirePipelineInfo);

	// destroy shader modules since we won't be recreating pipelines
	destroyShaderModule(blockVertShader);
	destroyShaderModule(blockFragShader);
	destroyShaderModule(wireVertShader);
	destroyShaderModule(wireFragShader);
}

VulkanChunkRenderer::~VulkanChunkRenderer() {
	destroyImage(blockTexture);
	vkDestroySampler(VulkanInstance::get().getDevice(), blockTextureSampler, nullptr);
	vkDestroyDescriptorSetLayout(VulkanInstance::get().getDevice(), blockTextureDescriptorSetLayout, nullptr);
	
	vkDestroyDescriptorSetLayout(VulkanInstance::get().getDevice(), stateBufferDescriptorSetLayout, nullptr);
}

void VulkanChunkRenderer::render(VulkanFrameData& frame, const glm::mat4& viewMatrix, const std::vector<std::shared_ptr<VulkanChunkAllocation>>& chunks) {
	// save chunk data to frame
	for (auto& chunk : chunks) {
		frame.getFrameLifetime().push(chunk);
	}

	// shared push constants
	ChunkPushConstants pushConstants{};
	pushConstants.mvp = viewMatrix;
        
	// block drawing pass
	{
		// bind render pipeline
		vkCmdBindPipeline(frame.getMainCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, blockPipeline->getHandle());
		
		// bind push constants
		vkCmdPushConstants(frame.getMainCommandBuffer(), blockPipeline->getLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ChunkPushConstants), &pushConstants);

		// bind texture descriptor
		vkCmdBindDescriptorSets(frame.getMainCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, blockPipeline->getLayout(), 1, 1, &blockTextureDescriptor, 0, nullptr);
        
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
		vkCmdPushConstants(frame.getMainCommandBuffer(), wirePipeline->getLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ChunkPushConstants), &pushConstants);

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
