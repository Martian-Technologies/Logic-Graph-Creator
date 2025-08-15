#include "circuitView.h"

#include "backend/backend.h"
#include "events/customEvents.h"

CircuitView::CircuitView(CircuitViewRenderer* renderer) : renderer(renderer), toolManager(&eventRegister, renderer, this) {
	renderer->updateView(&viewManager);
	viewManager.setUpEvents(eventRegister);
	viewManager.connectViewChanged(std::bind(&CircuitView::viewChanged, this));
}

void CircuitView::setBackend(Backend* backend) {
	if (backend == nullptr) {
		this->backend = nullptr;
		this->evaluator = nullptr;
		this->circuit = nullptr;
		renderer->setEvaluator(nullptr);
		renderer->setCircuit(nullptr);
		evaluatorStateInterface = EvaluatorStateInterface();
		toolManager.setCircuit(nullptr);
	} else if (this->backend != backend) {
		this->backend = backend;
		this->evaluator = nullptr;
		this->circuit = nullptr;
		renderer->setEvaluator(nullptr);
		renderer->setCircuit(nullptr);
		evaluatorStateInterface = EvaluatorStateInterface();
		toolManager.setCircuit(nullptr);
	}
}

void CircuitView::setEvaluator(Backend* backend, evaluator_id_t evaluatorId, const Address& address) {
	if (backend == nullptr) {
		logError("When setting CircuitView's evaluator the backend was null. Failed to connect! Doing nothing!", "CircuitView");
	} else if (evaluatorId == 0) {
		if (this->backend != backend) {
			this->backend = backend;
			this->evaluator = nullptr;
			this->circuit = nullptr;
			renderer->setEvaluator(nullptr);
			renderer->setCircuit(nullptr);
			evaluatorStateInterface = EvaluatorStateInterface();
			toolManager.setCircuit(nullptr);
		}
	} else {
		SharedEvaluator evaluator = backend->getEvaluatorManager().getEvaluator(evaluatorId);
		if (evaluator == nullptr) {
			logError("When setting CircuitView's evaluator the wrong backend or evaluator id was passed. Failed to connect! Doing nothing!", "CircuitView");
		} else {
			if (this->backend != backend) {
				this->backend = backend;
			}
			this->evaluator = evaluator;
			circuit_id_t circuitId = evaluator->getCircuitId(address);
			SharedCircuit circuit = backend->getCircuit(circuitId); // ok if null
			this->circuit = circuit;
			
			renderer->setEvaluator(evaluator); // should be one func: setEvaluator(evaluator, address)
			renderer->setAddress(address);
			renderer->setCircuit(circuit.get());

			evaluatorStateInterface = EvaluatorStateInterface(evaluator.get());
			toolManager.setCircuit(circuit.get());
		}
	}
}

void CircuitView::setEvaluator(Backend* backend, std::shared_ptr<Evaluator> evaluator, const Address& address) {
	if (backend == nullptr) {
		logError("When setting CircuitView's evaluator the backend was null. Failed to connect! Doing nothing!", "CircuitView");
	} else if (evaluator == nullptr) {
		if (this->backend != backend) {
			this->backend = backend;
			this->evaluator = nullptr;
			this->circuit = nullptr;
			renderer->setEvaluator(nullptr);
			renderer->setCircuit(nullptr);
			evaluatorStateInterface = EvaluatorStateInterface();
			toolManager.setCircuit(nullptr);
		}
	} else if (backend->getEvaluatorManager().getEvaluator(evaluator->getEvaluatorId()) != evaluator) {
		logError("When setting CircuitView's evaluator the wrong backend was passed. Failed to connect! Doing nothing!", "CircuitView");
	} else {
		if (this->backend != backend) {
			this->backend = backend;
		}
		this->evaluator = evaluator;

		circuit_id_t circuitId = evaluator->getCircuitId(address);
		SharedCircuit circuit = backend->getCircuit(circuitId); // ok if null
		this->circuit = circuit;

		renderer->setEvaluator(evaluator); // should be one func: setEvaluator(evaluator, address)
		renderer->setAddress(address);
		renderer->setCircuit(circuit.get());
		
		evaluatorStateInterface = EvaluatorStateInterface(evaluator.get());
		toolManager.setCircuit(circuit.get());
	}
}

