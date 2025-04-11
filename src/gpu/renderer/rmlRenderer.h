#ifndef rmlRenderer_h
#define rmlRenderer_h

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float2.hpp>
#include <RmlUi/Core/Vertex.h>

#include "gpu/renderer/vulkanDescriptor.h"
#include "gpu/vulkanBuffer.h"
#include "gpu/vulkanImage.h"
#include "vulkanPipeline.h"
#include "vulkanFrame.h"

// =========================== RML GEOMETRY =================================

struct RmlVertex : Rml::Vertex {
	static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
	static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
};

class RmlGeometryAllocation {
public:
	RmlGeometryAllocation(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices);
	~RmlGeometryAllocation();

	inline AllocatedBuffer& getVertexBuffer() { return vertexBuffer; }
	inline AllocatedBuffer& getIndexBuffer() { return indexBuffer; }
	inline unsigned int getNumIndices() { return numIndices; }
	
private:
	AllocatedBuffer vertexBuffer, indexBuffer;
	unsigned int numIndices;
};

struct RmlPushConstants {
	glm::vec2 translation;
};

// ============================= RML TEXTURES ===================================
class RmlTexture {
public:
	RmlTexture(void* data, VkExtent3D size, VkDescriptorSet myDescriptor);
	~RmlTexture();

	inline VkDescriptorSet& getDescriptor() { return descriptor; }

private:
	AllocatedImage image;
	VkSampler sampler;
	VkDescriptorSet descriptor;
};

// =========================== RML INSTRUCTIONS =================================

struct RmlDrawInstruction {
	inline RmlDrawInstruction(std::shared_ptr<RmlGeometryAllocation> geometry, glm::vec2 translation, std::shared_ptr<RmlTexture> texture = nullptr)
		: geometry(geometry), translation(translation), texture(texture) {}
	
	std::shared_ptr<RmlGeometryAllocation> geometry;
	glm::vec2 translation;
	std::shared_ptr<RmlTexture> texture = nullptr; // optional
};

struct RmlSetScissorInstruction {
	inline RmlSetScissorInstruction(glm::vec2 offset, glm::vec2 size)
		: offset(offset), size(size) {}
	
	glm::vec2 offset, size;
};

struct RmlEnableScissorInstruction {
	inline RmlEnableScissorInstruction(bool state)
		: state(state) {}
	
	bool state;
};

typedef std::variant<RmlDrawInstruction, RmlSetScissorInstruction, RmlEnableScissorInstruction> RmlRenderInstruction;

// ========================= RML RENDERER ====================================

class RmlRenderer {
public:
	RmlRenderer(VkRenderPass& renderPass, VkDescriptorSetLayout viewLayout);
	~RmlRenderer();

	void prepareForRmlRender();
	void endRmlRender();

	void render(VulkanFrameData& frame, VkExtent2D windowExtent, VkDescriptorSet viewDataSet);
	
public:
	// -- Rml::RenderInterface --
	Rml::CompiledGeometryHandle CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices);
	void ReleaseGeometry(Rml::CompiledGeometryHandle geometry);
	void RenderGeometry(Rml::CompiledGeometryHandle handle, Rml::Vector2f translation, Rml::TextureHandle texture);

	Rml::TextureHandle LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source);
	Rml::TextureHandle GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i source_dimensions);
	void ReleaseTexture(Rml::TextureHandle texture_handle);

	void EnableScissorRegion(bool enable);
	void SetScissorRegion(Rml::Rectanglei region);
private:
	std::unique_ptr<Pipeline> untexturedPipeline;
	std::unique_ptr<Pipeline> texturedPipeline;

	// geometry
	Rml::CompiledGeometryHandle currentGeometryHandle = 1;
	std::unordered_map<Rml::CompiledGeometryHandle, std::shared_ptr<RmlGeometryAllocation>> geometryAllocations;

	// textures
	Rml::TextureHandle currentTextureHandle = 1;
	std::unordered_map<Rml::CompiledGeometryHandle, std::shared_ptr<RmlTexture>> textures;
	// texture descriptor
	DescriptorAllocator descriptorAllocator;
	VkDescriptorSetLayout singleImageDescriptorSetLayout;
	
	// render instructions
	std::vector<RmlRenderInstruction> renderInstructions;
	std::vector<RmlRenderInstruction> tempRenderInstructions;
	std::mutex rmlInstructionMux;
};

#endif
