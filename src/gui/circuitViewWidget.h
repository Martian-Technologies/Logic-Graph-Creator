#ifndef circuitViewWidget_h
#define circuitViewWidget_h

#include <RmlUi/Core.h>
#include <SDL3/SDL.h>
#include "sdl/sdlWindow.h"

#include "computerAPI/circuits/circuitFileManager.h"
#include "backend/circuitView/circuitView.h"
#include "interaction/keybindHandler.h"
#include "util/vec2.h"

#include "renderer/sdlRenderer.h"

class CircuitViewWidget {
public:
	CircuitViewWidget(CircuitFileManager* fileManager, Rml::ElementDocument* document, Rml::Element* parent, SDL_Window* window, SDL_Renderer* sdlRenderer);
	~CircuitViewWidget() { parent->RemoveEventListener("keydown", &keybindHandler); }

	// setup
	inline CircuitView* getCircuitView() { return circuitView.get(); }
	inline CircuitFileManager* getFileManager() { return fileManager; }
	void setSimState(bool state);
	void simUseSpeed(bool state);
	void setSimSpeed(double speed);

	void render() { renderer->render(); }

	void load();
	void save();

private:
	// utility functions
	inline Vec2 pixelsToView(const SDL_Point& point) const;
	inline bool insideWindow(const SDL_Point& point) const;
	inline float getPixelsWidth() const;
	inline float getPixelsHeight() const;
	inline float getPixelsXPos() const;
	inline float getPixelsYPos() const;

	std::unique_ptr<CircuitView> circuitView;
	std::unique_ptr<SdlRenderer> renderer;
	CircuitFileManager* fileManager;
	Rml::ElementDocument* document;
	Rml::Element* parent;
	SDL_Window* window;
	KeybindHandler keybindHandler;

	// settings (temp)
	bool mouseControls = false;
};

#endif /* circuitViewWidget_h */