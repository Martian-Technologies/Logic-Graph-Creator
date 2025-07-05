#include "circuitViewWidget.h"

#include <SDL3/SDL.h>

#include "backend/circuitView/circuitView.h"
#include "computerAPI/directoryManager.h"
#include "backend/settings/settings.h"
#include "interaction/eventPasser.h"
#include "backend/backend.h"

void SaveCallback(void* userData, const char* const* filePaths, int filter) {
	CircuitViewWidget* circuitViewWidget = (CircuitViewWidget*)userData;
	if (filePaths && filePaths[0]) {
		std::string filePath = filePaths[0];
		if (!circuitViewWidget->getCircuitView()->getCircuit()) {
			logError("Circuit was null, could not save");
			return;
		}
		const std::string& UUID = circuitViewWidget->getCircuitView()->getCircuit()->getUUID();
		if (circuitViewWidget->getFileManager()->getSavePath(UUID) != nullptr)
			logWarning("This circuit " + circuitViewWidget->getCircuitView()->getCircuit()->getCircuitName() + " will be saved with a new UUID");
		circuitViewWidget->getFileManager()->saveToFile(filePath, UUID);
	} else {
		std::cout << "File dialog canceled." << std::endl;
	}
}

void LoadCallback(void* userData, const char* const* filePaths, int filter) {
	CircuitViewWidget* circuitViewWidget = (CircuitViewWidget*)userData;
	if (filePaths && filePaths[0]) {
		std::string filePath = filePaths[0];
		std::vector<circuit_id_t> ids = circuitViewWidget->getFileManager()->loadFromFile(filePath);
		if (ids.empty()) {
			// logError("Failed to load circuit file."); // Not a error! It is valid to load 0 circuits.
			return;
		}
		circuit_id_t id = ids.back();
		circuitViewWidget->getCircuitView()->getBackend()->linkCircuitViewWithCircuit(circuitViewWidget->getCircuitView(), id);
		for (auto& iter : circuitViewWidget->getCircuitView()->getBackend()->getEvaluatorManager().getEvaluators()) {
			if (iter.second->getCircuitId(Address()) == id) {
				circuitViewWidget->getCircuitView()->getBackend()->linkCircuitViewWithEvaluator(circuitViewWidget->getCircuitView(), iter.first, Address());
				return;
			}
		}
	} else {
		logInfo("File dialog canceled.");
	}
}

