#ifndef toolManager_h
#define toolManager_h

#include "toolStack.h"

class CircuitView;

class ToolManager {
public:
	inline ToolManager(EventRegister* eventRegister, ViewportID viewportID, CircuitView* circuitView) :
		toolStacks { ToolStack(eventRegister, viewportID, circuitView, this), ToolStack(eventRegister, viewportID, circuitView, this), ToolStack(eventRegister, viewportID, circuitView, this) } {
		toolStacks[activeToolStack].activate();
	}

	void selectStack(int stack);
	int getStack() const { return activeToolStack; }
	void selectBlock(BlockType blockType);
	SharedCircuitTool selectTool(SharedCircuitTool tool);

	void setMode(std::string mode);

	inline void setCircuit(Circuit* circuit) {
		this->circuit = circuit;
		for (auto& toolStack : toolStacks) toolStack.setCircuit(circuit);
	}

private:
	int activeToolStack = 0;
	Circuit* circuit;
	std::array<ToolStack, 3> toolStacks;
	std::map<std::string, SharedCircuitTool> toolInstances;
};

#endif /* toolManager_h */
