#ifndef mainRenderer_h
#define mainRenderer_h

#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Vertex.h>
#include <glm/ext/vector_float2.hpp>

#include "backend/evaluator/evaluator.h"
#include "gui/sdl/sdlWindow.h"

#include "mainRendererDefs.h"

#include "renderer/windowRenderer.h"
#include "vulkanInstance.h"

class MainRenderer {
public:
	static MainRenderer& get();

	// Windows ==================================================================================================================================
	WindowID registerWindow(SdlWindow* window);
	void resizeWindow(WindowID windowID, glm::vec2 size);
	void deregisterWindow(WindowID windowID);

	// RmlUI ====================================================================================================================================
	// Prepare for Rml
	void prepareForRmlRender(WindowID windowID);
	void endRmlRender(WindowID windowID);

	// Geometry resources
	Rml::CompiledGeometryHandle compileGeometry(WindowID windowID, Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices);
	void releaseGeometry(WindowID windowID, Rml::CompiledGeometryHandle geometry);

	// Texture resources
	Rml::TextureHandle loadTexture(WindowID windowID, Rml::Vector2i& texture_dimensions, const Rml::String& source);
	Rml::TextureHandle generateTexture(WindowID windowID, Rml::Span<const Rml::byte> source, Rml::Vector2i source_dimensions);
	void releaseTexture(WindowID windowID, Rml::TextureHandle texture_handle);

	// Rendering (per window)
	void renderGeometry(WindowID windowID, Rml::CompiledGeometryHandle handle, Rml::Vector2f translation, Rml::TextureHandle texture);
	void enableScissorRegion(WindowID windowID, bool enable);
	void setScissorRegion(WindowID windowID, Rml::Rectanglei region);
	
	// Viewports ================================================================================================================================
	ViewportID registerViewport(WindowID windowID, glm::vec2 origin, glm::vec2 size, Rml::Element* element); // tmp element
	void moveViewport(ViewportID viewportID, WindowID windowID, glm::vec2 origin, glm::vec2 size);
	void moveViewportView(ViewportID viewportIDID, FPosition topLeft, FPosition bottomRight);
	void setViewportCircuit(ViewportID viewportID, Circuit* circuit); // tmp circuit
	void setViewportEvaluator(ViewportID viewportID, Circuit* circuit, Evaluator* evaluator, Address address); // tmp circuit
	void deregisterViewport(ViewportID viewportID);
	// void sendDiff(ViewportID viewportID, ...)

	// elements
	ElementID addSelectionObjectElement(ViewportID viewportID, const SelectionObjectElement& selection);
	ElementID addSelectionElement(ViewportID viewportID, const SelectionElement& selection);
	void removeSelectionElement(ViewportID viewportID, ElementID id);

	ElementID addBlockPreview(ViewportID viewportID, BlockPreview&& blockPreview);
	void shiftBlockPreview(ViewportID viewportID, ElementID id, Vector shift);
	void removeBlockPreview(ViewportID viewportID, ElementID id);

	ElementID addConnectionPreview(ViewportID viewportID, const ConnectionPreview& connectionPreview);
	void removeConnectionPreview(ViewportID viewportID, ElementID id);

	ElementID addHalfConnectionPreview(ViewportID viewportID, const HalfConnectionPreview& halfConnectionPreview);
	void removeHalfConnectionPreview(ViewportID viewportID, ElementID id);
	
	// Block Data ===============================================================================================================================
		

private:
	inline WindowID getNewWindowID() { return ++lastWindowID; }
	inline ViewportID getNewViewportID() { return ++lastViewportID; }

	WindowID lastWindowID = 0;
	ViewportID lastViewportID = 0;
	std::map<WindowID, WindowRenderer> windowRenderers;
	std::map<ViewportID, ViewportRenderInterface> viewportRenderers;

	VulkanInstance vulkanInstance;
};

#endif /* mainRenderer_h */