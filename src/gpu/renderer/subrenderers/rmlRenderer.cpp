#include "rmlRenderer.h"

#include "gpu/vulkanInstance.h"
#include "computerAPI/directoryManager.h"
#include "computerAPI/fileLoader.h"
#include "gpu/vulkanShader.h"
#include "stb_image.h"

std::vector<VkVertexInputBindingDescription> RmlVertex::getBindingDescriptions() {
	std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
	bindingDescriptions[0].binding = 0;
	bindingDescriptions[0].stride = sizeof(RmlVertex);
	bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> RmlVertex::getAttributeDescriptions() {
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions(3);

	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(RmlVertex, position);

	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R8G8B8A8_UNORM;
	attributeDescriptions[1].offset = offsetof(RmlVertex, colour);

	attributeDescriptions[2].binding = 0;
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[2].offset = offsetof(RmlVertex, tex_coord);

	return attributeDescriptions;
}

RmlGeometryAllocation::RmlGeometryAllocation(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices) {
	size_t vertexBufferSize = vertices.size() * sizeof(RmlVertex);
	size_t indexBufferSize = indices.size() * sizeof(int);
	numIndices = indices.size();
;
	vertexBuffer = createBuffer(vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
	indexBuffer = createBuffer(indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

	vmaCopyMemoryToAllocation(VulkanInstance::get().getAllocator(), vertices.data(), vertexBuffer.allocation, 0, vertexBufferSize);
	vmaCopyMemoryToAllocation(VulkanInstance::get().getAllocator(), indices.data(), indexBuffer.allocation, 0, indexBufferSize);
}

RmlGeometryAllocation::~RmlGeometryAllocation() {
	destroyBuffer(indexBuffer);
	destroyBuffer(vertexBuffer);
}

RmlTexture::RmlTexture(void* data, VkExtent3D size, VkDescriptorSet myDescriptor) {
	// create image
	image = createImage(data, size, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);

	// create image sampler
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	vkCreateSampler(VulkanInstance::get().getDevice(), &samplerInfo, nullptr, &sampler);

	// update image descriptor
	descriptor = myDescriptor;
	DescriptorWriter writer;
	writer.writeImage(0, image.imageView, sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	writer.updateSet(VulkanInstance::get().getDevice(), descriptor);
}

RmlTexture::~RmlTexture() {
	vkDestroySampler(VulkanInstance::get().getDevice(), sampler, nullptr);
	destroyImage(image);
}

// ================================= RML RENDERER ==================================================

RmlRenderer::RmlRenderer(VkRenderPass& renderPass, VkDescriptorSetLayout viewLayout)
	: descriptorAllocator(100, {{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 }}) {
	
	// set up image descriptor
	DescriptorLayoutBuilder layoutBuilder;
	layoutBuilder.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	singleImageDescriptorSetLayout = layoutBuilder.build(VK_SHADER_STAGE_FRAGMENT_BIT);
	
	// load shaders
	VkShaderModule rmlVertShader = createShaderModule(readFileAsBytes(DirectoryManager::getResourceDirectory() / "shaders/rml.vert.spv"));
	VkShaderModule rmlFragShaderUntextured = createShaderModule(readFileAsBytes(DirectoryManager::getResourceDirectory() / "shaders/rml.frag.spv"));
	VkShaderModule rmlFragShaderTextured = createShaderModule(readFileAsBytes(DirectoryManager::getResourceDirectory() / "shaders/rmlTextured.frag.spv"));

	// set up pipelines
	// untextured
	PipelineInformation rmlPipelineInfo{};
	rmlPipelineInfo.vertShader = rmlVertShader;
	rmlPipelineInfo.fragShader = rmlFragShaderUntextured;
	rmlPipelineInfo.renderPass = renderPass;
	rmlPipelineInfo.vertexBindingDescriptions = RmlVertex::getBindingDescriptions();
	rmlPipelineInfo.vertexAttributeDescriptions = RmlVertex::getAttributeDescriptions();
	rmlPipelineInfo.pushConstantSize = sizeof(RmlPushConstants);
	rmlPipelineInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rmlPipelineInfo.descriptorSets.push_back(viewLayout); // descriptor set 0 (view info)
	untexturedPipeline = std::make_unique<Pipeline>(rmlPipelineInfo);
	// textured
	rmlPipelineInfo.fragShader = rmlFragShaderTextured;
	rmlPipelineInfo.descriptorSets.push_back(singleImageDescriptorSetLayout); // descriptor set 1 (texture image)
	texturedPipeline = std::make_unique<Pipeline>(rmlPipelineInfo);

	// destroy shaders
	destroyShaderModule(rmlVertShader);
	destroyShaderModule(rmlFragShaderUntextured);
	destroyShaderModule(rmlFragShaderTextured);
}

RmlRenderer::~RmlRenderer() {
	vkDestroyDescriptorSetLayout(VulkanInstance::get().getDevice(), singleImageDescriptorSetLayout, nullptr);
}

void RmlRenderer::prepareForRmlRender() {
	// clear list of temp RML instructions so we can start adding
	tempRenderInstructions.clear();
}

void RmlRenderer::endRmlRender() {
	// swap the real instructions out for the new ones
	std::lock_guard<std::mutex> lock(rmlInstructionMux);
	renderInstructions = std::move(tempRenderInstructions);
}

void RmlRenderer::render(VulkanFrameData& frame, VkExtent2D windowExtent, VkDescriptorSet viewDataSet) {
	VkCommandBuffer cmd = frame.getMainCommandBuffer();
	
	// set viewport state
	VkExtent2D& extent = windowExtent;
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(extent.width);
	viewport.height = static_cast<float>(extent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(cmd, 0, 1, &viewport);

	// set default scissor
	VkRect2D defaultScissor{};
	defaultScissor.offset = {0, 0};
	defaultScissor.extent = extent;
	vkCmdSetScissor(cmd, 0, 1, &defaultScissor);

	// set up data for custom scissor
	bool customScissorEnabled = false;
	VkRect2D customScissor{};
	customScissor.offset = {0, 0};
	customScissor.extent = extent;

	// bind untextured pipeline by default
	bool pipelineRebindNeeded = true;
	Pipeline* currentPipeline = untexturedPipeline.get();
	
	// set up shared push constants data
	RmlPushConstants pushConstants{ glm::vec2(0.0f, 0.0f) };

	// go through and do render instructions
	std::lock_guard<std::mutex> lock(rmlInstructionMux);
	for (const auto& instruction : renderInstructions) {
		if (std::holds_alternative<RmlDrawInstruction>(instruction)) {
			// DRAW instruction
			const RmlDrawInstruction& renderInstruction = std::get<RmlDrawInstruction>(instruction);

			// Add geometry we are going to use to the frame
			frame.getRmlAllocations().push_back(renderInstruction.geometry);
			
			// update pipeline if needed
			bool texturedDraw = renderInstruction.texture != nullptr;
			Pipeline* newPipeline = texturedDraw ? texturedPipeline.get() : untexturedPipeline.get();
			if (newPipeline != currentPipeline || pipelineRebindNeeded) {
				currentPipeline = newPipeline;
				pipelineRebindNeeded = false;
				
				// bind untextured pipeline
				vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, currentPipeline->getHandle());
				// upload view data descriptor
				vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, currentPipeline->getLayout(), 0, 1, &viewDataSet, 0, nullptr);
			}

			// bind texture descriptor if needed
			if (texturedDraw) {
				vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, texturedPipeline->getLayout(), 1, 1, &renderInstruction.texture->getDescriptor(), 0, nullptr);
			}
	
			// upload push constants
			pushConstants.translation = renderInstruction.translation;
			vkCmdPushConstants(cmd, currentPipeline->getLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(RmlPushConstants), &pushConstants);

			// bind vertex buffer
			VkBuffer vertexBuffers[] = { renderInstruction.geometry->getVertexBuffer().buffer };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);

			// bind index buffer
			vkCmdBindIndexBuffer(cmd, renderInstruction.geometry->getIndexBuffer().buffer, offsets[0], VK_INDEX_TYPE_UINT32);

			// draw
			vkCmdDrawIndexed(cmd, renderInstruction.geometry->getNumIndices(), 1, 0, 0, 0);
			
		}
		else if (std::holds_alternative<RmlEnableScissorInstruction>(instruction)) {
			// ENABLE SCISSOR instruction
			const RmlEnableScissorInstruction& enableScissorInstruction = std::get<RmlEnableScissorInstruction>(instruction);

			customScissorEnabled = enableScissorInstruction.state;
			if (customScissorEnabled) {
				vkCmdSetScissor(cmd, 0, 1, &customScissor);
			}
			else {
				vkCmdSetScissor(cmd, 0, 1, &defaultScissor);
			}
		}
		else if (std::holds_alternative<RmlSetScissorInstruction>(instruction)) {
			// SET SCISSOR instruction
			const RmlSetScissorInstruction& setScissorInstruction = std::get<RmlSetScissorInstruction>(instruction);

			customScissor.offset = { (int)setScissorInstruction.offset.x, (int)setScissorInstruction.offset.y};
			customScissor.extent = { (unsigned int)setScissorInstruction.size.x, (unsigned int)setScissorInstruction.size.y};
			if (customScissorEnabled) vkCmdSetScissor(cmd, 0, 1, &customScissor);
		}
	}
}

// =================================== RENDER INTERFACE ==================================================
Rml::CompiledGeometryHandle RmlRenderer::CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices) {
	// get and increment handle
	Rml::CompiledGeometryHandle newHandle = currentGeometryHandle++;
	// alocate new geometry
	geometryAllocations[newHandle] = std::make_shared<RmlGeometryAllocation>(vertices, indices);
	
	return newHandle;
}
void RmlRenderer::ReleaseGeometry(Rml::CompiledGeometryHandle geometry) {
	geometryAllocations.erase(geometry);
}
void RmlRenderer::RenderGeometry(Rml::CompiledGeometryHandle handle, Rml::Vector2f translation, Rml::TextureHandle texture) {
	// find geometry
	auto geometryItr = geometryAllocations.find(handle);
	if (geometryItr == geometryAllocations.end()) {
		logError("tried to render non-existent RML geometry", "Vulkan");
		return;
	}

	// find texture if specified
	std::shared_ptr<RmlTexture> texturePtr = nullptr;
	if (texture != 0) {
		auto textureItr = textures.find(texture);
		if (textureItr != textures.end()) {
			texturePtr = textureItr->second;
		}
	}
	
	tempRenderInstructions.push_back(RmlDrawInstruction(geometryItr->second, {translation.x, translation.y}, texturePtr));
}

