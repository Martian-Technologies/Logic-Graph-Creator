#include "app.h"

App::App() : rml(&rmlSystemInterface, &rmlRenderInterface), circuitFileManager(&(backend.getCircuitManager())) {
	pinchEventId = Rml::RegisterEventType("pinch", true, true, Rml::DefaultActionPhase::None);
	windows.emplace_back(&backend, &circuitFileManager, pinchEventId);
}

void App::runLoop() {
	running = true;
	while (running) {
		// process events (TODO - should probably just have a map of window ids to windows)
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_EVENT_QUIT: {
				// Main application quit (eg. ctrl-c in terminal)
				running = false;
				break;
			}
			case SDL_EVENT_WINDOW_CLOSE_REQUESTED: {
				// Single window was closed, check which window was closed and remove it
				auto itr = windows.begin();
				while (itr != windows.end()) {
					if (itr->recieveEvent(event)) {
						windows.erase(itr);
						break;
					}
					++itr;
				}
				break;
			}
			case SDL_EVENT_WINDOW_FOCUS_GAINED: {
				// Window focus switched, check which window gained focus
				for (Window& window : windows) {
					if (window.recieveEvent(event)) {
						// tell system interface about focus change
						rmlSystemInterface.SetWindow(window.getSdlWindow());
						break;
					}
				}
				break;
			}
			case SDL_EVENT_DROP_FILE: {
				std::string file = event.drop.data;
				std::cout << file << "\n";
				for (Window& window : windows) {
					circuit_id_t id = window.getCircuitViewWidget()->getFileManager()->loadFromFile(file);
					if (id == 0) {
						logError("Error", "Failed to load circuit file.");
						return;
					}
					window.getCircuitViewWidget()->getCircuitView()->getBackend()->linkCircuitViewWithCircuit(window.getCircuitViewWidget()->getCircuitView(), id);
					auto evaluatorId = window.getCircuitViewWidget()->getCircuitView()->getBackend()->createEvaluator(id);
					window.getCircuitViewWidget()->getCircuitView()->getBackend()->linkCircuitViewWithEvaluator(window.getCircuitViewWidget()->getCircuitView(), evaluatorId.value(), Address());
					window.getCircuitViewWidget()->setSimState(true);
					window.getCircuitViewWidget()->simUseSpeed(true);
					window.getCircuitViewWidget()->setSimSpeed(20);
				}
			}
			default: {
				// Send event to all windows
				for (Window& window : windows) {
					window.recieveEvent(event);
				}
			}
			}
		}

		// tell all windows to render
		for (Window& window : windows) {
			window.update();
			window.render(rmlRenderInterface);
		}
	}
}
