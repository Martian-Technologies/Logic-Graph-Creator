#include "circuitView.h"
#include "backend/backend.h"

CircuitView::CircuitView(CircuitViewRenderer* renderer) : renderer(renderer), toolManager(&eventRegister, renderer, this) {
	renderer->updateView(&viewManager);
	viewManager.setUpEvents(eventRegister);
	viewManager.connectViewChanged(std::bind(&CircuitView::viewChanged, this));
}

void CircuitView::setBackend(Backend* backend) {
	if (this->backend) {
		Backend* oldBackend = this->backend;
		this->backend = nullptr;
		oldBackend->unlinkCircuitView(this);
	}
	this->backend = backend;
	dataUpdateEventManager = backend->getDataUpdateEventManager();
}

void CircuitView::setEvaluator(std::shared_ptr<Evaluator> evaluator) {
	renderer->setEvaluator(evaluator);
	evaluatorStateInterface = EvaluatorStateInterface(evaluator.get());
	toolManager.setEvaluatorStateInterface(&evaluatorStateInterface);
	this->evaluator = evaluator;

	if (dataUpdateEventManager) dataUpdateEventManager->sendEvent("circuitViewChangeEvaluator");
}

void CircuitView::setCircuit(SharedCircuit circuit) {
	if (this->circuit) this->circuit->disconnectListener(this);
	if (circuit) {
		this->circuit = circuit;
		toolManager.setCircuit(circuit.get());
		renderer->setCircuit(circuit.get());
	} else {
		this->circuit = circuit;
		toolManager.setCircuit(nullptr);
		renderer->setCircuit(nullptr);
	}
	if (dataUpdateEventManager) dataUpdateEventManager->sendEvent("circuitViewChangeCircuit");
}

void CircuitView::setAddress(const Address& address) {
	this->address = address;
	renderer->setAddress(address);
}

void CircuitView::viewChanged() {
	eventRegister.doEvent(PositionEvent("Pointer Move", viewManager.getPointerPosition()));
	renderer->updateView(&viewManager);
}
