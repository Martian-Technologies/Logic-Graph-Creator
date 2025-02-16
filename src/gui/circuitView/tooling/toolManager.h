#ifndef toolManager_h
#define toolManager_h

#include "backend/evaluator/evaluatorStateInterface.h"
#include "../events/eventRegister.h"
#include "toolManagerInterface.h"
#include "../renderer/renderer.h"
#include "circuitTool.h"


class ToolManager {
public:
	inline ToolManager(EventRegister* eventRegister, Renderer* renderer) : eventRegister(eventRegister), renderer(renderer), toolManagerInterface(this) { }

	void reset();
	void pushTool(SharedCircuitTool newTool);
	void popTool();
	void clearTools();

	SharedCircuitTool getCurrentNonHelperTool() const;
	SharedCircuitTool getCurrentTool() const;

	void setCircuit(Circuit* circuit);
	void setEvaluatorStateInterface(EvaluatorStateInterface* evaluatorStateInterface);

private:
	// current block container
	Circuit* circuit;

	// tool function event linking
	ToolManagerInterface toolManagerInterface;
	EventRegister* eventRegister;

	Renderer* renderer;
	EvaluatorStateInterface* evaluatorStateInterface;

	std::vector<SharedCircuitTool> toolStack;
};

#endif /* toolManager_h */
