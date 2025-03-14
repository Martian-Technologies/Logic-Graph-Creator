#ifndef tensorMoveTool_h
#define tensorMoveTool_h

#include "backend/selection.h"
#include "selectionHelperTool.h"

class TensorCreationTool : public SelectionHelperTool {
public:
	void reset() override final {
		SelectionHelperTool::reset();
		selection = nullptr;
		tensorStage = -1;
		updateElements();
	}

	void activate() override final {
		SelectionHelperTool::activate();
		registerFunction("tool primary activate", std::bind(&TensorCreationTool::click, this, std::placeholders::_1));
		registerFunction("tool secondary activate", std::bind(&TensorCreationTool::unclick, this, std::placeholders::_1));
		registerFunction("tool rotate block cw", std::bind(&TensorCreationTool::confirm, this, std::placeholders::_1));
	}
	void updateElements() override final;

	bool click(const Event* event);
	bool unclick(const Event* event);
	bool confirm(const Event* event);
	
private:
	SharedSelection selection; 
	Position originPosition;
	Vector step;
	int tensorStage = -1;
};

#endif /* tensorMoveTool_h */
