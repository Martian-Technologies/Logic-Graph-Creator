#ifndef rmlInstance
#define rmlInstance

#include "gui/rml/rmlSystemInterface.h"
#include "gui/rml/rmlRenderInterface.h"

class RmlInstance {
public:
	RmlInstance(RmlSystemInterface* systemInterface, RmlRenderInterface* renderInterface);
	~RmlInstance();

private:
	RmlRenderInterface* renderInterface;
};

#endif
