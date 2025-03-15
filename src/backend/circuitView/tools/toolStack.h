#ifndef toolStack_h
#define toolStack_h

#include "backend/evaluator/evaluatorStateInterface.h"
#include "../events/eventRegister.h"
#include "toolStackInterface.h"
#include "../renderer/renderer.h"
#include "circuitTool.h"


class ToolStack {
public:
	inline ToolStack(EventRegister* eventRegister, Renderer* renderer) : eventRegister(eventRegister), renderer(renderer), toolStackInterface(this) {
		eventRegister->registerFunction("pointer enter view", std::bind(&ToolStack::enterBlockView, this, std::placeholders::_1));
		eventRegister->registerFunction("pointer exit view", std::bind(&ToolStack::exitBlockView, this, std::placeholders::_1));
		eventRegister->registerFunction("pointer move", std::bind(&ToolStack::pointerMove, this, std::placeholders::_1));
	}
	inline ToolStack(const ToolStack& other) = delete;
	inline ToolStack& operator=(const ToolStack& other) = delete;

	void activate();
	void deactivate() { if (!toolStack.empty()) { toolStack.back()->deactivate(); toolStack.back()->elementCreator.clear(); } }

	void reset();
	void pushTool(SharedCircuitTool newTool);
	void popTool();
	void clearTools();
	void popAbove(CircuitTool* toolNotToPop);

	SharedCircuitTool getCurrentNonHelperTool() const;
	SharedCircuitTool getCurrentTool() const;

	void setMode(std::string mode);

	void setCircuit(Circuit* circuit);
	void setEvaluatorStateInterface(EvaluatorStateInterface* evaluatorStateInterface);

private:
	SharedCircuitTool getCurrentNonHelperTool_();
	SharedCircuitTool getCurrentTool_();

	// mouse data for tool when setup
	bool enterBlockView(const Event* event);
	bool exitBlockView(const Event* event);
	bool pointerMove(const Event* event);

	bool pointerInView = false;
	FPosition lastPointerFPosition;
	Position lastPointerPosition;

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
