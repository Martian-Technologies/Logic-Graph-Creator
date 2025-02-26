#ifndef toolManager_h
#define toolManager_h

#include "toolStack.h"

class ToolManager {
public:
	inline ToolManager(EventRegister* eventRegister, Renderer* renderer) : toolStack(eventRegister, renderer) { }

	void selectBlock(BlockType blockType);

	void selectTool(std::string toolName);
	inline CircuitTool* getSelectedTool() { return selectedTool.get(); }
	inline std::string getSelectedToolName() { return selectedToolName; }

	void setMode(std::string mode);

	inline void setCircuit(Circuit* circuit) { toolStack.setCircuit(circuit); }
	inline void setEvaluatorStateInterface(EvaluatorStateInterface* interface) { toolStack.setEvaluatorStateInterface(interface); }

private:
	template<class ToolClass>
	void instanceNewtool(std::string name) {
		selectedToolName = name;
		selectedTool = std::make_shared<ToolClass>();
		toolInstances[name] = selectedTool;
		toolStack.clearTools();
		toolStack.pushTool(selectedTool);
	}

	ToolStack toolStack;

	std::string selectedToolName;
	SharedCircuitTool selectedTool;

	std::map<std::string, SharedCircuitTool> toolInstances;
};

#endif /* toolManager_h */