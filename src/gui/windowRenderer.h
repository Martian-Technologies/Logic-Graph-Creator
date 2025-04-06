#ifndef windowRenderer_h
#define windowRenderer_h

#include <RmlUi/Core/RenderInterface.h>
#include <thread>
#include <glm/mat4x4.hpp>

#include "gpu/renderer/rmlRenderer.h"
#include "gpu/renderer/vulkanFrame.h"
#include "gpu/renderer/vulkanPipeline.h"
#include "gui/sdl/sdlWindow.h"
#include "gpu/renderer/vulkanSwapchain.h"

constexpr unsigned int FRAMES_IN_FLIGHT = 2;

class WindowRenderer {
public:
	WindowRenderer(SdlWindow* sdlWindow);
	~WindowRenderer();

	// no copy
	WindowRenderer(const WindowRenderer&) = delete;
	WindowRenderer& operator=(const WindowRenderer&) = delete;

public:

	void resize(std::pair<uint32_t, uint32_t> windowSize);
	
	void prepareForRml();
	void endRml();
	
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
	void createRenderPass();
	void recreateSwapchain();
	void recordCommandBuffer(VulkanFrameData& frame, uint32_t imageIndex);
	
private:
	SdlWindow* sdlWindow;
	VkDevice device;
	std::atomic<bool> running = false;

	// main vulkan
	VkSurfaceKHR surface;
	VkRenderPass renderPass;
	std::unique_ptr<Swapchain> swapchain;
	std::atomic<bool> swapchainRecreationNeeded = false;
	std::unique_ptr<Pipeline> rmlPipeline;

	// size
	std::pair<uint32_t, uint32_t> windowSize;
	std::mutex windowSizeMux;
	glm::mat4 pixelViewMat;
	
	// frames
	std::vector<VulkanFrameData> frames; // TODO - (this should be a std array once we have proper RAII)
	int frameNumber = 0;
	inline VulkanFrameData& getCurrentFrame(int offset = 0) { return frames[(frameNumber + offset) % FRAMES_IN_FLIGHT]; };

	// rml geometry
	Rml::CompiledGeometryHandle currentGeometryHandle = 1;
	std::unordered_map<Rml::CompiledGeometryHandle, std::shared_ptr<RmlGeometryAllocation>> rmlGeometryAllocations;
	// rml instructions
	std::vector<RmlInstruction> rmlInstructions;
	std::vector<RmlInstruction> tempRmlInstructions;
	std::mutex rmlInstructionMux;

	// render loop
	std::thread renderThread;
	void renderLoop();
};

#endif
