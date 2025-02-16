#ifndef baseBlockPlacementTool_h
#define baseBlockPlacementTool_h

#include "../circuitToolHelper.h"

class BaseBlockPlacementTool : public CircuitToolHelper {
public:
	// This will also tell the tool to reset.
	inline void selectBlock(BlockType selectedBlock) { this->selectedBlock = selectedBlock; updateElements(); }
	inline void setRotation(Rotation rotation) { this->rotation = rotation; updateElements(); }

	void activate() override;

	bool rotateBlockCW(const Event* event);
	bool rotateBlockCCW(const Event* event);

protected:
	inline virtual void updateElements() { };

	BlockType selectedBlock = BlockType::NONE;
	Rotation rotation = Rotation::ZERO;
};

typedef std::shared_ptr<BaseBlockPlacementTool> SharedBaseBlockPlacementTool;

#endif /* baseBlockPlacementTool_h */
