#ifndef circuitTool_h
#define circuitTool_h

#include "backend/evaluator/evaluatorStateInterface.h"
#include "../renderer/elementCreator.h"
#include "toolManagerEventRegister.h"
#include "backend/circuit/circuit.h"
#include "../events/customEvents.h"

class ToolManager;
class ToolManagerInterface;

class CircuitTool {
	friend class ToolManager;
public:
	virtual ~CircuitTool() { }
	bool isHelper() { return helper; }
	virtual std::vector<std::string> getModes() { return {}; }
	
protected:
	virtual void reset() { };
	virtual void activate(ToolManagerEventRegister& toolManagerEventRegister) { }
	virtual void deactivate(ToolManagerEventRegister& toolManagerEventRegister) { }
	
	virtual void setMode(std::string toolMode) { }

	ToolManagerInterface* toolManagerInterface;
	ElementCreator elementCreator;

	EvaluatorStateInterface* evaluatorStateInterface = nullptr;
	Circuit* circuit = nullptr;
	bool helper = false;

private:
	// This will also tell the tool to reset.
	void setup(ElementCreator elementCreator, ToolManagerInterface* toolManagerInterface, EvaluatorStateInterface* evaluatorStateInterface, Circuit* circuit);
	inline void setEvaluatorStateInterface(EvaluatorStateInterface* evaluatorStateInterface) { this->evaluatorStateInterface = evaluatorStateInterface; }
	inline void setCircuit(Circuit* circuit) { this->circuit = circuit; reset(); }
};

typedef std::shared_ptr<CircuitTool> SharedCircuitTool;

#endif /* circuitTool_h */
