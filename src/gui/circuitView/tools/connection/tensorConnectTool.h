#ifndef tensorConnectTool_h
#define tensorConnectTool_h

#include "backend/selection.h"
#include "../circuitToolHelper.h"

class TensorConnectTool : public CircuitToolHelper {
public:
	inline void reset() override final {
		outputStage = -1;
		makingOutput = true;
		outputSelection = nullptr;
		inputSelection = nullptr;
		updateElements();
	}

	void activate() override final {
		CircuitTool::activate();
		registerFunction("tool primary activate", std::bind(&TensorConnectTool::click, this, std::placeholders::_1));
		// registerFunction("tool primary deactivate", std::bind(&TensorConnectTool::stopPlaceBlock, this, std::placeholders::_1));
		registerFunction("tool secondary activate", std::bind(&TensorConnectTool::unclick, this, std::placeholders::_1));
		// registerFunction("tool secondary deactivate", std::bind(&TensorConnectTool::stopDeleteBlocks, this, std::placeholders::_1));
		registerFunction("tool rotate block cw", std::bind(&TensorConnectTool::confirm, this, std::placeholders::_1));
	}

	void updateElements() override final;

	bool click(const Event* event);
	bool unclick(const Event* event);
	bool confirm(const Event* event);

private:
	bool makingOutput = true;

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
