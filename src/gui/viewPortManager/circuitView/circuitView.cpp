#include "circuitView.h"

#include "backend/backend.h"
#include "events/customEvents.h"

#include "gpu/mainRenderer.h"

CircuitView::CircuitView(ViewportID viewportID) : viewportID(viewportID), toolManager(&eventRegister, viewportID, this) {
	MainRenderer::get().moveViewportView(viewportID, viewManager.getViewCenter(), viewManager.getViewScale());
	viewManager.setUpEvents(eventRegister);
	viewManager.connectViewChanged(std::bind(&CircuitView::viewChanged, this));
}

void CircuitView::setBackend(Backend* backend) {
	if (backend == nullptr) {
		this->backend = nullptr;
		dataUpdateEventManager = nullptr;
		this->evaluator = nullptr;
		this->circuit = nullptr;
		MainRenderer::get().setViewportEvaluator(viewportID, nullptr, nullptr, Address());
		toolManager.setCircuit(nullptr);
	} else if (this->backend != backend) {
		this->backend = backend;
		dataUpdateEventManager = backend->getDataUpdateEventManager();
		this->evaluator = nullptr;
		this->circuit = nullptr;
		MainRenderer::get().setViewportEvaluator(viewportID, nullptr, nullptr, Address());
		toolManager.setCircuit(nullptr);
	}
	if (dataUpdateEventManager) {
		dataUpdateEventManager->sendEvent("circuitViewChangeEvaluator");
		dataUpdateEventManager->sendEvent("circuitViewChangeCircuit");
	}
}

void CircuitView::setEvaluator(Backend* backend, evaluator_id_t evaluatorId, const Address& address) {
	if (backend == nullptr) {
		logError("When setting CircuitView's evaluator the backend was null. Failed to connect! Doing nothing!", "CircuitView");
	} else if (evaluatorId == 0) {
		if (this->backend != backend) {
			this->backend = backend;
			dataUpdateEventManager = backend->getDataUpdateEventManager();
			this->evaluator = nullptr;
			this->circuit = nullptr;
			MainRenderer::get().setViewportEvaluator(viewportID, nullptr, nullptr, Address());
			toolManager.setCircuit(nullptr);
			if (dataUpdateEventManager) {
				dataUpdateEventManager->sendEvent("circuitViewChangeEvaluator");
				dataUpdateEventManager->sendEvent("circuitViewChangeCircuit");
			}
		}
	} else {
		SharedEvaluator evaluator = backend->getEvaluatorManager().getEvaluator(evaluatorId);
		if (evaluator == nullptr) {
			logError("When setting CircuitView's evaluator the wrong backend or evaluator id was passed. Failed to connect! Doing nothing!", "CircuitView");
		} else {
			if (this->backend != backend) {
				this->backend = backend;
				dataUpdateEventManager = backend->getDataUpdateEventManager();
			}
			this->evaluator = evaluator;
			this->address = address;
			
			circuit_id_t circuitId = evaluator->getCircuitId(address);
			SharedCircuit circuit = backend->getCircuit(circuitId); // ok if null
			this->circuit = circuit;
			
			MainRenderer::get().setViewportEvaluator(viewportID, circuit.get(), evaluator.get(), address);

			toolManager.setCircuit(circuit.get());

			if (dataUpdateEventManager) {
				dataUpdateEventManager->sendEvent("circuitViewChangeEvaluator");
				dataUpdateEventManager->sendEvent("circuitViewChangeCircuit");
			}
		}
	}
}

void CircuitView::setEvaluator(Backend* backend, std::shared_ptr<Evaluator> evaluator, const Address& address) {
	if (backend == nullptr) {
		logError("When setting CircuitView's evaluator the backend was null. Failed to connect! Doing nothing!", "CircuitView");
	} else if (evaluator == nullptr) {
		if (this->backend != backend) {
			this->backend = backend;
			dataUpdateEventManager = backend->getDataUpdateEventManager();
			this->evaluator = nullptr;
			this->circuit = nullptr;
			MainRenderer::get().setViewportEvaluator(viewportID, nullptr, nullptr, Address());
			toolManager.setCircuit(nullptr);
			if (dataUpdateEventManager) {
				dataUpdateEventManager->sendEvent("circuitViewChangeEvaluator");
				dataUpdateEventManager->sendEvent("circuitViewChangeCircuit");
			}
		}
	} else if (backend->getEvaluatorManager().getEvaluator(evaluator->getEvaluatorId()) != evaluator) {
		logError("When setting CircuitView's evaluator the wrong backend was passed. Failed to connect! Doing nothing!", "CircuitView");
	} else {
		if (this->backend != backend) {
			this->backend = backend;
			dataUpdateEventManager = backend->getDataUpdateEventManager();
		}
		this->evaluator = evaluator;
		this->address = address;

		circuit_id_t circuitId = evaluator->getCircuitId(address);
		SharedCircuit circuit = backend->getCircuit(circuitId); // ok if null
		this->circuit = circuit;

		MainRenderer::get().setViewportEvaluator(viewportID, circuit.get(), evaluator.get(), address);
		
		toolManager.setCircuit(circuit.get());

		if (dataUpdateEventManager) {
			dataUpdateEventManager->sendEvent("circuitViewChangeEvaluator");
			dataUpdateEventManager->sendEvent("circuitViewChangeCircuit");
		}
	}
}

