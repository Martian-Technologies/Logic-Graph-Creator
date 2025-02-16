#ifndef circuitTool_h
#define circuitTool_h

#include "backend/evaluator/evaluatorStateInterface.h"
#include "../renderer/elementCreator.h"
#include "backend/circuit/circuit.h"
#include "../events/customEvents.h"

class ToolManager;
class ToolManagerInterface;

class CircuitTool {
	friend class ToolManager;
public:
	virtual ~CircuitTool() { unregisterFunctions(); }
	bool isHelper() { return helper; }
	virtual std::vector<std::string> getModes() { return {}; }

protected:
	void registerFunction(std::string eventName, EventFunction function);
	void unregisterFunction(std::string eventName);
	void unregisterFunctions();

	virtual void reset() { }
	virtual void activate() { }
	virtual void deactivate() { unregisterFunctions(); }

	virtual void setMode(std::string toolMode) { }

	Circuit* circuit = nullptr;
	bool helper = false;

	ElementCreator elementCreator;
	ToolManagerInterface* toolManagerInterface;
	EvaluatorStateInterface* evaluatorStateInterface = nullptr;

private:
	// This will also tell the tool to reset.
	void setup(ElementCreator elementCreator, EventRegister* eventRegister, ToolManagerInterface* toolManagerInterface, EvaluatorStateInterface* evaluatorStateInterface, Circuit* circuit);
	void unsetup();
	inline void setEvaluatorStateInterface(EvaluatorStateInterface* evaluatorStateInterface) { this->evaluatorStateInterface = evaluatorStateInterface; }
	inline void setCircuit(Circuit* circuit) { this->circuit = circuit; reset(); }

	EventRegister* eventRegister;
	std::vector<std::pair<std::string, EventRegistrationSignature>> registeredEvents;
};

typedef std::shared_ptr<CircuitTool> SharedCircuitTool;

#endif /* circuitTool_h */