CircuitViewWidget::CircuitViewWidget(CircuitFileManager* fileManager, Rml::ElementDocument* document, Rml::Element* parent, SDL_Window* window, SDL_Renderer* sdlRenderer) : fileManager(fileManager), document(document), window(window), parent(parent) {
	// create circuitView
	renderer = std::make_unique<SdlRenderer>(sdlRenderer);
	circuitView = std::make_unique<CircuitView>(renderer.get());
	circuitView->getEventRegister().registerFunction("status bar changed", [this](const Event* event) -> bool {
		auto eventData = event->cast2<std::string>();
		if (eventData) setStatusBar(eventData->get());
		return false;
	});

	int w = parent->GetClientWidth();
	int h = parent->GetClientHeight();
	int x = parent->GetAbsoluteLeft() + parent->GetClientLeft();
	int y = parent->GetAbsoluteTop() + parent->GetClientTop();

	circuitView->getViewManager().setAspectRatio((float)w / (float)h);
	renderer->resize(w, h);
	renderer->reposition(x, y);
	renderer->initializeTileSet((DirectoryManager::getResourceDirectory() / "logicTiles.png").string());

	// create keybind shortcuts and connect them
	document->AddEventListener(Rml::EventId::Keydown, &keybindHandler);
	keybindHandler.addListener(
		"Keybinds/Editing/Undo",
		[this]() { if (circuitView->getCircuit()) circuitView->getCircuit()->undo(); }
	);
	keybindHandler.addListener(
		"Keybinds/Editing/Redo",
		[this]() { if (circuitView->getCircuit()) circuitView->getCircuit()->redo(); }
	);
	keybindHandler.addListener(
		"Keybinds/File/Save",
		[this]() { save(); }
	);
	keybindHandler.addListener(
		"Keybinds/File/Save As",
		[this]() { asSave(); }
	);
	keybindHandler.addListener(
		"Keybinds/File/Open",
		[this]() { load(); }
	);
	keybindHandler.addListener(
		"Keybinds/Editing/Copy",
		[this]() { circuitView->getEventRegister().doEvent(Event("Copy")); }
	);
	keybindHandler.addListener(
		"Keybinds/Editing/Paste",
		[this]() { if (circuitView->getBackend()) circuitView->getBackend()->getToolManagerManager().setTool("paste tool"); }
	);
	keybindHandler.addListener(
		"Keybinds/Editing/Tools/State Changer",
		[this]() { if (circuitView->getBackend()) circuitView->getBackend()->getToolManagerManager().setTool("state changer"); }
	);
	keybindHandler.addListener(
		"Keybinds/Editing/Tools/Connection",
		[this]() { if (circuitView->getBackend()) circuitView->getBackend()->getToolManagerManager().setTool("connection"); }
	);
	keybindHandler.addListener(
		"Keybinds/Editing/Tools/Move",
		[this]() { if (circuitView->getBackend()) circuitView->getBackend()->getToolManagerManager().setTool("move"); }
	);
	keybindHandler.addListener(
		"Keybinds/Editing/Tools/Mode Changer",
		[this]() { if (circuitView->getBackend()) circuitView->getBackend()->getToolManagerManager().setTool("mode changer"); }
	);
	keybindHandler.addListener(
		"Keybinds/Editing/Tools/Placement",
		[this]() { if (circuitView->getBackend()) circuitView->getBackend()->getToolManagerManager().setTool("placement"); }
	);
	keybindHandler.addListener(
		"Keybinds/Editing/Tools/Selection Maker",
		[this]() { if (circuitView->getBackend()) circuitView->getBackend()->getToolManagerManager().setTool("selection maker"); }
	);
	keybindHandler.addListener(
		"Keybinds/Editing/Rotate CCW",
		[this]() { circuitView->getEventRegister().doEvent(Event("Tool Rotate Block CCW")); }
	);
	keybindHandler.addListener(
		"Keybinds/Editing/Rotate CW",
		[this]() { circuitView->getEventRegister().doEvent(Event("Tool Rotate Block CW")); }
	);
	keybindHandler.addListener(
		"Keybinds/Editing/Confirm",
		[this]() { circuitView->getEventRegister().doEvent(Event("Confirm")); }
	);
	keybindHandler.addListener(
		"Keybinds/Editing/Tool Invert Mode",
		[this]() { circuitView->getEventRegister().doEvent(Event("Tool Invert Mode")); }
	);
	keybindHandler.addListener(
		"Keybinds/File/New",
		[this]() { newCircuit(); }
	);

	document->AddEventListener(Rml::EventId::Resize, new EventPasser(
		[this](Rml::Event& event) {
			doResize = true;
		}
	));

	parent->AddEventListener(Rml::EventId::Mousedown, new EventPasser(
		[this](Rml::Event& event) {
			int button = event.GetParameter<int>("button", 0);
			if (button == 0) { // left
				if (event.GetParameter<int>("alt_key", 0)) {
					if (circuitView->getEventRegister().doEvent(PositionEvent("View Attach Anchor", circuitView->getViewManager().getPointerPosition()))) { event.StopPropagation(); return; }
				}
				if (circuitView->getEventRegister().doEvent(PositionEvent("Tool Primary Activate", circuitView->getViewManager().getPointerPosition()))) event.StopPropagation();
			} else if (button == 1) { // right
				if (circuitView->getEventRegister().doEvent(PositionEvent("Tool Secondary Activate", circuitView->getViewManager().getPointerPosition()))) event.StopPropagation();
			}
		}
	));

	Rml::Element* root = document->GetElementById("main-container");
	root->AddEventListener(Rml::EventId::Mouseup, new EventPasser(
		[this](Rml::Event& event) {
			int button = event.GetParameter<int>("button", 0);
			if (button == 0) { // left
				circuitView->getEventRegister().doEvent(PositionEvent("View Dettach Anchor", circuitView->getViewManager().getPointerPosition()));
				circuitView->getEventRegister().doEvent(PositionEvent("tool primary deactivate", circuitView->getViewManager().getPointerPosition()));
			} else if (button == 1) { // right
				circuitView->getEventRegister().doEvent(PositionEvent("tool secondary deactivate", circuitView->getViewManager().getPointerPosition()));
			}
		}
	));

	parent->AddEventListener(Rml::EventId::Mousemove, new EventPasser(
		[this](Rml::Event& event) {
			SDL_FPoint point(event.GetParameter<int>("mouse_x", 0), event.GetParameter<int>("mouse_y", 0));
			if (insideWindow(point)) { // inside the widget
				Vec2 viewPos = pixelsToView(point);
				circuitView->getEventRegister().doEvent(PositionEvent("Pointer Move", circuitView->getViewManager().viewToGrid(viewPos)));
			}
		}
	));

	parent->AddEventListener(Rml::EventId::Mouseover, new EventPasser(
		[this](Rml::Event& event) {
			// // grab focus so key inputs work without clicking
			// setFocus(Qt::MouseFocusReason);
			this->parent->Focus();
			SDL_FPoint point(event.GetParameter<int>("mouse_x", 0), event.GetParameter<int>("mouse_y", 0));
			Vec2 viewPos = pixelsToView(point);
			if (viewPos.x < 0 || viewPos.y < 0 || viewPos.x > 1 || viewPos.y > 1) return;
			circuitView->getEventRegister().doEvent(PositionEvent("pointer enter view", circuitView->getViewManager().viewToGrid(viewPos)));
		}
	));

	parent->AddEventListener(Rml::EventId::Mouseout, new EventPasser(
		[this](Rml::Event& event) {
			SDL_FPoint point(event.GetParameter<int>("mouse_x", 0), event.GetParameter<int>("mouse_y", 0));
			Vec2 viewPos = pixelsToView(point);
			if (viewPos.x >= 0 && viewPos.y >= 0 && viewPos.x <= 1 && viewPos.y <= 1) return;
			circuitView->getEventRegister().doEvent(PositionEvent("pointer exit view", circuitView->getViewManager().viewToGrid(viewPos)));
		}
	));

	parent->AddEventListener(Rml::EventId::Mousescroll, new EventPasser(
		[this](Rml::Event& event) {
			SDL_FPoint delta(event.GetParameter<float>("wheel_delta_x", 0) * 12, event.GetParameter<float>("wheel_delta_y", 0) * -12);
			// logInfo("{}, {}", "", delta.x, delta.y);
			if (mouseControls) {
				if (circuitView->getEventRegister().doEvent(DeltaEvent("view zoom", (float)(delta.y) / 150.f))) event.StopPropagation();
			} else {
				if (event.GetParameter<int>("shift_key", 0)) {
					// do zoom
					if (circuitView->getEventRegister().doEvent(DeltaEvent("view zoom", (float)(delta.y) / 150.f))) event.StopPropagation();
				} else {
					if (circuitView->getEventRegister().doEvent(DeltaXYEvent(
						"view pan",
						delta.x / getPixelsWidth() * circuitView->getViewManager().getViewWidth(),
						delta.y / getPixelsHeight() * circuitView->getViewManager().getViewHeight()
					))) event.StopPropagation();
				}
			}
		}
	));
	parent->AddEventListener("pinch", new EventPasser(
		[this](Rml::Event& event) {
			float delta = event.GetParameter<float>("delta", 0);
			logInfo(delta);
			if (circuitView->getEventRegister().doEvent(DeltaEvent("view zoom", (float)(delta) * 50))) event.StopPropagation();
			// if (mouseControls) {
			// } else {
			// 	if (event.GetParameter<int>("shift_key", 0)) {
			// 		// do zoom
			// 		if (circuitView->getEventRegister().doEvent(DeltaEvent("view zoom", (float)(delta.y) / 100.f))) event.StopPropagation();
			// 	} else {
			// 		if (circuitView->getEventRegister().doEvent(DeltaXYEvent(
			// 			"view pan",
			// 			delta.x / getPixelsWidth() * circuitView->getViewManager().getViewWidth(),
			// 			delta.y / getPixelsHeight() * circuitView->getViewManager().getViewHeight()
			// 		))) event.StopPropagation();
			// 	}
			// }
		}
	));
}

