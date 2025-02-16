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

	inline ~ToolManager() {
		unregisterEvents();
	}

    inline SharedCircuitTool getCurrentTool() const { if (!toolStack.empty()) return toolStack.back(); return nullptr; }

	inline void pushTool(SharedCircuitTool newTool) {
		toolStack.push_back(newTool);
		toolStack.back()->setup(ElementCreator(renderer), &toolManagerInterface, evaluatorStateInterface, circuit);
		toolStack.back()->initialize(toolManagerEventRegister);
	}

	inline void popTool() {
		unregisterEvents();
		toolStack.pop_back();
		if (!toolStack.empty()) {
			toolStack.back()->setEvaluatorStateInterface(evaluatorStateInterface);
			toolStack.back()->initialize(toolManagerEventRegister);
		}
	}

	inline void clearTools() {
		unregisterEvents();
		toolStack.clear();
	}

	inline void setCircuit(Circuit* circuit) {
		this->circuit = circuit;
		toolStack.clear();
	}

	inline void setEvaluatorStateInterface(EvaluatorStateInterface* evaluatorStateInterface) {
		this->evaluatorStateInterface = evaluatorStateInterface;
		if (!toolStack.empty()) toolStack.back()->setEvaluatorStateInterface(evaluatorStateInterface);
	}

	inline void reset() { if (toolStack.size()) toolStack.back()->reset(); }

private:
	void unregisterEvents() {
		for (auto eventFuncPair : registeredEvents) {
			eventRegister->unregisterFunction(eventFuncPair.first, eventFuncPair.second);
		}
		registeredEvents.clear();
	}

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