// Textures
Rml::TextureHandle RmlRenderer::LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source) {

	// get and increment handle
	Rml::TextureHandle newHandle = currentTextureHandle++;

	// load texture
	int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(source.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

	// allocate new texture
	VkExtent3D size { (uint32_t)texWidth, (uint32_t)texHeight, 1};
    textures[newHandle] = std::make_shared<RmlTexture>((void*)source.data(), size, descriptorAllocator.allocate(singleImageDescriptorSetLayout));

	// free pixels
	stbi_image_free(pixels);
	
	return newHandle;
}
Rml::TextureHandle RmlRenderer::GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i source_dimensions) {
	// get and increment handle
	Rml::TextureHandle newHandle = currentTextureHandle++;
	
	// alocate new texture
	VkExtent3D size;
	size.width = source_dimensions.x;
	size.height = source_dimensions.y;
	size.depth = 1;
	textures[newHandle] = std::make_shared<RmlTexture>((void*)source.data(), size, descriptorAllocator.allocate(singleImageDescriptorSetLayout));
	
	return newHandle;
}
void RmlRenderer::ReleaseTexture(Rml::TextureHandle texture_handle) {
	textures.erase(texture_handle);
}

// Scissor
void RmlRenderer::EnableScissorRegion(bool enable) {
	tempRenderInstructions.push_back(RmlEnableScissorInstruction(true));
}
void RmlRenderer::SetScissorRegion(Rml::Rectanglei region) {
	tempRenderInstructions.push_back(RmlSetScissorInstruction({region.Left(), region.Top()}, {region.Width(), region.Height()}));
}
