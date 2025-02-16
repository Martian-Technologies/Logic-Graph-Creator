#ifndef areaPlaceTool_h
#define areaPlaceTool_h

#include "baseBlockPlacementTool.h"

class AreaPlaceTool : public BaseBlockPlacementTool {
protected:
	void activate(ToolManagerEventRegister& toolManagerEventRegister) override final;
	inline void reset() override final { click = 'n'; }

	bool startPlaceBlock(const Event* event);
	bool startDeleteBlocks(const Event* event);
	bool pointerMove(const Event* event);
	bool enterBlockView(const Event* event);
	bool exitBlockView(const Event* event);

private:
	Position clickPosition;
	char click = 'n';
};

#endif /* areaPlaceTool_h */
