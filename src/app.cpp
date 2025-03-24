#include "app.h"

#include <SDL3/SDL_vulkan.h>

App::App()
	: rml(&rmlSystemInterface, &rmlRenderInterface) {

	// Creating starting window
	windows.emplace_back();

	// Set up Vulkan (temp)
	// get instance extensions
	std::vector<const char*> extensions;
	{
		unsigned int count;
		const char* const* extensions_list = SDL_Vulkan_GetInstanceExtensions(&count);
		if (!extensions_list) { throw std::runtime_error("Failed to get list of require Vulkan instance extensions."); }
		extensions.resize(count);
		for (int i = 0; i < count; i++) {
			extensions[i] = extensions_list[i];
		}
	}

	// Create create surface lambda for rmlUi
	auto CreateSurface = [&](VkInstance instance, VkSurfaceKHR* out_surface) {
		return windows.front().createSurface(instance, out_surface);
	};
	
	if (!rmlRenderInterface.Initialize(std::move(extensions), CreateSurface)) {
		throw std::runtime_error("Failed to initialize rmlUi Vulkan render interface.");
	}

	// Initialize renderer interface
	rmlSystemInterface.SetWindow(windows.front().getSdlWindow());
	int w, h;
	SDL_GetWindowSize(windows.front().getSdlWindow(), &w, &h);
	rmlRenderInterface.SetViewport(w, h);

	// Create RmlUi Context (still temp)
	rmlContext = Rml::CreateContext("main", Rml::Vector2i(w, h));
}

App::~App() {
	rmlRenderInterface.Shutdown();
}

void App::runLoop() {
	running = true;
	while (running && !windows.empty()) {
		// process events
		std::vector<SDL_Event> events = sdl.pollEvents();
		for (SDL_Event& event : events) {
			// quit event
			if (event.type == SDL_EVENT_QUIT) {
				running = false;
			}

			// send event to first window that wants it
			for (Window& window : windows) {
				if (window.recieveEvent(event)) break;
			}

			// TODO - window close and focus management (and sending to rml)

			// TEMP rmlUI
			switch (event.type) {
			case SDL_EVENT_KEY_DOWN:
			{
				const Rml::Input::KeyIdentifier key = RmlSDL::ConvertKey(event.key.key);
				const int key_modifier = RmlSDL::GetKeyModifierState();
				
				if (!RmlSDL::InputEventHandler(rmlContext, windows.front().getSdlWindow(), event))
					break;
			}
			break;
			case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
			{
				Rml::Vector2i dimensions = {event.window.data1, event.window.data2};
				rmlRenderInterface.SetViewport(dimensions.x, dimensions.y);
				rmlContext->SetDimensions(dimensions);
			}
			break;
			default:
			{
				RmlSDL::InputEventHandler(rmlContext, windows.front().getSdlWindow(), event);
			}
			break;
			}
		}

		// render each window
		rmlContext->Update();

		if (!rmlRenderInterface.IsSwapchainValid()) {
			rmlRenderInterface.RecreateSwapchain();
		}
		if (rmlRenderInterface.IsSwapchainValid()) {
			rmlRenderInterface.BeginFrame();
			rmlContext->Render();
			rmlRenderInterface.EndFrame();
		}
	}

	running = false;
}

