#include "mainRenderer.h"

MainRenderer& MainRenderer::get() {
	static MainRenderer singleton;
	return singleton;
}

WindowID MainRenderer::registerWindow(SdlWindow* sdlWindow) {
	auto pair = windowRenderers.try_emplace(getNewWindowID(), sdlWindow, &vulkanInstance);
	return lastWindowID;
}

void MainRenderer::resizeWindow(WindowID windowID, glm::vec2 size) {
	auto iter = windowRenderers.find(windowID);
	if (iter == windowRenderers.end()) {
		logError("Failed to call resizeWindow on non existent window {}.", "MainRenderer", windowID);
		return;
	}
	iter->second.resize({size.x, size.y});
}

void MainRenderer::deregisterWindow(WindowID windowID) {
	auto iter = windowRenderers.find(windowID);
	if (iter == windowRenderers.end()) {
		logError("Failed to call deregisterWindow on non existent window {}.", "MainRenderer", windowID);
		return;
	}
	windowRenderers.erase(iter);
}

void MainRenderer::prepareForRmlRender(WindowID windowID) {
	auto iter = windowRenderers.find(windowID);
	if (iter == windowRenderers.end()) {
		logError("Failed to call prepareForRmlRender on non existent window {}.", "MainRenderer", windowID);
		return;
	}
	iter->second.getRmlRenderer().prepareForRmlRender();
}

void MainRenderer::endRmlRender(WindowID windowID) {
	auto iter = windowRenderers.find(windowID);
	if (iter == windowRenderers.end()) {
		logError("Failed to call endRmlRender on non existent window {}", "MainRenderer", windowID);
		return;
	}
	iter->second.getRmlRenderer().endRmlRender();
}

Rml::CompiledGeometryHandle MainRenderer::compileGeometry(WindowID windowID, Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices) {
	auto iter = windowRenderers.find(windowID);
	if (iter == windowRenderers.end()) {
		logError("Failed to call CompileGeometry on non existent window {}", "MainRenderer", windowID);
		return (Rml::CompiledGeometryHandle)0;
	}
	return iter->second.getRmlRenderer().compileGeometry(vertices, indices);
}

void MainRenderer::releaseGeometry(WindowID windowID, Rml::CompiledGeometryHandle geometry) {
	auto iter = windowRenderers.find(windowID);
	if (iter == windowRenderers.end()) {
		logError("Failed to call ReleaseGeometry on non existent window {}", "MainRenderer", windowID);
		return;
	}
	iter->second.getRmlRenderer().releaseGeometry(geometry);
}

Rml::TextureHandle MainRenderer::loadTexture(WindowID windowID, Rml::Vector2i& texture_dimensions, const Rml::String& source) {
	auto iter = windowRenderers.find(windowID);
	if (iter == windowRenderers.end()) {
		logError("Failed to call loadTexture on non existent window {}", "MainRenderer", windowID);
		return (Rml::TextureHandle)0;
	}
	return iter->second.getRmlRenderer().loadTexture(texture_dimensions, source);
}

Rml::TextureHandle MainRenderer::generateTexture(WindowID windowID, Rml::Span<const Rml::byte> source, Rml::Vector2i source_dimensions) {
	auto iter = windowRenderers.find(windowID);
	if (iter == windowRenderers.end()) {
		logError("Failed to call generateTexture on non existent window {}", "MainRenderer", windowID);
		return (Rml::TextureHandle)0;
	}
	return iter->second.getRmlRenderer().generateTexture(source, source_dimensions);
}

void MainRenderer::releaseTexture(WindowID windowID, Rml::TextureHandle texture_handle) {
	auto iter = windowRenderers.find(windowID);
	if (iter == windowRenderers.end()) {
		logError("Failed to call releaseTexture on non existent window {}", "MainRenderer", windowID);
		return;
	}
	iter->second.getRmlRenderer().releaseTexture(texture_handle);
}

