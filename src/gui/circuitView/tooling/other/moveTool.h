#ifndef moveTool_h
#define moveTool_h

#include "backend/selection.h"
#include "../circuitTool.h"

class MoveTool : public CircuitTool {
public:
	inline void reset() override final {
		stage = 'o';
		originSelection = nullptr;
		tensorStage = -1;
		updateElements();
	}

	void activate() override final {
		CircuitTool::activate();
		registerFunction("tool primary activate", std::bind(&MoveTool::click, this, std::placeholders::_1));
		registerFunction("tool secondary activate", std::bind(&MoveTool::unclick, this, std::placeholders::_1));
		registerFunction("pointer move", std::bind(&MoveTool::pointerMove, this, std::placeholders::_1));
		// registerFunction("pointer enter view", std::bind(&MoveTool::enterBlockView, this, std::placeholders::_1));
		// registerFunction("pointer exit view", std::bind(&MoveTool::exitBlockView, this, std::placeholders::_1));
		registerFunction("tool rotate block cw", std::bind(&MoveTool::confirm, this, std::placeholders::_1));
	}

	bool click(const Event* event);
	bool unclick(const Event* event);
	bool confirm(const Event* event);
	bool pointerMove(const Event* event);

	// bool startPlaceBlock(const Event* event);
	// bool stopPlaceBlock(const Event* event);
	// bool startDeleteBlocks(const Event* event);
	// bool stopDeleteBlocks(const Event* event);
	// bool enterBlockView(const Event* event);
	// bool exitBlockView(const Event* event);

private:
	void updateElements();

	char stage = 'o';

	FPosition pointer;

	// blocks
	Position originPosition;
	SharedSelection originSelection;

	// tensor stuff
	Vector step;
	int tensorStage = -1;

	// destination
	Position destination;
};

#endif /* moveTool_h */
