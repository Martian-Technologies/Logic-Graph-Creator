#ifndef mainRenderer_h
#define mainRenderer_h

#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Vertex.h>
#include <glm/ext/vector_float2.hpp>

#include "backend/evaluator/evaluator.h"
#include "gui/sdl/sdlWindow.h"

typedef uint32_t WindowID;
typedef uint32_t ViewportID;

class MainRenderer {
public:
	static MainRenderer& get();

	// Windows ==================================================================================================================================
	WindowID registerWindow(SdlWindow window);
	void resizeWindow(WindowID window, glm::vec2 size);
	void deregisterWindow(WindowID window);

	// RmlUI ====================================================================================================================================
	// Geometry resources
	Rml::CompiledGeometryHandle CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices);
	void ReleaseGeometry(Rml::CompiledGeometryHandle geometry);

	// Texture resources
	Rml::TextureHandle LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source);
	Rml::TextureHandle GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i source_dimensions);
	void ReleaseTexture(Rml::TextureHandle texture_handle);

	// Rendering (per window)
	void RenderGeometry(WindowID window, Rml::CompiledGeometryHandle handle, Rml::Vector2f translation, Rml::TextureHandle texture);
	void EnableScissorRegion(WindowID window, bool enable);
	void SetScissorRegion(WindowID window, Rml::Rectanglei region);
	
	// Viewports ================================================================================================================================
	ViewportID registerViewport(WindowID window, glm::vec2 origin, glm::vec2 size);
	void moveViewport(ViewportID viewport, WindowID window, glm::vec2 origin, glm::vec2 size);
	void setViewportEvaluator(Evaluator* evaluator, Address address);
	void deregisterViewport(ViewportID viewport);
	// void sendDiff(ViewportID viewport, ...)
	
	// Block Data ===============================================================================================================================

private:
};

#endif