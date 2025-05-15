#include "simControlsManager.h"

#include <RmlUi/Core/Input.h>

#include "gui/interaction/eventPasser.h"
#include "gui/circuitViewWidget.h"

SimControlsManager::SimControlsManager(Rml::ElementDocument* document, std::shared_ptr<CircuitViewWidget> circuitViewWidget, DataUpdateEventManager* dataUpdateEventManager) : circuitViewWidget(circuitViewWidget), dataUpdateEventReceiver(dataUpdateEventManager) {
	toggleSimElement = document->GetElementById("toggle-simulation");
	limitSpeedElement = document->GetElementById("limit-speed-checkbox");
	tpsInputElement = document->GetElementById("tps-input");

	toggleSimElement->AddEventListener("click", new EventPasser(std::bind(&SimControlsManager::toggleSimulation, this)));
	limitSpeedElement->AddEventListener("click", new EventPasser([this](Rml::Event& event) {
		limitSpeed();
		event.StopPropagation();
	}));
	tpsInputElement->AddEventListener("change", new EventPasser(std::bind(&SimControlsManager::setTPS, this)));

	dataUpdateEventReceiver.linkFunction("circuitViewChangeEvaluator", std::bind(&SimControlsManager::update, this));
	update();
}

void SimControlsManager::update() {
	Evaluator* evaluator = circuitViewWidget->getCircuitView()->getEvaluator();
	if (evaluator) {
		if (evaluator->isPause()) {
			toggleSimElement->SetClass("checked", false);
		} else {
			toggleSimElement->SetClass("checked", true);
		}
		if (evaluator->getUseTickrate()) {
			limitSpeedElement->SetAttribute("checked", true);
		} else {
			limitSpeedElement->RemoveAttribute("checked");
		}
		std::string tpsStr = std::format("{:.1f}", (double)(evaluator->getTickrate())/60.);
		if (tpsStr.back() == '0') tpsStr.pop_back();
		if (tpsStr.back() == '.') tpsStr.pop_back();
		tpsInputElement->SetInnerRML(std::string(tpsStr.size(), ' ') + "tps");
		tpsInputElement->SetAttribute<Rml::String>("value", tpsStr);
	} else {
		toggleSimElement->SetClass("checked", false);
		limitSpeedElement->RemoveAttribute("checked");
		tpsInputElement->SetInnerRML("tps");
		tpsInputElement->SetAttribute<Rml::String>("value", "");
	}
}

void SimControlsManager::toggleSimulation() {
	Evaluator* evaluator = circuitViewWidget->getCircuitView()->getEvaluator();
	if (evaluator) {
		if (evaluator->isPause()) {
			evaluator->setPause(false);
		} else {
			evaluator->setPause(true);
		}
	}
	update();
}

void SimControlsManager::limitSpeed() {
	Evaluator* evaluator = circuitViewWidget->getCircuitView()->getEvaluator();
	if (evaluator) {
		evaluator->setUseTickrate(!(evaluator->getUseTickrate()));
	}
	update();
}

void SimControlsManager::setTPS() {
	Evaluator* evaluator = circuitViewWidget->getCircuitView()->getEvaluator();
	if (evaluator) {
		Rml::String value = tpsInputElement->GetAttribute<Rml::String>("value", "");
		std::stringstream ss(value);
		double tps = 0;
		ss >> tps;
		if (tps == 0) {
			std::string tpsStr = "";
			for (char c : value) {
				if (c == '0') {
					if (tpsStr.empty()) tpsStr += "0";
				} else if (c == '.') {
					tpsStr += ".";
					break;
				} else {
					break;
				}
			}
			tpsInputElement->SetInnerRML(std::string(tpsStr.size(), ' ') + "tps");
			tpsInputElement->SetAttribute<Rml::String>("value", tpsStr);
			return;
		}
		if (value.back() == '.') {
			std::string tpsStr = std::format("{:.1f}", tps);
			if (tpsStr.back() == '0') tpsStr.pop_back();
			if (tpsStr.back() == '.') {
				tpsInputElement->SetInnerRML(std::string(tpsStr.size(), ' ') + "tps");
				tpsInputElement->SetAttribute<Rml::String>("value", tpsStr);
				return;
			}
		}
		evaluator->setTickrate((unsigned long long)(tps*60.));
	}
	update();
}
