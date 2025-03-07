#ifndef circuitTool_h
#define circuitTool_h

#include "backend/evaluator/evaluatorStateInterface.h"
#include "../renderer/elementCreator.h"
#include "backend/circuit/circuit.h"
#include "../events/customEvents.h"
#include "toolStackInterface.h"

class ToolStack;

class CircuitTool {
	friend class ToolStack;
public:
	virtual ~CircuitTool() { unregisterFunctions(); }
	bool isHelper() { return helper; }
	static inline std::vector<std::string> getModes() { return {}; }

protected:
	void registerFunction(std::string eventName, EventFunction function);
	void unregisterFunction(std::string eventName);
	void unregisterFunctions();
	
	virtual void reset() { elementCreator.clear(); }
	virtual void activate();
	virtual void deactivate() { unregisterFunctions(); }
	
	virtual void setMode(std::string toolMode) { }
	
	virtual void updateElements() {}

	bool pointerInView = false;
	FPosition lastPointerFPosition;
	Position lastPointerPosition;

	Circuit* circuit = nullptr;
	bool helper = false;

	ElementCreator elementCreator;
	ToolStackInterface* toolStackInterface;
	EvaluatorStateInterface* evaluatorStateInterface = nullptr;

private:
	// This will also tell the tool to reset.
	void setup(Renderer* elementCreator, EventRegister* eventRegister, ToolStackInterface* toolStackInterface, EvaluatorStateInterface* evaluatorStateInterface, Circuit* circuit);
	void unsetup();
	inline void setEvaluatorStateInterface(EvaluatorStateInterface* evaluatorStateInterface) { this->evaluatorStateInterface = evaluatorStateInterface; }
	inline void setCircuit(Circuit* circuit) { this->circuit = circuit; reset(); }

	bool enterBlockView(const Event* event);
	bool exitBlockView(const Event* event);
	bool pointerMove(const Event* event);

	EventRegister* eventRegister;
	std::vector<std::pair<std::string, EventRegistrationSignature>> registeredEvents;
};

typedef std::shared_ptr<CircuitTool> SharedCircuitTool;

#endif /* circuitTool_h */