void MainRenderer::renderGeometry(WindowID windowID, Rml::CompiledGeometryHandle handle, Rml::Vector2f translation, Rml::TextureHandle texture) {
	auto iter = windowRenderers.find(windowID);
	if (iter == windowRenderers.end()) {
		logError("Failed to call renderGeometry on non existent window {}", "MainRenderer", windowID);
		return;
	}
	iter->second.getRmlRenderer().renderGeometry(handle, translation, texture);
}

void MainRenderer::enableScissorRegion(WindowID windowID, bool enable) {
	auto iter = windowRenderers.find(windowID);
	if (iter == windowRenderers.end()) {
		logError("Failed to call enableScissorRegion on non existent window {}", "MainRenderer", windowID);
		return;
	}
	iter->second.getRmlRenderer().enableScissorRegion(enable);
}

void MainRenderer::setScissorRegion(WindowID windowID, Rml::Rectanglei region) {
	auto iter = windowRenderers.find(windowID);
	if (iter == windowRenderers.end()) {
		logError("Failed to call setScissorRegion on non existent window {}", "MainRenderer", windowID);
		return;
	}
	iter->second.getRmlRenderer().setScissorRegion(region);
}
	
ViewportID MainRenderer::registerViewport(WindowID windowID, glm::vec2 origin, glm::vec2 size, Rml::Element* element) {
	auto iter = windowRenderers.find(windowID);
	if (iter == windowRenderers.end()) {
		logError("Failed to call registerViewport on non existent window {}", "MainRenderer", windowID);
		return 0;
	}
	auto pair = viewportRenderers.try_emplace(windowID, getNewViewportID(), iter->second.getDevice(), element, &(iter->second));
	iter->second.registerViewportRenderInterface(&(pair.first->second));
	return lastViewportID;
}

void MainRenderer::moveViewport(ViewportID viewportID, WindowID windowID, glm::vec2 origin, glm::vec2 size) {
	auto viewportIter = viewportRenderers.find(viewportID);
	if (viewportIter == viewportRenderers.end()) {
		logError("Failed to call moveViewport on non existent viewport {}", "MainRenderer", viewportID);
		return;
	}
	if (viewportIter->second.getWindowID() != windowID) {
		auto windowIter = windowRenderers.find(windowID);
		if (windowIter == windowRenderers.end()) {
			logError("Failed to call moveViewport on non existent window {}", "MainRenderer", windowID);
			return;
		}
		logError("moving viewport to other window not supported yet");
		return;
	}
	viewportIter->second.updateViewFrame(origin, size);
}

void MainRenderer::moveViewportView(ViewportID viewportID, FPosition topLeft, FPosition bottomRight) {
	auto iter = viewportRenderers.find(viewportID);
	if (iter == viewportRenderers.end()) {
		logError("Failed to call moveViewportView on non existent viewport {}", "MainRenderer", viewportID);
		return;
	}
	iter->second.updateView(topLeft, bottomRight);
}

void MainRenderer::setViewportCircuit(ViewportID viewportID, Circuit* circuit) {
	auto iter = viewportRenderers.find(viewportID);
	if (iter == viewportRenderers.end()) {
		logError("Failed to call setViewportCircuit on non existent viewport {}", "MainRenderer", viewportID);
		return;
	}
	iter->second.setCircuit(circuit);
	iter->second.setEvaluator(nullptr);
}
void MainRenderer::setViewportEvaluator(ViewportID viewportID, Circuit* circuit, Evaluator* evaluator, Address address) {
	auto iter = viewportRenderers.find(viewportID);
	if (iter == viewportRenderers.end()) {
		logError("Failed to call setViewportEvaluator on non existent viewport {}", "MainRenderer", viewportID);
		return;
	}
	iter->second.setCircuit(circuit);
	iter->second.setEvaluator(evaluator);
	iter->second.setEvaluator(evaluator);
}
void MainRenderer::deregisterViewport(ViewportID viewport) { }
// void sendDiff(ViewportID viewport, ...)

