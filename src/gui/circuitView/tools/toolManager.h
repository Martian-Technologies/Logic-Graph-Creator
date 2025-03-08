#ifndef toolManager_h
#define toolManager_h

#include "toolStack.h"

class ToolManager {
public:
	inline ToolManager(EventRegister* eventRegister, Renderer* renderer) :
		toolStacks { ToolStack(eventRegister, renderer), ToolStack(eventRegister, renderer), ToolStack(eventRegister, renderer) } {
		selectedTools[0] = { "none", nullptr };
		selectedTools[1] = { "none", nullptr };
		selectedTools[2] = { "none", nullptr };
		toolStacks[activeToolStack].activate();
	}

	void selectStack(int stack);
	int getStack() const { return activeToolStack; }
	void selectBlock(BlockType blockType);
	void selectTool(std::string toolName);
	inline CircuitTool* getSelectedTool() { return selectedTools[activeToolStack].second.get(); }
	inline std::string getSelectedToolName() { return selectedTools[activeToolStack].first; }

	void setMode(std::string mode);

	inline void setCircuit(Circuit* circuit) {
		for (auto& toolStack : toolStacks) toolStack.setCircuit(circuit);
	}
	inline void setEvaluatorStateInterface(EvaluatorStateInterface* interface) {
		for (auto& toolStack : toolStacks) toolStack.setEvaluatorStateInterface(interface);
	}

private:
	template<class ToolClass>
	void instanceNewtool(std::string name, int stack) {
		if (activeToolStack != stack) {
			if (activeToolStack != -1) toolStacks[activeToolStack].deactivate();
			activeToolStack = stack;
			toolStacks[activeToolStack].activate();
		}
		SharedCircuitTool selectedTool = std::make_shared<ToolClass>();
		toolInstances[name] = { selectedTool, stack };
		selectedTools[activeToolStack] = { name, selectedTool };
		toolStacks[activeToolStack].clearTools();
		toolStacks[activeToolStack].pushTool(selectedTool);
	}

	int activeToolStack = 0;
	std::array<ToolStack, 3> toolStacks;
	std::array<std::pair<std::string, SharedCircuitTool>, 3> selectedTools;

	std::map<std::string, std::pair<SharedCircuitTool, int>> toolInstances;
};

#endif /* toolManager_h */