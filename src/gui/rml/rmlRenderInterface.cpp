#include "rmlRenderInterface.h"
#include "gpu/mainRenderer.h"

// Geometry
Rml::CompiledGeometryHandle RmlRenderInterface::CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices) {
	return MainRenderer::get().compileGeometry(currentWindowID, vertices, indices);
}
void RmlRenderInterface::ReleaseGeometry(Rml::CompiledGeometryHandle geometry) {
	MainRenderer::get().releaseGeometry(currentWindowID, geometry);
}
void RmlRenderInterface::RenderGeometry(Rml::CompiledGeometryHandle handle, Rml::Vector2f translation, Rml::TextureHandle texture) {
	MainRenderer::get().renderGeometry(currentWindowID, handle, translation, texture);
}

// Textures
Rml::TextureHandle RmlRenderInterface::LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source) {
	return MainRenderer::get().loadTexture(currentWindowID, texture_dimensions, source);
}
Rml::TextureHandle RmlRenderInterface::GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i source_dimensions) {
	return MainRenderer::get().generateTexture(currentWindowID, source, source_dimensions);
}
void RmlRenderInterface::ReleaseTexture(Rml::TextureHandle texture_handle) {
	if (currentWindowID == 0) return;
	MainRenderer::get().releaseTexture(currentWindowID, texture_handle);
}

// Scissor
void RmlRenderInterface::EnableScissorRegion(bool enable) {
	MainRenderer::get().enableScissorRegion(currentWindowID, enable);
}
void RmlRenderInterface::SetScissorRegion(Rml::Rectanglei region) {
	MainRenderer::get().setScissorRegion(currentWindowID, region);
}
