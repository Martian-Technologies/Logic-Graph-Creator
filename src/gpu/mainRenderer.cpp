#include "mainRenderer.h"

MainRenderer& MainRenderer::get() {
	static MainRenderer singleton;
	return singleton;
}

WindowID MainRenderer::registerWindow(SdlWindow window) { }
void MainRenderer::resizeWindow(WindowID window, glm::vec2 size) { }
void MainRenderer::deregisterWindow(WindowID window) { }

void MainRenderer::prepareForRml(WindowID windowID) { }
void MainRenderer::endRml(WindowID windowID) { }

Rml::CompiledGeometryHandle MainRenderer::CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices) { }
void MainRenderer::ReleaseGeometry(Rml::CompiledGeometryHandle geometry) { }

Rml::TextureHandle MainRenderer::LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source) { }
Rml::TextureHandle MainRenderer::GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i source_dimensions) { }
void MainRenderer::ReleaseTexture(Rml::TextureHandle texture_handle) { }

void MainRenderer::RenderGeometry(WindowID window, Rml::CompiledGeometryHandle handle, Rml::Vector2f translation, Rml::TextureHandle texture) { }
void MainRenderer::EnableScissorRegion(WindowID window, bool enable) { }
void MainRenderer::SetScissorRegion(WindowID window, Rml::Rectanglei region) { }
	
ViewportID MainRenderer::registerViewport(WindowID window, glm::vec2 origin, glm::vec2 size) { }
void MainRenderer::moveViewport(ViewportID viewport, WindowID window, glm::vec2 origin, glm::vec2 size) { }
void MainRenderer::moveViewportView(ViewportID viewportID, FPosition viewCenter, float viewScale) { }
void MainRenderer::setViewportCircuit(ViewportID viewport, Circuit* circuit) { }
void MainRenderer::setViewportEvaluator(ViewportID viewport, Circuit* circuit, Evaluator* evaluator, Address address) { }
void MainRenderer::deregisterViewport(ViewportID viewport) { }
// void sendDiff(ViewportID viewport, ...)

ElementID MainRenderer::addSelectionObjectElement(ViewportID viewport, const SelectionObjectElement& selection) { }
ElementID MainRenderer::addSelectionElement(ViewportID viewport, const SelectionElement& selection) { }
void MainRenderer::removeSelectionElement(ViewportID viewport, ElementID id) { }

ElementID MainRenderer::addBlockPreview(ViewportID viewport, BlockPreview&& blockPreview) { }
void MainRenderer::shiftBlockPreview(ViewportID viewport, ElementID id, Vector shift) { }
void MainRenderer::removeBlockPreview(ViewportID viewport, ElementID id) { }

ElementID MainRenderer::addConnectionPreview(ViewportID viewport, const ConnectionPreview& connectionPreview) { }
void MainRenderer::removeConnectionPreview(ViewportID viewport, ElementID id) { }

ElementID MainRenderer::addHalfConnectionPreview(ViewportID viewport, const HalfConnectionPreview& halfConnectionPreview) { }
void MainRenderer::removeHalfConnectionPreview(ViewportID viewport, ElementID id) { }