#ifndef STATUS_MANAGER_H
#define STATUS_MANAGER_H

#include <RmlUi/Core.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>

class SimControlsManager {
public:
	SimControlsManager(Rml::ElementDocument* document);
	~SimControlsManager();

	const Rml::String getTPS() const { return dynamic_cast<Rml::ElementFormControlTextArea*>(statusArea->GetElementById("tps-input"))->GetValue(); }
private:
	void Initialize();
	void toggleSimulation();
	void limitSpeed();
	void setTPS(Rml::Element* element);

	Rml::Element* statusArea;
	bool toggleSim;
	bool limitSimSpeed;
};

#endif
