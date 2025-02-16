#ifndef blockPlacementTool_h
#define blockPlacementTool_h

#include "baseBlockPlacementTool.h"
#include "singlePlaceTool.h"
#include "areaPlaceTool.h"
#include "../circuitTool.h"

class BlockPlacementTool : public CircuitTool {
public:
	BlockPlacementTool() {
		activePlacementTool = std::make_shared<SinglePlaceTool>();
	}

	void activate(ToolManagerEventRegister& toolManagerEventRegister) override final { if (activePlacementTool) toolManagerInterface->pushTool(activePlacementTool); }

	std::vector<std::string> getModes() override final { return {"Single", "Area"}; }
	void setMode(std::string toolMode) override final {
		if (mode != toolMode) {
			if (toolMode == "None") {
				activePlacementTool = nullptr;
			} else if (toolMode == "Single") {
				activePlacementTool = std::make_shared<SinglePlaceTool>();
			} else if (toolMode == "Area") {
				activePlacementTool = std::make_shared<AreaPlaceTool>();
			}
		}
	}

	inline void selectBlock(BlockType selectedBlock) {
		this->selectedBlock = selectedBlock;
		if (activePlacementTool) activePlacementTool->selectBlock(selectedBlock);
	}

	inline void setRotation(Rotation rotation) {
		this->rotation = rotation;
		if (activePlacementTool) activePlacementTool->setRotation(rotation);
	}

private:
	std::string mode = "None";
	SharedBaseBlockPlacementTool activePlacementTool = nullptr;
	BlockType selectedBlock = BlockType::NONE;
	Rotation rotation = Rotation::ZERO;
};

typedef std::shared_ptr<BlockPlacementTool> SharedBlockPlacementTool;

#endif /* blockPlacementTool_h */