void CircuitView::setCircuit(Backend* backend, circuit_id_t circuitId) {
	if (backend == nullptr) {
		logError("When setting CircuitView's circuit the backend was null. Failed to connect! Doing nothing!", "CircuitView");
	} else if (circuitId == 0) {
		if (this->backend != backend) {
			this->backend = backend;
			this->evaluator = nullptr;
			this->circuit = nullptr;
			renderer->setEvaluator(nullptr);
			renderer->setCircuit(nullptr);
			evaluatorStateInterface = EvaluatorStateInterface();
			toolManager.setCircuit(nullptr);
		}
	} else {
		SharedCircuit circuit = backend->getCircuit(circuitId);
		if (circuit == nullptr) {
			logError("When setting CircuitView's circuit the wrong backend or circuit id was passed. Failed to connect! Doing nothing!", "CircuitView");
		} else {
			if (this->backend != backend) {
				this->backend = backend;
			}
			this->evaluator = nullptr;
			this->circuit = circuit;
			renderer->setEvaluator(nullptr);
			renderer->setCircuit(circuit.get());
			evaluatorStateInterface = EvaluatorStateInterface();
			toolManager.setCircuit(circuit.get());
		}
	}
}

void CircuitView::setCircuit(Backend* backend, SharedCircuit circuit) {
	if (backend == nullptr) {
		logError("When setting CircuitView's circuit the backend was null. Failed to connect! Doing nothing!", "CircuitView");
	} else if (circuit == nullptr) {
		if (this->backend != backend) {
			this->backend = backend;
			this->evaluator = nullptr;
			this->circuit = nullptr;
			renderer->setEvaluator(nullptr);
			renderer->setCircuit(nullptr);
			evaluatorStateInterface = EvaluatorStateInterface();
			toolManager.setCircuit(nullptr);
		}
	} else if (backend->getCircuit(circuit->getCircuitId()) != circuit) {
		logError("When setting CircuitView's circuit the wrong backend was passed. Failed to connect! Doing nothing!", "CircuitView");
	} else {
		if (this->backend != backend) {
			this->backend = backend;
		}
		this->evaluator = nullptr;
		this->circuit = circuit;
		renderer->setEvaluator(nullptr);
		renderer->setCircuit(circuit.get());
		evaluatorStateInterface = EvaluatorStateInterface();
		toolManager.setCircuit(circuit.get());
	}
}

// bool Backend::linkCircuitView(CircuitView* circuitView) {
// 	if (circuitView->getBackend() != this) {
// 		circuitViews.emplace(circuitView);
// 		circuitView->setEvaluator(nullptr);
// 		circuitView->setCircuit(nullptr);
// 		circuitView->setBackend(this);
// 	}
// 	return true;
// }

// bool Backend::unlinkCircuitView(CircuitView* circuitView) {
// 	if (circuitView->getBackend() == this) {
// 		circuitViews.erase(circuitView);
// 		circuitView->setEvaluator(nullptr);
// 		circuitView->setCircuit(nullptr);
// 		circuitView->setBackend(nullptr);
// 	}
// 	return true;
// }

// bool Backend::linkCircuitViewWithCircuit(CircuitView* circuitView, circuit_id_t circuitId) {
// 	SharedCircuit circuit = circuitManager.getCircuit(circuitId);
// 	if (circuit) {
// 		if (circuitView->getCircuit() == circuit.get()) return true;
// 		linkCircuitView(circuitView);
// 		circuitView->setEvaluator(nullptr);
// 		circuitView->setCircuit(circuit);
// 		return true;
// 	} else {
// 		linkCircuitView(circuitView);
// 		circuitView->setEvaluator(nullptr);
// 		circuitView->setCircuit(nullptr);
// 		return true;
// 	}
// 	return false;
// }

// bool Backend::linkCircuitViewWithEvaluator(CircuitView* circuitView, evaluator_id_t evalId, const Address& address) {
// 	// if (!circuitView->getCircuit()) return false;

// 	SharedEvaluator evaluator = evaluatorManager.getEvaluator(evalId);
// 	if (evaluator) {
// 		circuit_id_t circuitId = evaluator->getCircuitId(address);
// 		linkCircuitViewWithCircuit(circuitView, circuitId);
// 		circuitView->setEvaluator(evaluator);
// 		circuitView->setAddress(address);
// 		return true;
// 	}
// 	return false;
// }

void CircuitView::viewChanged() {
	eventRegister.doEvent(PositionEvent("Pointer Move", viewManager.getPointerPosition()));
	renderer->updateView(&viewManager);
}
