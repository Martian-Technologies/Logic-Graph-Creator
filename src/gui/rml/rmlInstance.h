#ifndef rmlInstance
#define rmlInstance

#include "gui/rml/RmlUi_Platform_SDL.h"
#include "gui/rml/rmlRenderInterface.h"

class RmlInstance {
public:
	RmlInstance(SystemInterface_SDL* systemInterface, RmlRenderInterface* renderInterface);
	~RmlInstance();
};

#endif
