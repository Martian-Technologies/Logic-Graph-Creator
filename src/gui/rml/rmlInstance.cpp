#include "rmlInstance.h"

#include <RmlUi/Core.h>

#include "computerAPI/directoryManager.h"

RmlInstance::RmlInstance(SystemInterface_SDL* systemInterface, RmlRenderInterface* renderInterface) {
	logInfo("Initializing RmlUI...");

	Rml::SetSystemInterface(systemInterface);
	Rml::SetRenderInterface(renderInterface);
	
	if (!Rml::Initialise()) {
		throwFatalError("Could not initialize RmlUI.");
	}

	Rml::LoadFontFace((DirectoryManager::getResourceDirectory() / "gui/fonts/monaspace.otf").string());
}
	
RmlInstance::~RmlInstance() {
	logInfo("Shutting down RmlUI...");
	Rml::Shutdown();
}
