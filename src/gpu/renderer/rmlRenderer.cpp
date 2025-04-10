#include "rmlRenderer.h"

#include "vulkanFrame.h"
#include "gpu/vulkanInstance.h"
#include "computerAPI/directoryManager.h"
#include "computerAPI/fileLoader.h"
#include "gpu/vulkanShader.h"

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
	vertexBuffer = createBuffer(vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO);
	indexBuffer = createBuffer(indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO);

	vmaCopyMemoryToAllocation(VulkanInstance::get().getAllocator(), vertices.data(), vertexBuffer.allocation, 0, vertexBufferSize);
	vmaCopyMemoryToAllocation(VulkanInstance::get().getAllocator(), indices.data(), indexBuffer.allocation, 0, indexBufferSize);
}

RmlGeometryAllocation::~RmlGeometryAllocation() {
	destroyBuffer(indexBuffer);
	destroyBuffer(vertexBuffer);
}

// ================================= RML RENDERER ==================================================

RmlRenderer::RmlRenderer(VkRenderPass& renderPass, VkDescriptorSetLayout viewLayout) {
	// set up pipeline
	VkShaderModule rmlVertShader = createShaderModule(readFileAsBytes(DirectoryManager::getResourceDirectory() / "shaders/rml.vert.spv"));
	VkShaderModule rmlFragShader = createShaderModule(readFileAsBytes(DirectoryManager::getResourceDirectory() / "shaders/rml.frag.spv"));
	PipelineInformation rmlPipelineInfo{};
	rmlPipelineInfo.vertShader = rmlVertShader;
	rmlPipelineInfo.fragShader = rmlFragShader;
	rmlPipelineInfo.renderPass = renderPass;
	rmlPipelineInfo.vertexBindingDescriptions = RmlVertex::getBindingDescriptions();
	rmlPipelineInfo.vertexAttributeDescriptions = RmlVertex::getAttributeDescriptions();
	rmlPipelineInfo.pushConstantSize = sizeof(RmlPushConstants);
	rmlPipelineInfo.descriptorSets.push_back(viewLayout);
	rmlPipelineInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	pipeline = std::make_unique<Pipeline>(rmlPipelineInfo);
	destroyShaderModule(rmlVertShader);
	destroyShaderModule(rmlFragShader);
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
	
	// bind pipeline
	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getHandle());
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

	// set scissor (default)
	VkRect2D defaultScissor{};
	defaultScissor.offset = {0, 0};
	defaultScissor.extent = extent;
	vkCmdSetScissor(cmd, 0, 1, &defaultScissor);

	// set up data for custom scissor
	bool customScissorEnabled = false;
	VkRect2D customScissor{};
	customScissor.offset = {0, 0};
	customScissor.extent = extent;

	// view data descriptor
	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getLayout(), 0, 1, &viewDataSet, 0, nullptr);
	
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

			// upload push constants
			pushConstants.translation = renderInstruction.translation;
			vkCmdPushConstants(cmd, pipeline->getLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(RmlPushConstants), &pushConstants);

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
	rmlGeometryAllocations[newHandle] = std::make_shared<RmlGeometryAllocation>(vertices, indices);
	
	return newHandle;
}
void RmlRenderer::ReleaseGeometry(Rml::CompiledGeometryHandle geometry) {
	rmlGeometryAllocations.erase(geometry);
}
void RmlRenderer::RenderGeometry(Rml::CompiledGeometryHandle handle, Rml::Vector2f translation, Rml::TextureHandle texture) {
	// find geometry
	auto geometryItr = rmlGeometryAllocations.find(handle);
	if (geometryItr == rmlGeometryAllocations.end()) {
		logError("tried to render non-existent RML geometry", "Vulkan");
		return;
	}
	
	tempRenderInstructions.push_back(RmlDrawInstruction(geometryItr->second, {translation.x, translation.y}));
}

// Textures
Rml::TextureHandle RmlRenderer::LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source) {
	logInfo("texture load");
	return 1;
}
Rml::TextureHandle RmlRenderer::GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i source_dimensions) {
	logInfo("texture gen");
	return 1;
}
void RmlRenderer::ReleaseTexture(Rml::TextureHandle texture_handle) {
	logInfo("texture release");
}

// Scissor
void RmlRenderer::EnableScissorRegion(bool enable) {
	tempRenderInstructions.push_back(RmlEnableScissorInstruction(true));
}
void RmlRenderer::SetScissorRegion(Rml::Rectanglei region) {
	tempRenderInstructions.push_back(RmlSetScissorInstruction({region.Left(), region.Top()}, {region.Width(), region.Height()}));
}
