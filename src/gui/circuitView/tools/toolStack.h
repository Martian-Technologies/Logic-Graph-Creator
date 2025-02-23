#ifndef toolStack_h
#define toolStack_h

#include "backend/evaluator/evaluatorStateInterface.h"
#include "../events/eventRegister.h"
#include "toolStackInterface.h"
#include "../renderer/renderer.h"
#include "circuitTool.h"


class ToolStack {
public:
	inline ToolStack(EventRegister* eventRegister, Renderer* renderer) : eventRegister(eventRegister), renderer(renderer), toolStackInterface(this) { }

	void reset();
	void pushTool(SharedCircuitTool newTool);
	void popTool();
	void clearTools();

	SharedCircuitTool getCurrentNonHelperTool() const;
	SharedCircuitTool getCurrentTool() const;

	void setMode(std::string mode);

	void setCircuit(Circuit* circuit);
	void setEvaluatorStateInterface(EvaluatorStateInterface* evaluatorStateInterface);

private:
	SharedCircuitTool getCurrentNonHelperTool();
	SharedCircuitTool getCurrentTool();


	// current block container
	Circuit* circuit;

	// tool function event linking
	ToolStackInterface toolStackInterface;
	EventRegister* eventRegister;

	Renderer* renderer;
	EvaluatorStateInterface* evaluatorStateInterface;

	std::vector<SharedCircuitTool> toolStack;
};

#endif /* toolStack_h */
