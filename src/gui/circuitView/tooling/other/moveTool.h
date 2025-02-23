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
		registerFunction("tool rotate block cw", std::bind(&MoveTool::confirm, this, std::placeholders::_1));
	}
	void updateElements() override final;

	bool click(const Event* event);
	bool unclick(const Event* event);
	bool confirm(const Event* event);
	
	private:
	
	char stage = 'o';
	
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
