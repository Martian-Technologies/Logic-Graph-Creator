#ifndef mainRenderer_h
#define mainRenderer_h

#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Vertex.h>
#include <glm/ext/vector_float2.hpp>

#include "backend/evaluator/evaluator.h"
#include "gui/sdl/sdlWindow.h"

#include "mainRendererDefs.h"

// Element Types ------------------------------
struct SelectionElement {
	SelectionElement() = default;
	SelectionElement(Position topLeft, Position bottomRight, bool inverted = false)
		: topLeft(topLeft), bottomRight(bottomRight), inverted(inverted) { }

	SelectionElement(Position topLeft, bool inverted = false)
		: topLeft(topLeft), bottomRight(topLeft), inverted(inverted) { }

	Position topLeft;
	Position bottomRight;
	bool inverted;
};

struct SelectionObjectElement {
	enum RenderMode {
		SELECTION,
		SELECTION_INVERTED,
		ARROWS
	};
	SelectionObjectElement(SharedSelection selection, RenderMode renderMode = RenderMode::SELECTION)
		: selection(selection), renderMode(renderMode) { }

	SharedSelection selection;
	RenderMode renderMode;
};

struct BlockPreview {
	struct Block {
		Block(BlockType type, Position position, Orientation orientation)
		: type(type), position(position), orientation(orientation) { }
		BlockType type;
		Position position;
		Orientation orientation;
	};

	BlockPreview() = default;
	BlockPreview(BlockType type, Position position, Orientation orientation)
		: blocks({BlockPreview::Block(type, position, orientation)}) { }
	BlockPreview(std::vector<Block>&& blocks) : blocks(std::move(blocks)) {}

	std::vector<Block> blocks;
};

struct ConnectionPreview {
	ConnectionPreview() = default;
	ConnectionPreview(Position output, Position input)
		: output(output), input(input) { }

	Position output;
	Position input;
};

struct HalfConnectionPreview {
	HalfConnectionPreview() = default;
	HalfConnectionPreview(Position output, FPosition input)
		: output(output), input(input) { }

	Position output;
	FPosition input;
};

class MainRenderer {
public:
	static MainRenderer& get();

	// Windows ==================================================================================================================================
	WindowID registerWindow(SdlWindow window);
	void resizeWindow(WindowID window, glm::vec2 size);
	void deregisterWindow(WindowID window);

	// RmlUI ====================================================================================================================================
	// Prepare for Rml
	void prepareForRml(WindowID windowID);
	void endRml(WindowID windowID);

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
	void moveViewportView(ViewportID viewportID, FPosition viewCenter, float viewScale);
	void setViewportCircuit(ViewportID viewport, Circuit* circuit); // tmp circuit
	void setViewportEvaluator(ViewportID viewport, Circuit* circuit, Evaluator* evaluator, Address address); // tmp circuit
	void deregisterViewport(ViewportID viewport);
	// void sendDiff(ViewportID viewport, ...)

	// elements
	virtual ElementID addSelectionObjectElement(ViewportID viewport, const SelectionObjectElement& selection) = 0;
	virtual ElementID addSelectionElement(ViewportID viewport, const SelectionElement& selection) = 0;
	virtual void removeSelectionElement(ViewportID viewport, ElementID id) = 0;

	virtual ElementID addBlockPreview(ViewportID viewport, BlockPreview&& blockPreview) = 0;
	virtual void shiftBlockPreview(ViewportID viewport, ElementID id, Vector shift) = 0;
	virtual void removeBlockPreview(ViewportID viewport, ElementID id) = 0;

	virtual ElementID addConnectionPreview(ViewportID viewport, const ConnectionPreview& connectionPreview) = 0;
	virtual void removeConnectionPreview(ViewportID viewport, ElementID id) = 0;

	virtual ElementID addHalfConnectionPreview(ViewportID viewport, const HalfConnectionPreview& halfConnectionPreview) = 0;
	virtual void removeHalfConnectionPreview(ViewportID viewport, ElementID id) = 0;
	
	// Block Data ===============================================================================================================================
		

private:
};

#endif