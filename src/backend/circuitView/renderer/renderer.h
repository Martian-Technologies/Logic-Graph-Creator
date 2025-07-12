#ifndef renderer_h
#define renderer_h

typedef unsigned int ElementID;

#include "backend/evaluator/evaluator.h"
#include "../viewManager/viewManager.h"
#include "backend/circuit/circuit.h"

// This is the renderer interface. It defines all the methods that non-windowing
// or renderer-specific code should be able to call. Each implementation is
// responsible for defining a render function and calling it in a loop.

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
		Block(BlockType type, Position position, Rotation rotation)
		: type(type), position(position), rotation(rotation) { }
		BlockType type;
		Position position;
		Rotation rotation;
	};

	BlockPreview() = default;
	BlockPreview(BlockType type, Position position, Rotation rotation)
		: blocks({BlockPreview::Block(type, position, rotation)}) { }
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

// Renderer -----------------------------------

class Renderer {
	friend class ElementCreator;
public:
	// main flow
	virtual void setCircuit(Circuit* circuit) = 0;
	virtual void setEvaluator(std::shared_ptr<Evaluator> evaluator) = 0;
	virtual void setAddress(const Address& address) = 0;

	virtual void updateView(ViewManager* viewManager) = 0;
	virtual void updateCircuit(DifferenceSharedPtr diff) = 0;

	virtual float getLastFrameTimeMs() const = 0;

private:
	// elements
	virtual ElementID addSelectionObjectElement(const SelectionObjectElement& selection) = 0;
	virtual ElementID addSelectionElement(const SelectionElement& selection) = 0;
	virtual void removeSelectionElement(ElementID id) = 0;

	virtual ElementID addBlockPreview(BlockPreview&& blockPreview) = 0;
	virtual void shiftBlockPreview(ElementID id, Vector shift) = 0;
	virtual void removeBlockPreview(ElementID id) = 0;

	virtual ElementID addConnectionPreview(const ConnectionPreview& connectionPreview) = 0;
	virtual void removeConnectionPreview(ElementID id) = 0;

	virtual ElementID addHalfConnectionPreview(const HalfConnectionPreview& halfConnectionPreview) = 0;
	virtual void removeHalfConnectionPreview(ElementID id) = 0;

	virtual void spawnConfetti(FPosition start) = 0;
};

#endif /* renderer_h */
