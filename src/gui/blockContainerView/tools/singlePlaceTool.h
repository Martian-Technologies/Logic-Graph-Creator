#ifndef singlePlaceTool_h
#define singlePlaceTool_h

#include "baseBlockPlacementTool.h"

class SinglePlaceTool : public BaseBlockPlacementTool {
public:
	inline void reset() override final { memset(clicks, 'n', 2); }

	void initialize(ToolManagerEventRegister& toolManagerEventRegister) override final {
		BaseBlockPlacementTool::initialize(toolManagerEventRegister);
		toolManagerEventRegister.registerFunction("tool primary activate", std::bind(&SinglePlaceTool::startPlaceBlock, this, std::placeholders::_1));
		toolManagerEventRegister.registerFunction("tool primary deactivate", std::bind(&SinglePlaceTool::stopPlaceBlock, this, std::placeholders::_1));
		toolManagerEventRegister.registerFunction("tool secondary activate", std::bind(&SinglePlaceTool::startDeleteBlocks, this, std::placeholders::_1));
		toolManagerEventRegister.registerFunction("tool secondary deactivate", std::bind(&SinglePlaceTool::stopDeleteBlocks, this, std::placeholders::_1));
		toolManagerEventRegister.registerFunction("pointer move", std::bind(&SinglePlaceTool::pointerMove, this, std::placeholders::_1));
		toolManagerEventRegister.registerFunction("pointer enter view", std::bind(&SinglePlaceTool::enterBlockView, this, std::placeholders::_1));
		toolManagerEventRegister.registerFunction("pointer exit view", std::bind(&SinglePlaceTool::exitBlockView, this, std::placeholders::_1));
	}

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