void CircuitView::setCircuit(Backend* backend, circuit_id_t circuitId) {
	if (backend == nullptr) {
		logError("When setting CircuitView's circuit the backend was null. Failed to connect! Doing nothing!", "CircuitView");
	} else if (circuitId == 0) {
		if (this->backend != backend) {
			this->backend = backend;
			dataUpdateEventManager = backend->getDataUpdateEventManager();
			this->evaluator = nullptr;
			this->circuit = nullptr;
			MainRenderer::get().setViewportEvaluator(viewportID, nullptr, nullptr, Address());
			toolManager.setCircuit(nullptr);
			if (dataUpdateEventManager) {
				dataUpdateEventManager->sendEvent("circuitViewChangeEvaluator");
				dataUpdateEventManager->sendEvent("circuitViewChangeCircuit");
			}
		}
	} else {
		SharedCircuit circuit = backend->getCircuit(circuitId);
		if (circuit == nullptr) {
			logError("When setting CircuitView's circuit the wrong backend or circuit id was passed. Failed to connect! Doing nothing!", "CircuitView");
		} else {
			if (this->backend != backend) {
				this->backend = backend;
				dataUpdateEventManager = backend->getDataUpdateEventManager();
			}
			this->evaluator = nullptr;
			this->circuit = circuit;
			MainRenderer::get().setViewportCircuit(viewportID, circuit.get());
			toolManager.setCircuit(circuit.get());

			if (dataUpdateEventManager) {
				dataUpdateEventManager->sendEvent("circuitViewChangeEvaluator");
				dataUpdateEventManager->sendEvent("circuitViewChangeCircuit");
			}
		}
	}
}

void CircuitView::setCircuit(Backend* backend, SharedCircuit circuit) {
	if (backend == nullptr) {
		logError("When setting CircuitView's circuit the backend was null. Failed to connect! Doing nothing!", "CircuitView");
	} else if (circuit == nullptr) {
		if (this->backend != backend) {
			this->backend = backend;
			dataUpdateEventManager = backend->getDataUpdateEventManager();
			this->evaluator = nullptr;
			this->circuit = nullptr;
			MainRenderer::get().setViewportEvaluator(viewportID, nullptr, nullptr, Address());
			toolManager.setCircuit(nullptr);

			if (dataUpdateEventManager) {
				dataUpdateEventManager->sendEvent("circuitViewChangeEvaluator");
				dataUpdateEventManager->sendEvent("circuitViewChangeCircuit");
			}
		}
	} else if (backend->getCircuit(circuit->getCircuitId()) != circuit) {
		logError("When setting CircuitView's circuit the wrong backend was passed. Failed to connect! Doing nothing!", "CircuitView");
	} else {
		if (this->backend != backend) {
			this->backend = backend;
			dataUpdateEventManager = backend->getDataUpdateEventManager();
		}
		this->evaluator = nullptr;
		this->circuit = circuit;
		MainRenderer::get().setViewportCircuit(viewportID, circuit.get());
		toolManager.setCircuit(circuit.get());

		if (dataUpdateEventManager) {
			dataUpdateEventManager->sendEvent("circuitViewChangeEvaluator");
			dataUpdateEventManager->sendEvent("circuitViewChangeCircuit");
		}
	}
}

void CircuitView::viewChanged() {
	eventRegister.doEvent(PositionEvent("Pointer Move", viewManager.getPointerPosition()));
	MainRenderer::get().moveViewportView(viewportID, viewManager.getViewCenter(), viewManager.getViewScale());
}
