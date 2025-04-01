#ifndef rmlInstance
#define rmlInstance

#include "gui/rml/RmlUi_Platform_SDL.h"
#include "gui/rml/RmlUi_Renderer_SDL.h"

class RmlInstance {
public:
	RmlInstance(SystemInterface_SDL* systemInterface, RenderInterface_SDL* renderInterface);
	~RmlInstance();
};

#endif
