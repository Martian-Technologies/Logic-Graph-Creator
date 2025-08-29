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

	Rml::LoadFontFace((DirectoryManager::getResourceDirectory() / "gui/fonts/monaspace.otf").generic_string());
}

RmlInstance::~RmlInstance() {
	logInfo("Shutting down RmlUI...");
	renderInterface->setWindowToRenderOn(0);
	Rml::Shutdown();
}
