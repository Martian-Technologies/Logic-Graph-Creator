#ifndef windowRenderer_h
#define windowRenderer_h

#include <RmlUi/Core/RenderInterface.h>

#include "gpu/renderer/vulkanFrame.h"
#include "gui/sdl/sdlWindow.h"
#include "gpu/renderer/vulkanSwapchain.h"

class WindowRenderer {
public:
	WindowRenderer(SdlWindow* sdlWindow);
	~WindowRenderer();
	
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
	
	VkSurfaceKHR surface;
	VkRenderPass renderPass;
	Swapchain* swapchain = nullptr; // this should be a smart pointer, but I don't want to write a move constructor right now
	std::vector<VulkanFrameData> frames; // TODO - (this should be a std array once we have proper RAII)
};

#endif