void CircuitViewWidget::render() {
	if (doResize) {
		doResize = false;
		int w = this->parent->GetClientWidth();
		int h = this->parent->GetClientHeight();
		int x = this->parent->GetAbsoluteLeft() + this->parent->GetClientLeft();
		int y = this->parent->GetAbsoluteTop() + this->parent->GetClientTop();

		circuitView->getViewManager().setAspectRatio((float)w / (float)h);

		renderer->resize(w, h);
		renderer->reposition(x, y);
	}
	Evaluator* evaluator = circuitView->getEvaluator();
	std::string tpsText = "Real tps: N/A";
	if (evaluator) {
		tpsText = "Real tps: " + std::format("{:.2f}", (double)(evaluator->getRealTickrate()));
	}
	Rml::Element* realTpsDisplay = parent->GetElementById("real-tps-display");
	Rml::Element* realTpsDisplayElement = realTpsDisplay->GetChild(0);
	if (!realTpsDisplayElement)
		realTpsDisplayElement = realTpsDisplay->AppendChild(std::move(realTpsDisplay->GetOwnerDocument()->CreateTextNode("")));
	Rml::ElementText* realTpsDisplayText = rmlui_dynamic_cast<Rml::ElementText*>(realTpsDisplayElement);
	if (realTpsDisplayText->GetText() != tpsText) {
		realTpsDisplayText->SetText(tpsText);
	}
	renderer->render();
}

