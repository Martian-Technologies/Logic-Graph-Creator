#ifndef rmlInstance
#define rmlInstance

#include <RmlUi/Core.h>

class RmlInstance {
public:
	RmlInstance(Rml::SystemInterface* systemInterface, Rml::RenderInterface* renderInterface);
	~RmlInstance();
};

#endif
