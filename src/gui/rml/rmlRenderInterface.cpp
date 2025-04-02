#include "rmlRenderInterface.h"

// Geometry
Rml::CompiledGeometryHandle RmlRenderInterface::CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices) {
	return currentWindowRenderer->CompileGeometry(vertices, indices);
}
void RmlRenderInterface::ReleaseGeometry(Rml::CompiledGeometryHandle geometry) {
	currentWindowRenderer->ReleaseGeometry(geometry);
}
void RmlRenderInterface::RenderGeometry(Rml::CompiledGeometryHandle handle, Rml::Vector2f translation, Rml::TextureHandle texture) {
	currentWindowRenderer->RenderGeometry(handle, translation, texture);
}

// Textures
Rml::TextureHandle RmlRenderInterface::LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source) {
	return currentWindowRenderer->LoadTexture(texture_dimensions, source);
}
Rml::TextureHandle RmlRenderInterface::GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i source_dimensions) {
	return currentWindowRenderer->GenerateTexture(source, source_dimensions);
}
void RmlRenderInterface::ReleaseTexture(Rml::TextureHandle texture_handle) {
	currentWindowRenderer->ReleaseTexture(texture_handle);
}

// Scissor
void RmlRenderInterface::EnableScissorRegion(bool enable) {
	currentWindowRenderer->EnableScissorRegion(enable);
}
void RmlRenderInterface::SetScissorRegion(Rml::Rectanglei region) {
	currentWindowRenderer->SetScissorRegion(region);
}
