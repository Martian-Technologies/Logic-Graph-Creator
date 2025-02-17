#ifndef areaPlaceTool_h
#define areaPlaceTool_h

#include "baseBlockPlacementTool.h"

class AreaPlaceTool : public BaseBlockPlacementTool {
public:
	inline void reset() override final { click = 'n'; }
	
	void activate() override final;

	bool startPlaceBlock(const Event* event);
	bool startDeleteBlocks(const Event* event);
	bool pointerMove(const Event* event);
	bool enterBlockView(const Event* event);
	bool exitBlockView(const Event* event);

protected:
	void updateElements() override;

private:
	Position clickPosition;
	char click = 'n';
};

#endif /* areaPlaceTool_h */
