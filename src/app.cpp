#include "app.h"

Rml::EventId pinchEventId = Rml::EventId::Invalid;

Rml::EventId getPinchEventId() {
	if (pinchEventId == Rml::EventId::Invalid) {
		logError("Dont call getPinchEventId before initializing the App");
		return Rml::EventId::Scroll;
	}
	return pinchEventId;
}

App::App() : rml(&rmlSystemInterface, &rmlRenderInterface), backend(&circuitFileManager), circuitFileManager(&(backend.getCircuitManager())), fileListener(std::chrono::milliseconds(200)) {
	pinchEventId = Rml::RegisterEventType("pinch", true, true, Rml::DefaultActionPhase::None);
	windows.push_back(std::make_unique<MainWindow>(&backend, &circuitFileManager, &fileListener, rmlRenderInterface, &vulkan));
}

void App::runLoop() {
	bool firstPass = true;
	running = true;
	while (running) {
		// Wait for the next event (so we don't overload the cpu)
		SDL_WaitEvent(nullptr);
		
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
					if ((*itr)->recieveEvent(event)) {
						windows.erase(itr);
						break;
					}
					++itr;
				}
				break;
			}
			case SDL_EVENT_WINDOW_FOCUS_GAINED: {
				// Window focus switched, check which window gained focus
				for (auto& window : windows) {
					if (window->recieveEvent(event)) {
						// tell system interface about focus change
						rmlSystemInterface.SetWindow(window->getSdlWindow());
						break;
					}
				}
				break;
			}
			default: {
				// Send event to all windows
				for (auto& window : windows) {
					window->recieveEvent(event);
				}
			}
			}
		}

		// tell all windows to update rml
		for (auto& window : windows) {
			window->updateRml(rmlRenderInterface);
		}
		if (firstPass) {
			firstPass = false;
			for (auto& window : windows) {
				window->getCircuitViewWidget()->handleResize();
			}
		}
	}
}
