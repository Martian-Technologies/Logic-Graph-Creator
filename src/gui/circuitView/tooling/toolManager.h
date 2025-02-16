#ifndef toolManager_h
#define toolManager_h

#include "backend/evaluator/evaluatorStateInterface.h"
#include "toolManagerEventRegister.h"
#include "../events/eventRegister.h"
#include "toolManagerInterface.h"
#include "../renderer/renderer.h"
#include "circuitTool.h"


class ToolManager {
public:
	inline ToolManager(EventRegister* eventRegister, Renderer* renderer) :
		eventRegister(eventRegister), renderer(renderer), toolManagerEventRegister(eventRegister, &registeredEvents), toolManagerInterface(this) { }

	inline ~ToolManager() { unregisterEvents(); }

	void reset();
	void pushTool(SharedCircuitTool newTool);
	void popTool();
	void clearTools();

	SharedCircuitTool getCurrentNonHelperTool() const;
	SharedCircuitTool getCurrentTool() const;

	void setCircuit(Circuit* circuit);
	void setEvaluatorStateInterface(EvaluatorStateInterface* evaluatorStateInterface);
	

private:
	void unregisterEvents();

	// current block container
	Circuit* circuit;

	// tool function event linking
	ToolManagerEventRegister toolManagerEventRegister;
	ToolManagerInterface toolManagerInterface;
	EventRegister* eventRegister;
	std::vector<std::pair<std::string, EventRegistrationSignature>> registeredEvents;

	Renderer* renderer;
	EvaluatorStateInterface* evaluatorStateInterface;

	// which tool data
	std::vector<SharedCircuitTool> toolStack;
};

#endif /* toolManager_h */
