#ifndef blockPlacementTool_h
#define blockPlacementTool_h

#include "baseBlockPlacementTool.h"
#include "singlePlaceTool.h"
#include "../circuitTool.h"

class BlockPlacementTool : public CircuitTool {
public:
	BlockPlacementTool() {
		activePlacementTool = std::make_shared<SinglePlaceTool>();
	}

	void initialize(ToolManagerEventRegister& toolManagerEventRegister) override {
		toolManagerInterface->pushTool(activePlacementTool);
	}

	// inline void setPlacementType(std::string toolType) {
	// 	// if (this->toolType == toolType) return;
	// 	// if (toolType == "Single Place") changeTool<SinglePlaceTool>();
	// 	// else if (toolType == "Area Place") changeTool<AreaPlaceTool>();
	// 	// else if (toolType == "NONE") clearTool();
	// 	// else return;
	// 	// this->toolType = toolType;
	// }

	inline void selectBlock(BlockType selectedBlock) {
		this->selectedBlock = selectedBlock;
		if (activePlacementTool) activePlacementTool->selectBlock(selectedBlock);
	}

	inline void setRotation(Rotation rotation) {
		this->rotation = rotation;
		if (activePlacementTool) activePlacementTool->setRotation(rotation);
	}

	inline Rotation getRotation() { return rotation; }
	inline BlockType getSelectedBlock() const { return selectedBlock; }
	
protected:
	SharedBaseBlockPlacementTool activePlacementTool = nullptr;
	BlockType selectedBlock = BlockType::NONE;
	Rotation rotation = Rotation::ZERO;
};

typedef std::shared_ptr<BlockPlacementTool> SharedBlockPlacementTool;

#endif /* blockPlacementTool_h */
