#include "windowRenderer.h"

#include "gpu/vulkanInstance.h"

WindowRenderer::WindowRenderer(SdlWindow* sdlWindow) {
	logInfo("Initializing window renderer...");

	this->sdlWindow = sdlWindow;
	vkSurface = sdlWindow->createSurface(VulkanInstance::get().getInstance());
	VulkanInstance::get().ensureDeviceCreation(vkSurface);
}


// -- Rml::RenderInterface --

Rml::CompiledGeometryHandle WindowRenderer::CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices) {
	return 1;
}
void WindowRenderer::ReleaseGeometry(Rml::CompiledGeometryHandle geometry) {
	
}
void WindowRenderer::RenderGeometry(Rml::CompiledGeometryHandle handle, Rml::Vector2f translation, Rml::TextureHandle texture) {
	
}

// Textures
Rml::TextureHandle WindowRenderer::LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source) {
	return 1;
}
Rml::TextureHandle WindowRenderer::GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i source_dimensions) {
	return 1;
}
void WindowRenderer::ReleaseTexture(Rml::TextureHandle texture_handle) {
	
}

// Scissor
void WindowRenderer::EnableScissorRegion(bool enable) {
	
}
void WindowRenderer::SetScissorRegion(Rml::Rectanglei region) {
	
}
