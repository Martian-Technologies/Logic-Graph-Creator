#ifndef circuitTool_h
#define circuitTool_h

#include <QPainter>

#include "backend/evaluator/evaluatorStateInterface.h"
#include "backend/circuit/circuit.h"
#include "../renderer/elementCreator.h"
#include "toolManagerEventRegister.h"
#include "../events/customEvents.h"
class LogicGridWindow;


class CircuitTool {
public:
	virtual ~CircuitTool() { }

	// This will also tell the tool to reset.
	inline void setup(ElementCreator elementCreator, EvaluatorStateInterface* evaluatorStateInterface, Circuit* circuit) {
		setElementCreator(elementCreator);
		setEvaluatorStateInterface(evaluatorStateInterface);
		setCircuit(circuit);
	}
	inline void setElementCreator(ElementCreator elementCreator) { this->elementCreator = elementCreator; }
	inline void setEvaluatorStateInterface(EvaluatorStateInterface* evaluatorStateInterface) { this->evaluatorStateInterface = evaluatorStateInterface; }
	inline void setCircuit(Circuit* circuit) { this->circuit = circuit; reset(); }
	virtual void initialize(ToolManagerEventRegister& toolManagerEventRegister) { }

	virtual void reset() { };

protected:
	Circuit* circuit = nullptr;
	EvaluatorStateInterface* evaluatorStateInterface = nullptr;
	ElementCreator elementCreator;
};

#endif /* circuitTool_h */
