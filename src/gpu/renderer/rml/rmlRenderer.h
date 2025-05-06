#ifndef rmlRenderer_h
#define rmlRenderer_h

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float2.hpp>
#include <RmlUi/Core/Vertex.h>

#include "gpu/abstractions/vulkanDescriptor.h"
#include "gpu/abstractions/vulkanBuffer.h"
#include "gpu/abstractions/vulkanImage.h"
#include "gpu/abstractions/vulkanPipeline.h"
#include "gpu/renderer/frameManager.h"

// =========================== RML GEOMETRY =================================

struct RmlVertex : Rml::Vertex {
	static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
	static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
};

class RmlGeometryAllocation {
public:
	RmlGeometryAllocation(VulkanDevice* device, Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices);
	~RmlGeometryAllocation();

	inline AllocatedBuffer& getVertexBuffer() { return vertexBuffer; }
	inline AllocatedBuffer& getIndexBuffer() { return indexBuffer; }
	inline unsigned int getNumIndices() { return numIndices; }
	
private:
	AllocatedBuffer vertexBuffer, indexBuffer;
	unsigned int numIndices;

	VulkanDevice* device;
};

// ============================= RML TEXTURES ===================================
class RmlTexture {
public:
	RmlTexture(VulkanDevice* device, void* data, VkExtent3D size, VkDescriptorSet myDescriptor);
	~RmlTexture();

	inline VkDescriptorSet& getDescriptor() { return descriptor; }

private:
	AllocatedImage image;
	VkSampler sampler;
	VkDescriptorSet descriptor;

	VulkanDevice* device;
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

struct RmlPushConstants {
	glm::mat4 pixelViewMat;
	glm::vec2 translation;
};

class RmlRenderer {
public:
	void init(VulkanDevice* device, VkRenderPass& renderPass);
	void cleanup();

	void prepareForRmlRender();
	void endRmlRender();

	void render(Frame& frame, VkExtent2D windowExtent);
	
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
	// pipelines
	Pipeline untexturedPipeline;
	Pipeline texturedPipeline;

	// geometry allocations
	Rml::CompiledGeometryHandle currentGeometryHandle = 1;
	std::unordered_map<Rml::CompiledGeometryHandle, std::shared_ptr<RmlGeometryAllocation>> geometryAllocations;

	// texture allocations
	Rml::TextureHandle currentTextureHandle = 1;
	std::unordered_map<Rml::CompiledGeometryHandle, std::shared_ptr<RmlTexture>> textures;
	
	// texture descriptor
	DescriptorAllocator descriptorAllocator;
	VkDescriptorSetLayout singleImageDescriptorSetLayout;
	
	// render instructions
	std::vector<RmlRenderInstruction> renderInstructions;
	std::vector<RmlRenderInstruction> tempRenderInstructions;
	std::mutex rmlInstructionMux;

	// refs
	VulkanDevice* device;
};

#endif
