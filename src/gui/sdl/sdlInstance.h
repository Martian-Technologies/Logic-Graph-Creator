#ifndef SdlInstance_h
#define SdlInstance_h

#include <SDL3/SDL.h>

class SdlInstance {
public:
	SdlInstance();
	~SdlInstance();

	std::vector<SDL_Event> pollEvents();
};

#endif