ElementID MainRenderer::addSelectionObjectElement(ViewportID viewportID, const SelectionObjectElement& selection) {
	auto iter = viewportRenderers.find(viewportID);
	if (iter == viewportRenderers.end()) {
		logError("Failed to call addSelectionObjectElement on non existent viewport {}", "MainRenderer", viewportID);
		return 0;
	}
	return iter->second.addSelectionObjectElement(selection);
}

ElementID MainRenderer::addSelectionElement(ViewportID viewportID, const SelectionElement& selection) {
	auto iter = viewportRenderers.find(viewportID);
	if (iter == viewportRenderers.end()) {
		logError("Failed to call addSelectionElement on non existent viewport {}", "MainRenderer", viewportID);
		return 0;
	}
	return iter->second.addSelectionElement(selection);
}

void MainRenderer::removeSelectionElement(ViewportID viewportID, ElementID id) {
	auto iter = viewportRenderers.find(viewportID);
	if (iter == viewportRenderers.end()) {
		logError("Failed to call removeSelectionElement on non existent viewport {}", "MainRenderer", viewportID);
		return;
	}
	iter->second.removeSelectionElement(id);
}

ElementID MainRenderer::addBlockPreview(ViewportID viewportID, BlockPreview&& blockPreview) {
	auto iter = viewportRenderers.find(viewportID);
	if (iter == viewportRenderers.end()) {
		logError("Failed to call addBlockPreview on non existent viewport {}", "MainRenderer", viewportID);
		return 0;
	}
	return iter->second.addBlockPreview(std::move(blockPreview));
}

void MainRenderer::shiftBlockPreview(ViewportID viewportID, ElementID id, Vector shift) {
	auto iter = viewportRenderers.find(viewportID);
	if (iter == viewportRenderers.end()) {
		logError("Failed to call shiftBlockPreview on non existent viewport {}", "MainRenderer", viewportID);
		return;
	}
	iter->second.shiftBlockPreview(id, shift);
}

void MainRenderer::removeBlockPreview(ViewportID viewportID, ElementID id) {
	auto iter = viewportRenderers.find(viewportID);
	if (iter == viewportRenderers.end()) {
		logError("Failed to call removeBlockPreview on non existent viewport {}", "MainRenderer", viewportID);
		return;
	}
	iter->second.removeBlockPreview(id);
}

ElementID MainRenderer::addConnectionPreview(ViewportID viewportID, const ConnectionPreview& connectionPreview) {
	auto iter = viewportRenderers.find(viewportID);
	if (iter == viewportRenderers.end()) {
		logError("Failed to call addConnectionPreview on non existent viewport {}", "MainRenderer", viewportID);
		return 0;
	}
	return iter->second.addConnectionPreview(connectionPreview);
}

void MainRenderer::removeConnectionPreview(ViewportID viewportID, ElementID id) {
	auto iter = viewportRenderers.find(viewportID);
	if (iter == viewportRenderers.end()) {
		logError("Failed to call removeConnectionPreview on non existent viewport {}", "MainRenderer", viewportID);
		return;
	}
	iter->second.removeConnectionPreview(id);
}

ElementID MainRenderer::addHalfConnectionPreview(ViewportID viewportID, const HalfConnectionPreview& halfConnectionPreview) {
	auto iter = viewportRenderers.find(viewportID);
	if (iter == viewportRenderers.end()) {
		logError("Failed to call addHalfConnectionPreview on non existent viewport {}", "MainRenderer", viewportID);
		return 0;
	}
	return iter->second.addHalfConnectionPreview(halfConnectionPreview);
}

void MainRenderer::removeHalfConnectionPreview(ViewportID viewportID, ElementID id) {
	auto iter = viewportRenderers.find(viewportID);
	if (iter == viewportRenderers.end()) {
		logError("Failed to call removeHalfConnectionPreview on non existent viewport {}", "MainRenderer", viewportID);
		return;
	}
	iter->second.removeHalfConnectionPreview(id);
}
