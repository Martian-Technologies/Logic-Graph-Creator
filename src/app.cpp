#include "app.h"
#include "gui/rml/RmlUi_Backend.h"
#include "gui/interaction/MenuTreeListener.h"
#include "computerAPI/directoryManager.h"

App::App() {
	Backend::Initialize("Gatality", 800, 600, true);
	
	Rml::SetRenderInterface(Backend::GetRenderInterface());
	Rml::SetSystemInterface(Backend::GetSystemInterface());

	Rml::Initialise();
	
	rmlContext = Rml::CreateContext("main", Rml::Vector2i(800, 600));

	Rml::LoadFontFace((DirectoryManager::getResourceDirectory() / "gui/fonts/monaspace.otf").string());
	
	Rml::ElementDocument* document = rmlContext->LoadDocument((DirectoryManager::getResourceDirectory() / "gui/mainwindow.rml").string());

	Rml::ElementList menuTreeItems;
	document->GetElementsByTagName(menuTreeItems, "li");

	for (Rml::Element* element : menuTreeItems) {
    if (element->GetClassNames().find("parent") != std::string::npos) {
      //this line causes compilation error lol!
			element->AddEventListener("click", new MenuTreeListener());
    }
	}

	document->Show();
}

App::~App() {
	Rml::Shutdown();
	Backend::Shutdown();
}

void App::runLoop() {
	while (Backend::ProcessEvents(rmlContext)) {
		rmlContext->Update();

		Backend::BeginFrame();
		rmlContext->Render();
		Backend::PresentFrame();
	}
}

