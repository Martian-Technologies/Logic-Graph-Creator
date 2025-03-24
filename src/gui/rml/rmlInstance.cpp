#include "rmlInstance.h"

RmlInstance::RmlInstance(Rml::SystemInterface* systemInterface, Rml::RenderInterface* renderInterface) {
	logInfo("Initializing RmlUI...");

	Rml::SetSystemInterface(systemInterface);
	Rml::SetRenderInterface(renderInterface);
	if (!Rml::Initialise()) {
		throw std::runtime_error("Could not initialize RmlUI.");
	}
}
	
RmlInstance::~RmlInstance() {
	logInfo("Shutting down RmlUI...");
	Rml::Shutdown();
}
