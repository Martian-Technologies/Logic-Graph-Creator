#include "sdlWindow.h"
#include "util/fastMath.h"

SdlWindow::SdlWindow(const std::string& name) {
	logInfo("Creating SDL window...");
	handle = SDL_CreateWindow(name.c_str(), 800, 600, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
	if (!handle) {
		throw std::runtime_error("SDL could not create window! SDL_Error: " + std::string(SDL_GetError()));
	}

	int winW, winH, drawW, drawH;
	SDL_GetWindowSize(handle, &winW, &winH);
	SDL_GetWindowSizeInPixels(handle, &drawW, &drawH);

	float scaleX = (float)drawW / winW;
	float scaleY = (float)drawH / winH;

	if (!approx_equals(scaleX, scaleY)) {
		logError("width scale not the same x={} y={}", "SdlWindow", scaleX, scaleY);
	}

	windowScalingSize = scaleX;
}

SdlWindow::~SdlWindow() {
	logInfo("Destroying SDL window...");
	SDL_DestroyWindow(handle);
}

bool SdlWindow::isThisMyEvent(const SDL_Event& event) {
	return SDL_GetWindowFromEvent(&event) == handle;
}
