#include "rmlRenderInterface.h"

// Geometry
Rml::CompiledGeometryHandle RmlRenderInterface::CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices) {
	return currentRenderer->CompileGeometry(vertices, indices);
}
void RmlRenderInterface::ReleaseGeometry(Rml::CompiledGeometryHandle geometry) {
	currentRenderer->ReleaseGeometry(geometry);
}
void RmlRenderInterface::RenderGeometry(Rml::CompiledGeometryHandle handle, Rml::Vector2f translation, Rml::TextureHandle texture) {
	currentRenderer->RenderGeometry(handle, translation, texture);
}

// Textures
Rml::TextureHandle RmlRenderInterface::LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source) {
	return currentRenderer->LoadTexture(texture_dimensions, source);
}
Rml::TextureHandle RmlRenderInterface::GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i source_dimensions) {
	return currentRenderer->GenerateTexture(source, source_dimensions);
}
void RmlRenderInterface::ReleaseTexture(Rml::TextureHandle texture_handle) {
	currentRenderer->ReleaseTexture(texture_handle);
}

// Scissor
void RmlRenderInterface::EnableScissorRegion(bool enable) {
	currentRenderer->EnableScissorRegion(enable);
}
void RmlRenderInterface::SetScissorRegion(Rml::Rectanglei region) {
	currentRenderer->SetScissorRegion(region);
}
