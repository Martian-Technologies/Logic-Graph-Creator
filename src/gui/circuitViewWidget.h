#ifndef circuitViewWidget_h
#define circuitViewWidget_h

#include <RmlUi/Core.h>
#include <SDL3/SDL.h>

#include "gpu/renderer/viewportRenderInterface.h"
#include "gpu/renderer/windowRenderer.h"
#include "sdl/sdlWindow.h"

#include "computerAPI/circuits/circuitFileManager.h"
#include "backend/circuitView/circuitView.h"
#include "interaction/keybindHandler.h"
#include "util/vec2.h"

class CircuitViewWidget {
public:
	CircuitViewWidget(CircuitFileManager* fileManager, Rml::ElementDocument* document, Rml::Element* element, SDL_Window* window, WindowRenderer* windowRenderer);
	~CircuitViewWidget() { element->RemoveEventListener("keydown", &keybindHandler); }

	// setup
	inline CircuitView* getCircuitView() { return circuitView.get(); }
	inline CircuitFileManager* getFileManager() { return fileManager; }
	void setSimState(bool state);
	void simUseSpeed(bool state);
	void setSimSpeed(double speed);

	void handleResize();
	
	void load();
	void save();

private:
	// utility functions
	inline Vec2 pixelsToView(const SDL_FPoint& point) const;
	inline bool insideWindow(const SDL_FPoint& point) const;
	inline float getPixelsWidth() const;
	inline float getPixelsHeight() const;
	inline float getPixelsXPos() const;
	inline float getPixelsYPos() const;

	std::unique_ptr<ViewportRenderInterface> rendererInterface;
	std::unique_ptr<CircuitView> circuitView;
	CircuitFileManager* fileManager;
	Rml::ElementDocument* document;
	Rml::Element* element;
	SDL_Window* window;
	KeybindHandler keybindHandler;
	bool doResize = false;

	// settings (temp)
	bool mouseControls = false;
};

#endif /* circuitViewWidget_h */
