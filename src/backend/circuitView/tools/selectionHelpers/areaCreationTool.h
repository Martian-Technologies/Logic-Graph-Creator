#ifndef areaMoveTool_h
#define areaMoveTool_h

#include "backend/selection.h"
#include "selectionHelperTool.h"

class AreaCreationTool : public SelectionHelperTool {
public:
	void reset() override final {
		SelectionHelperTool::reset();
		hasOrigin = false;
		updateElements();
	}

	void activate() override final {
		SelectionHelperTool::activate();
		registerFunction("tool primary activate", std::bind(&AreaCreationTool::click, this, std::placeholders::_1));
		registerFunction("tool secondary activate", std::bind(&AreaCreationTool::unclick, this, std::placeholders::_1));
	}
	void updateElements() override final;

	bool click(const Event* event);
	bool unclick(const Event* event);
	
private:
	Position originPosition;
	bool hasOrigin = false;;
};

#endif /* tensorMoveTool_h */
