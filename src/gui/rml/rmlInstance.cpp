#include "rmlInstance.h"

#include <RmlUi/Core.h>

#include "computerAPI/directoryManager.h"

RmlInstance::RmlInstance(RmlSystemInterface* systemInterface, RmlRenderInterface* renderInterface)
	: renderInterface(renderInterface) {
  
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
	renderInterface->pointToRenderer(nullptr);
	Rml::Shutdown();
}