void CircuitViewWidget::setSimState(bool state) {
	if (circuitView->getEvaluator())
		circuitView->getEvaluator()->setPause(!state);
}

void CircuitViewWidget::simUseSpeed(bool state) {
	if (circuitView->getEvaluator())
		circuitView->getEvaluator()->setUseTickrate(state);
}

void CircuitViewWidget::setSimSpeed(double speed) {
	if (circuitView->getEvaluator())
		circuitView->getEvaluator()->setTickrate(std::round(speed * 60));
}

void CircuitViewWidget::newCircuit() {
	circuit_id_t id = circuitView->getBackend()->createCircuit();
	circuitView->getBackend()->linkCircuitViewWithCircuit(circuitView.get(), id);
	for (auto& iter : circuitView->getBackend()->getEvaluatorManager().getEvaluators()) {
		if (iter.second->getCircuitId(Address()) == id) {
			circuitView->getBackend()->linkCircuitViewWithEvaluator(circuitView.get(), iter.first, Address());
			return;
		}
	}
}

void CircuitViewWidget::setStatusBar(const std::string& text) {
	Rml::Element* statusBar = parent->GetElementById("status-bar");
	statusBar->SetInnerRML(text);
}

// save current circuit view widget we are viewing. Right now only works if it is the only widget in application.
// Called via Ctrl-S keybind
void CircuitViewWidget::save() {
	if (fileManager && circuitView->getCircuit()) {
		if (!fileManager->save(circuitView->getCircuit()->getUUID())) {
			// if failed to save the circuit with out a path
			static SDL_DialogFileFilter filter;
			filter.name = "Circuit Files";
			filter.pattern = "*.cir";
			SDL_ShowSaveFileDialog(SaveCallback, this, window, &filter, 0, nullptr);
		}
	}
}

void CircuitViewWidget::asSave() {
	if (fileManager && circuitView->getCircuit()) {
		static SDL_DialogFileFilter filter;
		filter.name = "Circuit Files";
		filter.pattern = "*.cir";
		SDL_ShowSaveFileDialog(SaveCallback, this, window, &filter, 0, nullptr);
	}
}

// for drag and drop load directly onto this circuit view widget
void CircuitViewWidget::load() {
	if (!fileManager) return;

	static const SDL_DialogFileFilter filters[] = {
		{ "Circuit Files",  "*.cir" },
		{ "OpenCircuit Files", "*.circiut" },
	};

	SDL_ShowOpenFileDialog(LoadCallback, this, nullptr, nullptr, 0, nullptr, true);
}

inline Vec2 CircuitViewWidget::pixelsToView(const SDL_FPoint& point) const {
	return Vec2((point.x - getPixelsXPos()) / getPixelsWidth(), (point.y - getPixelsYPos()) / getPixelsHeight());
}

inline bool CircuitViewWidget::insideWindow(const SDL_FPoint& point) const {
	int x = point.x - getPixelsXPos();
	int y = point.y - getPixelsYPos();
	return x >= 0 && y >= 0 && x < getPixelsWidth() && y < getPixelsHeight();
}

inline float CircuitViewWidget::getPixelsWidth() const {
	return parent->GetClientWidth();
}

inline float CircuitViewWidget::getPixelsHeight() const {
	return parent->GetClientHeight();
}

inline float CircuitViewWidget::getPixelsXPos() const {
	return parent->GetAbsoluteLeft() + parent->GetClientLeft();
}

inline float CircuitViewWidget::getPixelsYPos() const {
	return parent->GetAbsoluteTop() + parent->GetClientTop();
}
