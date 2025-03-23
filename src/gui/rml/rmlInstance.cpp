#include "rmlInstance.h"

RmlInstance::RmlInstance() {
	logInfo("Initializing RmlUI...");

	if (!Rml::Initialise()) {
		throw std::runtime_error("Could not initialize RmlUI.");
	}
}
	
RmlInstance::~RmlInstance() {
	logInfo("Shutting down RmlUI...");
	Rml::Shutdown();
}
