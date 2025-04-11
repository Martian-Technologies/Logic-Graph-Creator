#ifndef circuitViewWidget_h
#define circuitViewWidget_h

#include "computerAPI/circuits/circuitFileManager.h"
#include "backend/circuitView/circuitView.h"
#include "util/vec2.h"

class CircuitViewWidget {
public:
	CircuitViewWidget(CircuitFileManager* fileManager);

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
	// std::unique_ptr<Renderer> renderer;
	CircuitFileManager* fileManager;


	// settings (temp)
	bool mouseControls;
};

#endif /* circuitViewWidget_h */