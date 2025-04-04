#ifndef windowRenderer_h
#define windowRenderer_h

#include <RmlUi/Core/RenderInterface.h>
#include <thread>

#include "gpu/renderer/vulkanFrame.h"
#include "gui/sdl/sdlWindow.h"
#include "gpu/renderer/vulkanSwapchain.h"

constexpr unsigned int FRAMES_IN_FLIGHT = 2;

class WindowRenderer {
public:
	WindowRenderer(SdlWindow* sdlWindow);
	~WindowRenderer();

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
	
private:
	SdlWindow* sdlWindow;
	VkDevice device;
	bool running = false;
	
	VkSurfaceKHR surface;
	VkRenderPass renderPass;
	Swapchain* swapchain = nullptr; // this should be a smart pointer, but I don't want to write a move constructor right nown

	// size
	std::pair<uint32_t, uint32_t> windowSize;
	std::mutex windowSizeMux;
	
	// frames
	std::vector<VulkanFrameData> frames; // TODO - (this should be a std array once we have proper RAII)
	int frameNumber = 0;
	inline VulkanFrameData& getCurrentFrame(int offset = 0) { return frames[(frameNumber + offset) % FRAMES_IN_FLIGHT]; };

	// render loop
	std::thread renderThread;
	void renderLoop();
};

#endif
