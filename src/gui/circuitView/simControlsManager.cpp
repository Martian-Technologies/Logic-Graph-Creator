#include "simControlsManager.h"
#include "gui/interaction/eventPasser.h"
#include <RmlUi/Core/Input.h>

SimControlsManager::SimControlsManager(Rml::ElementDocument* document) {
	statusArea = document->GetElementById("status-area");
	Initialize();
}

SimControlsManager::~SimControlsManager() {

}

void SimControlsManager::Initialize() {
	Rml::Element* toggleSimElement = statusArea->GetElementById("toggle-simulation-option");
	Rml::Element* limitSpeedElement = statusArea->GetElementById("limit-speed-checkbox");
	Rml::Element* tpsInputElement = statusArea->GetElementById("tps-input");

	toggleSimElement->AddEventListener("click", new EventPasser(
		[this](Rml::Event& event) {
			toggleSimulation();
		}
	));

	limitSpeedElement->AddEventListener("change", new EventPasser(
		[this](Rml::Event& event) {
			limitSpeed();
		}
	));

	tpsInputElement->AddEventListener("keydown", new EventPasser(
		[this](Rml::Event& event) {
			int keyIdentifier = event.GetParameter<int>("key_identifier", 0);
			if (keyIdentifier == Rml::Input::KI_RETURN) {
				setTPS(event.GetCurrentElement());
			
				// event.StopPropagation(); // sets width to amount of info added
			}
		}
	));
}

void SimControlsManager::toggleSimulation() {
	toggleSim = !toggleSim;
	logInfo("toggled sim");
}
void SimControlsManager::limitSpeed() {
	limitSimSpeed = false;
	logInfo("limited sim speed");
}
void SimControlsManager::setTPS(Rml::Element* element) {
	Rml::String value = element->GetAttribute<Rml::String>("value", "");

	element->SetInnerRML(value + "tps");

	// logInfo("very janky, needs works");
	logInfo(static_cast<std::string>(value));
}
