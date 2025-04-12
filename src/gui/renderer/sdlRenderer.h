#ifndef sdlRenderer_h
#define sdlRenderer_h

#include <SDL3/SDL.h>

#include "backend/circuitView/renderer/renderer.h"
#include "backend/circuitView/renderer/tileSet.h"

class SdlRenderer : public Renderer {
public:
	SdlRenderer(SDL_Renderer* sdlRenderer);

	// general flow
	void initializeTileSet(const std::string& filePath);
	void resize(int w, int h);
	void reposition(int x, int y);
	void render();

	// updating
	void setCircuit(Circuit* circuit) override;
	void setEvaluator(Evaluator* evaluator) override;

	void updateView(ViewManager* viewManager) override;
	virtual void updateCircuit(DifferenceSharedPtr diff) override;

	inline float getLastFrameTimeMs() const override { return lastFrameTime; }

private:
	// elements
	ElementID addSelectionElement(const SelectionObjectElement& selection) override;
	ElementID addSelectionElement(const SelectionElement& selection) override;
	void removeSelectionElement(ElementID selection) override;

	ElementID addBlockPreview(const BlockPreview& blockPreview) override;
	void removeBlockPreview(ElementID blockPreview) override;

	ElementID addConnectionPreview(const ConnectionPreview& connectionPreview) override;
	void removeConnectionPreview(ElementID connectionPreview) override;

	ElementID addHalfConnectionPreview(const HalfConnectionPreview& halfConnectionPreview) override;
	void removeHalfConnectionPreview(ElementID halfConnectionPreview) override;

	void spawnConfetti(FPosition start) override;

private:
	SDL_Point gridToSDL(FPosition position);
	SDL_Point gridToSDL(FVector vector);
	inline float scalePixelCount(float pixelCount) { return pixelCount / viewManager->getViewHeight() * ((float)h) / 500.f; }

	SDL_Color getStateColor(logic_state_t state);

	void renderSelection(const SharedSelection selection, SelectionObjectElement::RenderMode mode, unsigned int depth = 0);
	void renderBlock(BlockType type, Position position, Rotation rotation, logic_state_t state = logic_state_t::UNDEFINED);
	void renderConnection(FPosition aPos, FPosition bPos, FVector aControlOffset, FVector bControlOffset, logic_state_t state);
	void renderConnection(Position aPos, const Block* a, Position bPos, const Block* b, logic_state_t state);
	void renderConnection(Position aPos, Position bPos, logic_state_t state);
	void renderConnection(Position aPos, FPosition bPos, logic_state_t state);

	void drawArrow(const SDL_Point& start, const SDL_Point& end, float size, const SDL_Color& color);
	void drawText(const SDL_Point& center, const std::string& text, float size, const SDL_Color& color);

	int w, h, x, y;
	Circuit* circuit;
	Evaluator* evaluator;
	ViewManager* viewManager;
	SDL_Texture* tileSet;
	SDL_Renderer* sdlRenderer;
	std::unique_ptr<TileSetInfo> tileSetInfo;

	// Elements
	ElementID currentID = 0;
	std::unordered_map<ElementID, SelectionElement> selectionElements;
	std::unordered_map<ElementID, SelectionElement> invertedSelectionElements;
	std::unordered_map<ElementID, SelectionObjectElement> selectionObjectElements;
	std::unordered_map<ElementID, BlockPreview> blockPreviews;
	std::unordered_map<ElementID, ConnectionPreview> connectionPreviews;
	std::unordered_map<ElementID, HalfConnectionPreview> halfConnectionPreviews;

	// info
	float lastFrameTime = 0.0f;
};


#endif /* sdlRenderer_h */
