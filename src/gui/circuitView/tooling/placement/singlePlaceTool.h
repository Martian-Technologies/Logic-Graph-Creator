#ifndef singlePlaceTool_h
#define singlePlaceTool_h

#include "baseBlockPlacementTool.h"

class SinglePlaceTool : public BaseBlockPlacementTool {
public:
	inline void reset() override final { memset(clicks, 'n', 2); }

	void activate(ToolManagerEventRegister& toolManagerEventRegister) override final;

	bool startPlaceBlock(const Event* event);
	bool stopPlaceBlock(const Event* event);
	bool startDeleteBlocks(const Event* event);
	bool stopDeleteBlocks(const Event* event);
	bool pointerMove(const Event* event);
	bool enterBlockView(const Event* event);
	bool exitBlockView(const Event* event);

private:
	void updateElements() override;

	Position position;
	char clicks[2] = { 'n', 'n' };
};

#endif /* SinglePlaceTool_h */
