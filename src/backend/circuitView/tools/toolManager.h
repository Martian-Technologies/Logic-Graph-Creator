#ifndef toolManager_h
#define toolManager_h

#include "toolStack.h"

class CircuitView;

class ToolManager {
public:
	inline ToolManager(EventRegister* eventRegister, Renderer* renderer, CircuitView* circuitView) :
		toolStacks { ToolStack(eventRegister, renderer, circuitView), ToolStack(eventRegister, renderer, circuitView), ToolStack(eventRegister, renderer, circuitView) } {
		toolStacks[activeToolStack].activate();
	}

	void selectStack(int stack);
	int getStack() const { return activeToolStack; }
	void selectBlock(BlockType blockType);
	void selectTool(SharedCircuitTool tool);
	// inline CircuitTool* getSelectedTool() { return selectedTools[activeToolStack].second.get(); }
	// inline std::string getSelectedToolName() { return selectedTools[activeToolStack].first; }

	void setMode(std::string mode);

	inline void setCircuit(Circuit* circuit) {
		for (auto& toolStack : toolStacks) toolStack.setCircuit(circuit);
	}
	inline void setEvaluatorStateInterface(EvaluatorStateInterface* interface) {
		for (auto& toolStack : toolStacks) toolStack.setEvaluatorStateInterface(interface);
	}

private:
	int activeToolStack = 0;
	std::array<ToolStack, 3> toolStacks;
};

#endif /* toolManager_h */