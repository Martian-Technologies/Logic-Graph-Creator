#ifndef tensorConnectTool_h
#define tensorConnectTool_h

#include "backend/selection.h"
#include "circuitTool.h"

class TensorConnectTool : public CircuitTool {
public:
	inline void reset() override final {
		outputStage = -1;
		makingOutput = true;
		outputSelection = nullptr;
		inputSelection = nullptr;
		updateElements();
	}

	void initialize(ToolManagerEventRegister& toolManagerEventRegister) override final {
		CircuitTool::initialize(toolManagerEventRegister);
		toolManagerEventRegister.registerFunction("tool primary activate", std::bind(&TensorConnectTool::click, this, std::placeholders::_1));
		// toolManagerEventRegister.registerFunction("tool primary deactivate", std::bind(&TensorConnectTool::stopPlaceBlock, this, std::placeholders::_1));
		toolManagerEventRegister.registerFunction("tool secondary activate", std::bind(&TensorConnectTool::unclick, this, std::placeholders::_1));
		// toolManagerEventRegister.registerFunction("tool secondary deactivate", std::bind(&TensorConnectTool::stopDeleteBlocks, this, std::placeholders::_1));
		toolManagerEventRegister.registerFunction("pointer move", std::bind(&TensorConnectTool::pointerMove, this, std::placeholders::_1));
		// toolManagerEventRegister.registerFunction("pointer enter view", std::bind(&TensorConnectTool::enterBlockView, this, std::placeholders::_1));
		// toolManagerEventRegister.registerFunction("pointer exit view", std::bind(&TensorConnectTool::exitBlockView, this, std::placeholders::_1));
		toolManagerEventRegister.registerFunction("tool rotate block cw", std::bind(&TensorConnectTool::confirm, this, std::placeholders::_1));
	}

	bool click(const Event* event);
	bool unclick(const Event* event);
	bool confirm(const Event* event);
	bool pointerMove(const Event* event);

	// bool startPlaceBlock(const Event* event);
	// bool stopPlaceBlock(const Event* event);
	// bool startDeleteBlocks(const Event* event);
	// bool stopDeleteBlocks(const Event* event);
	// bool enterBlockView(const Event* event);
	// bool exitBlockView(const Event* event);

private:
	void updateElements();

	bool makingOutput = true;
	FPosition pointer;

	// output
	int outputStage = -1;
	Position outputPosition;
	SharedSelection outputSelection;

	// input
	int inputStage = -1;
	Position inputPosition;
	SharedSelection inputSelection;

	// tmp
	Vector step;
};

#endif /* tensorConnectTool_h */
