#ifndef circuitViewWidget_h
#define circuitViewWidget_h

#include "backend/circuitView/renderer/rendererTMP.h"
#include "computerAPI/circuits/circuitFileManager.h"
#include "backend/circuitView/circuitView.h"
#include "interaction/keybindHandler.h"
#include "util/vec2.h"

#include <RmlUi/Core.h>

class CircuitViewWidget {
public:
	CircuitViewWidget(CircuitFileManager* fileManager, Rml::ElementDocument* document, Rml::Element* parent);
	~CircuitViewWidget() { parent->RemoveEventListener("keydown", &keybindHandler); }

	// setup
	inline CircuitView* getCircuitView() { return circuitView.get(); }
	void setSimState(bool state);
	void simUseSpeed(bool state);
	void setSimSpeed(double speed);

	void load(const std::string& filePath);
	void save();

private:
	// utility functions
	// inline float getPixelsWidth() { return (float)rect().width(); }
	// inline float getPixelsHeight() { return (float)rect().height(); }

	std::unique_ptr<CircuitView> circuitView;
	std::unique_ptr<RendererTMP> renderer;
	CircuitFileManager* fileManager;
	Rml::ElementDocument* document;
	Rml::Element* parent;
	KeybindHandler keybindHandler;

	// settings (temp)
	bool mouseControls = false;
};

#endif /* circuitViewWidget_h */