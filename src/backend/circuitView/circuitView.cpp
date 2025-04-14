#include "circuitView.h"

CircuitView::CircuitView(Renderer* renderer) : renderer(renderer), toolManager(&eventRegister, renderer, this) {
	renderer->updateView(&viewManager);
	viewManager.setUpEvents(eventRegister);
	viewManager.connectViewChanged(std::bind(&CircuitView::viewChanged, this));
}

void CircuitView::setBackend(Backend* backend) {
	this->backend = backend;
}

void CircuitView::setEvaluator(std::shared_ptr<Evaluator> evaluator) {
	renderer->setEvaluator(evaluator.get());
	evaluatorStateInterface = EvaluatorStateInterface(evaluator.get());
	toolManager.setEvaluatorStateInterface(&evaluatorStateInterface);
	this->evaluator = evaluator;
}

void CircuitView::setCircuit(SharedCircuit circuit) {
	if (this->circuit) this->circuit->disconnectListener(this);

	this->circuit = circuit;
	toolManager.setCircuit(circuit.get());
	renderer->setCircuit(circuit.get());
	if (circuit) {
		circuit->connectListener(this, std::bind(&CircuitView::circuitChanged, this, std::placeholders::_1, std::placeholders::_2));
	}
}

void CircuitView::viewChanged() {
	eventRegister.doEvent(PositionEvent("Pointer Move", viewManager.getPointerPosition()));
	renderer->updateView(&viewManager);
}

void CircuitView::circuitChanged(DifferenceSharedPtr difference, circuit_id_t circuitId) {
	renderer->updateCircuit(difference);
}
