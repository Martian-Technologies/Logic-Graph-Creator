#include <SDL3_image/SDL_image.h>

#include "backend/evaluator/logicState.h"
#include "backend/address.h"
#include "sdlRenderer.h"
#include "util/vec2.h"

inline bool SDL_SetRenderDrawColor(SDL_Renderer* renderer, const SDL_Color* color) {
	return SDL_SetRenderDrawColor(renderer, color->r, color->g, color->b, color->a);
}

inline bool SDL_SetRenderDrawColor(SDL_Renderer* renderer, const SDL_Color& color) {
	return SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
}

void SDL_DrawThickLine(SDL_Renderer* renderer, float x1, float y1, float x2, float y2, float thickness, const SDL_Color& color) {
	float dx = x2 - x1;
	float dy = y2 - y1;
	float length = sqrtf(dx * dx + dy * dy);

	if (length == 0) return; // Avoid div by zero

	// Normalize perpendicular vector
	float perpX = -(dy / length) * (thickness / 2.0f);
	float perpY = (dx / length) * (thickness / 2.0f);

	SDL_Vertex verts[4];

	verts[0].position.x = x1 + perpX;
	verts[0].position.y = y1 + perpY;

	verts[1].position.x = x2 + perpX;
	verts[1].position.y = y2 + perpY;

	verts[2].position.x = x2 - perpX;
	verts[2].position.y = y2 - perpY;

	verts[3].position.x = x1 - perpX;
	verts[3].position.y = y1 - perpY;

	// Set color (no texture, so ignore tex coords)
	for (int i = 0; i < 4; i++) {
		verts[i].color.r = ((float)color.r) / 255.f;
		verts[i].color.g = ((float)color.g) / 255.f;
		verts[i].color.b = ((float)color.b) / 255.f;
		verts[i].color.a = ((float)color.a) / 255.f;
		verts[i].tex_coord.x = 0.0f;
		verts[i].tex_coord.y = 0.0f;
	}

	int indices[6] = { 0, 1, 2, 2, 3, 0 };

	SDL_RenderGeometry(renderer, NULL, verts, 4, indices, 6);
}

SdlRenderer::SdlRenderer(SDL_Renderer* sdlRenderer) : sdlRenderer(sdlRenderer), w(0), h(0), circuit(nullptr), tileSetInfo(nullptr) { }

void SdlRenderer::initializeTileSet(const std::string& filePath) {
	if (filePath != "") {
		tileSet = IMG_LoadTexture(sdlRenderer, filePath.c_str());
		SDL_SetTextureBlendMode(tileSet, SDL_BLENDMODE_BLEND);

		if (!tileSet) {
			logError("TileSet image could not be loaded from file: {}", "SdlRenderer", filePath);
		}

		// create tileSet
		tileSetInfo = std::make_unique<TileSetInfo>(256, 15);
	}
}

void SdlRenderer::resize(int w, int h) {
	this->w = w;
	this->h = h;
}

void SdlRenderer::reposition(int x, int y) {
	this->x = x;
	this->y = y;
}

void SdlRenderer::setCircuit(Circuit* circuit) {
	this->circuit = circuit;
}

void SdlRenderer::setEvaluator(Evaluator* evaluator) {
	this->evaluator = evaluator;
}

void SdlRenderer::updateView(ViewManager* viewManager) {
	this->viewManager = viewManager;
}

void SdlRenderer::updateCircuit(DifferenceSharedPtr diff) {

}

void SdlRenderer::render() {
	if (!circuit) {
		SDL_SetRenderDrawColor(sdlRenderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
		SDL_RenderLine(sdlRenderer, x, y, x + w, y + h);
		SDL_RenderLine(sdlRenderer, x + w, y, x, y + h);
		return;
	}

	// error checking
	if (!viewManager) {
		SDL_SetRenderDrawColor(sdlRenderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
		SDL_RenderLine(sdlRenderer, x, y, x + w, y + h);
		SDL_RenderLine(sdlRenderer, x + w, y, x, y + h);
		logError("has no viewManager, can not proceed with render.", "SdlRenderer");
		return;
	}

	if (!tileSet || !tileSetInfo) {
		SDL_SetRenderDrawColor(sdlRenderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
		SDL_RenderLine(sdlRenderer, x, y, x + w, y + h);
		SDL_RenderLine(sdlRenderer, x + w, y, x, y + h);
		logError("has no tileSet, can not proceed with render.", "SdlRenderer");
		return;
	}

	// QElapsedTimer timer;
	// timer.start();

	Vec2Int emptyTilePoint = tileSetInfo->getTopLeftPixel(0, logic_state_t::LOW);
	Vec2Int emptyTileSize = tileSetInfo->getCellPixelSize();
	SDL_FRect emptyTileSetRect;
	emptyTileSetRect.x = emptyTilePoint.x;
	emptyTileSetRect.y = emptyTilePoint.y;
	emptyTileSetRect.w = emptyTileSize.x;
	emptyTileSetRect.h = emptyTileSize.y;

	// get bounds
	Position topLeftBound = viewManager->getTopLeft().snap();
	Position bottomRightBound = viewManager->getBottomRight().snap();

	if (evaluator) {
		// get states
		std::vector<Address> blockAddresses;
		std::vector<const Block*> blocks;
		for (const auto& block : *(circuit->getBlockContainer())) {
			if (areaWithinArea(block.second.getPosition(), block.second.getLargestPosition(), topLeftBound, bottomRightBound)) {
				blockAddresses.push_back(Address(block.second.getPosition()));
				blocks.push_back(&(block.second));
			}
		}
		std::vector<logic_state_t> blockStates = evaluator->getBulkStates(blockAddresses);

		// render grid
		for (int x = topLeftBound.x; x <= bottomRightBound.x; ++x) {
			for (int y = topLeftBound.y; y <= bottomRightBound.y; ++y) {
				SDL_FPoint point = gridToSDL(FPosition(x, y));
				SDL_FPoint pointBR = gridToSDL(FPosition(x + 1, y + 1));
				SDL_FRect dstrect;
				dstrect.x = point.x + this->x;
				dstrect.y = point.y + this->y;
				dstrect.w = pointBR.x - point.x;
				dstrect.h = pointBR.y - point.y;
				SDL_RenderTexture(sdlRenderer, tileSet, &emptyTileSetRect, &dstrect);
			}
		}
		// render blocks
		// sdlRenderer->setRenderHint(SDL_Renderer::SmoothPixmapTransform);
		for (unsigned int i = 0; i < blocks.size(); i++) {
			renderBlock(blocks[i]->type(), blocks[i]->getPosition(), blocks[i]->getRotation(), blockStates[i]);
		}

		// render block previews
		SDL_SetTextureAlphaMod(tileSet, 80);
		for (const auto& preview : blockPreviews) {
			renderBlock(preview.second.type, preview.second.position, preview.second.rotation, logic_state_t::LOW);
		}
		SDL_SetTextureAlphaMod(tileSet, 255);

		// render connections
		for (unsigned int i = 0; i < blocks.size(); i++) {
			logic_state_t state = blockStates[i];
			for (connection_end_id_t id = 0; id < blocks[i]->getConnectionContainer().getConnectionCount(); id++) {
				// continue if input, we only want outputs
				if (blocks[i]->isConnectionInput(id)) continue;

				Position pos = blocks[i]->getConnectionPosition(id).first;
				for (auto connectionIter : blocks[i]->getConnectionContainer().getConnections(id)) {
					const Block* other = circuit->getBlockContainer()->getBlock(connectionIter.getBlockId());
					Position otherPos = other->getConnectionPosition(connectionIter.getConnectionId()).first;
					if (
						(pos.x + 2 > topLeftBound.x || otherPos.x + 2 > topLeftBound.x) &&
						(pos.y + 2 > topLeftBound.y || otherPos.y + 2 > topLeftBound.y) &&
						(pos.x - 2 < bottomRightBound.x || otherPos.x - 2 < bottomRightBound.x) &&
						(pos.y - 2 < bottomRightBound.y || otherPos.y - 2 < bottomRightBound.y)
					) {
						renderConnection(pos, blocks[i], otherPos, other, state);
					}
				}
			}
		}
		// render connection previews
		for (const auto& preview : connectionPreviews) {
			renderConnection(preview.second.input, preview.second.output, logic_state_t::LOW);
		}
		// render half connection previews
		for (const auto& preview : halfConnectionPreviews) {
			renderConnection(preview.second.input, preview.second.output, logic_state_t::LOW);
		}
	} else {
		// render grid
		for (int x = topLeftBound.x; x <= bottomRightBound.x; ++x) {
			for (int y = topLeftBound.y; y <= bottomRightBound.y; ++y) {
				SDL_FPoint point = gridToSDL(FPosition(x, y));
				SDL_FPoint pointBR = gridToSDL(FPosition(x + 1, y + 1));
				SDL_FRect dstrect;
				dstrect.x = point.x + this->x;
				dstrect.y = point.y + this->y;
				dstrect.w = pointBR.x - point.x;
				dstrect.h = pointBR.y - point.y;
				SDL_RenderTexture(sdlRenderer, tileSet, &emptyTileSetRect, &dstrect);
			}
		}

		// sdlRenderer->setRenderHint(SDL_Renderer::SmoothPixmapTransform);
		std::vector<const Block*> blocks;
		for (const auto& block : *(circuit->getBlockContainer())) {
			if (areaWithinArea(block.second.getPosition(), block.second.getLargestPosition(), topLeftBound, bottomRightBound)) {
				renderBlock(block.second.type(), block.second.getPosition(), block.second.getRotation(), logic_state_t::UNDEFINED);
			}
			blocks.push_back(&(block.second));
		}

		// render block previews
		SDL_SetTextureAlphaMod(tileSet, 80);
		for (const auto& preview : blockPreviews) {
			renderBlock(preview.second.type, preview.second.position, preview.second.rotation, logic_state_t::LOW);
		}
		SDL_SetTextureAlphaMod(tileSet, 255);

		// render connections
		for (unsigned int i = 0; i < blocks.size(); i++) {
			for (connection_end_id_t id = 0; id < blocks[i]->getConnectionContainer().getConnectionCount(); id++) {
				// continue if input, we only want outputs
				if (blocks[i]->isConnectionInput(id)) continue;

				Position pos = blocks[i]->getConnectionPosition(id).first;
				for (auto connectionIter : blocks[i]->getConnectionContainer().getConnections(id)) {
					const Block* other = circuit->getBlockContainer()->getBlock(connectionIter.getBlockId());
					Position otherPos = other->getConnectionPosition(connectionIter.getConnectionId()).first;
					if (
						(pos.x + 2 > topLeftBound.x || otherPos.x + 2 > topLeftBound.x) &&
						(pos.y + 2 > topLeftBound.y || otherPos.y + 2 > topLeftBound.y) &&
						(pos.x - 2 < bottomRightBound.x || otherPos.x - 2 < bottomRightBound.x) &&
						(pos.y - 2 < bottomRightBound.y || otherPos.y - 2 < bottomRightBound.y)
					) {
						renderConnection(pos, blocks[i], otherPos, other, logic_state_t::UNDEFINED);
					}
				}
			}
		}
		// render connection previews
		for (const auto& preview : connectionPreviews) {
			renderConnection(preview.second.input, preview.second.output, logic_state_t::UNDEFINED);
		}
		// render half connection previews
		for (const auto& preview : halfConnectionPreviews) {
			renderConnection(preview.second.input, preview.second.output, logic_state_t::UNDEFINED);
		}
	}

	// render selections
	// normal selection
	SDL_Color transparentBlue(0, 0, 255, 50);
	SDL_SetRenderDrawColor(sdlRenderer, transparentBlue);
	for (const auto& selection : selectionElements) {
		SDL_FPoint topLeft = gridToSDL(selection.second.topLeft.free());
		SDL_FPoint size = gridToSDL((selection.second.bottomRight - selection.second.topLeft + Vector(1.0f, 1.0f)).free());
		SDL_FRect rect(topLeft.x + x, topLeft.y + y, size.x, size.y);
		SDL_RenderFillRect(sdlRenderer, &rect);
	}
	// inverted selections
	SDL_Color transparentRed(255, 0, 0, 50);
	SDL_SetRenderDrawColor(sdlRenderer, transparentRed);
	for (const auto& selection : invertedSelectionElements) {
		SDL_FPoint topLeft = gridToSDL(selection.second.topLeft.free());
		SDL_FPoint size = gridToSDL((selection.second.bottomRight - selection.second.topLeft + Vector(1.0f, 1.0f)).free());
		SDL_FRect rect(topLeft.x + x, topLeft.y + y, size.x, size.y);
		SDL_RenderFillRect(sdlRenderer, &rect);
	}
	// selection object
	for (const auto selection : selectionObjectElements) {
		renderSelection(selection.second.selection, selection.second.renderMode);
	}

	// for (int x = topLeftBound.x; x <= bottomRightBound.x; ++x) {
	// 	for (int y = topLeftBound.y; y <= bottomRightBound.y; ++y) {
	// 		drawText(gridToSDL(FPosition(x, y)+FVector(0.3f, 0.16f)), std::string::fromStdString(Position(x, y).toString()), 15, SDL_Color("#97A9E1"));
	// 	}
	// }
	// lastFrameTime = timer.nsecsElapsed() / 1e6f;
}

const SDL_Color arrowColorOrder[] = {
	SDL_Color(255, 0, 0, 180),
	SDL_Color(0, 255, 0, 180),
	SDL_Color(0, 0, 255, 180),
	SDL_Color(255, 255, 0, 180),
	SDL_Color(255, 0, 255, 180),
	SDL_Color(0, 255, 255, 180),
	SDL_Color(255, 255, 255, 180),
	SDL_Color(127, 0, 0, 180),
	SDL_Color(0, 127, 0, 180),
	SDL_Color(0, 0, 127, 180),
	SDL_Color(127, 127, 0, 180),
	SDL_Color(127, 0, 127, 180),
	SDL_Color(0, 127, 127, 180),
	SDL_Color(127, 127, 127, 180),
	SDL_Color(255, 127, 0, 180),
	SDL_Color(255, 0, 127, 180),
	SDL_Color(0, 255, 127, 180),
	SDL_Color(127, 255, 0, 180),
	SDL_Color(127, 0, 255, 180),
	SDL_Color(0, 127, 255, 180),
	SDL_Color(255, 127, 127, 180),
	SDL_Color(127, 255, 127, 180),
	SDL_Color(127, 127, 255, 180),
	SDL_Color(255, 255, 127, 180),
	SDL_Color(255, 127, 255, 180),
	SDL_Color(127, 255, 255, 180)
};

void SdlRenderer::renderSelection(const SharedSelection selection, SelectionObjectElement::RenderMode mode, unsigned int depth) {
	switch (mode) {
	case SelectionObjectElement::RenderMode::SELECTION:
	{
		SharedCellSelection cellSelection = selectionCast<CellSelection>(selection);
		if (cellSelection) {
			SDL_FPoint topLeft = gridToSDL(cellSelection->getPosition().free());
			SDL_FPoint size = gridToSDL((cellSelection->getPosition() - cellSelection->getPosition() + Vector(1)).free());
			SDL_SetRenderDrawColor(sdlRenderer, SDL_Color(0, 0, 255, 64));
			SDL_FRect rect(topLeft.x + x, topLeft.y + y, size.x, size.y);
			SDL_RenderFillRect(sdlRenderer, &rect);
			return;
		}
		SharedDimensionalSelection dimensionalSelection = selectionCast<DimensionalSelection>(selection);
		if (dimensionalSelection) {
			for (int i = 0; i < dimensionalSelection->size(); i++) {
				renderSelection(dimensionalSelection->getSelection(i), mode, depth + 1);
			}
			return;
		}
		return;
	}
	case SelectionObjectElement::RenderMode::SELECTION_INVERTED:
	{
		SharedCellSelection cellSelection = selectionCast<CellSelection>(selection);
		if (cellSelection) {
			SDL_FPoint topLeft = gridToSDL(cellSelection->getPosition().free());
			SDL_FPoint size = gridToSDL((cellSelection->getPosition() - cellSelection->getPosition() + Vector(1)).free());
			SDL_SetRenderDrawColor(sdlRenderer, SDL_Color(255, 0, 0, 64));
			SDL_FRect rect(topLeft.x + x, topLeft.y + y, size.x, size.y);
			SDL_RenderFillRect(sdlRenderer, &rect);
		}
		SharedDimensionalSelection dimensionalSelection = selectionCast<DimensionalSelection>(selection);
		if (dimensionalSelection) {
			for (int i = 0; i < dimensionalSelection->size(); i++) {
				renderSelection(dimensionalSelection->getSelection(i), mode, depth + 1);
			}
			return;
		}
		return;
	}
	case SelectionObjectElement::RenderMode::ARROWS:
	{
		SharedCellSelection cellSelection = selectionCast<CellSelection>(selection);
		if (cellSelection) {
			SDL_FPoint topLeft = gridToSDL(cellSelection->getPosition().free());
			SDL_FPoint size = gridToSDL((cellSelection->getPosition() - cellSelection->getPosition() + Vector(1)).free());
			SDL_SetRenderDrawColor(sdlRenderer, SDL_Color(255, 0, 0255, 64));
			SDL_FRect rect(topLeft.x + x, topLeft.y + y, size.x, size.y);
			SDL_RenderFillRect(sdlRenderer, &rect);
			// sdlRenderer->setBrush(SDL_Color(0, 0, 255, 64));
			// sdlRenderer->drawRect(QRectF(gridToSDL(cellSelection->getPosition().free()), gridToSDL((cellSelection->getPosition() + Vector(1)).free())));
			return;
		}
		SharedDimensionalSelection dimensionalSelection = selectionCast<DimensionalSelection>(selection);
		if (dimensionalSelection) {
			SharedProjectionSelection projectionSelection = selectionCast<ProjectionSelection>(selection);
			if (projectionSelection) {
				SharedDimensionalSelection dSel = dimensionalSelection;
				Position orgin;
				unsigned int height = 0;
				while (dSel) {
					SharedSelection sel = dSel->getSelection(0);
					SharedCellSelection cSel = selectionCast<CellSelection>(sel);
					if (cSel) {
						orgin = cSel->getPosition();
						break;
					}
					height++;
					dSel = selectionCast<DimensionalSelection>(sel);
				}
				for (int i = 1; i < projectionSelection->size(); i++) {
					SDL_FPoint start = gridToSDL(orgin.free() + FVector(0.5f, 0.5f));
					orgin += projectionSelection->getStep();
					SDL_FPoint end = gridToSDL(orgin.free() + FVector(0.5f, 0.5f));
					drawArrow(start, end, 16.0f, arrowColorOrder[height % 26]);
				}
				renderSelection(dimensionalSelection->getSelection(0), mode, depth + 1);
			} else {
				for (int i = 0; i < dimensionalSelection->size(); i++) {
					renderSelection(dimensionalSelection->getSelection(i), mode, depth + 1);
				}
			}
			return;
		}
		return;
	}
	default:
		break;
	}
}

void SdlRenderer::renderBlock(BlockType type, Position position, Rotation rotation, logic_state_t state) {
	// block
	Vector blockSize(circuit->getBlockContainer()->getBlockDataManager()->getBlockSize(type));

	Vector blockOriginOffset = rotateVectorWithArea(
		Vector(0),
		blockSize,
		rotation
	);

	FVector shift;
	switch (rotation) {
	case Rotation::ZERO:
		break;
	case Rotation::NINETY:
		shift.dx = -0.5f;
		shift.dy = -0.5f;
		break;
	case Rotation::ONE_EIGHTY:
		shift.dx = 0;
		shift.dy = -1;
		break;
	case Rotation::TWO_SEVENTY:
		shift.dx = 0.5f;
		shift.dy = -0.5f;
		break;
	}

	SDL_FPoint size = gridToSDL(blockSize.free());
	SDL_FPoint rotationPoint = gridToSDL((position + blockOriginOffset).free() + shift);
	// get tile set coordinate
	if (type > 13) {
		type = BlockType::LIGHT;
	}
	Vec2Int tilePoint = tileSetInfo->getTopLeftPixel(type + 1, state);
	Vec2Int tileSize = tileSetInfo->getCellPixelSize();

	SDL_FRect tileSetRect;
	tileSetRect.x = tilePoint.x;
	tileSetRect.y = tilePoint.y;
	tileSetRect.w = tileSize.x;
	tileSetRect.h = tileSize.y;


	SDL_FRect dstrect;
	dstrect.x = rotationPoint.x + x;
	dstrect.y = rotationPoint.y + y;
	dstrect.w = size.x;
	dstrect.h = size.y;
	SDL_RenderTextureRotated(sdlRenderer, tileSet, &tileSetRect, &dstrect, getDegrees(rotation), nullptr, SDL_FLIP_NONE);


	// // rotate and position sdlRenderer to center of block
	// sdlRenderer->translate(rotationPoint);
	// sdlRenderer->rotate(getDegrees(rotation));

	// // draw the block from the center
	// sdlRenderer->drawPixmap(drawRect, tileSet, tileSetRect);

	// // undo transformations
	// sdlRenderer->rotate(-getDegrees(rotation));
	// sdlRenderer->translate(-rotationPoint);
}

SDL_Color SdlRenderer::getStateColor(logic_state_t state) {
	switch (state) {
	case logic_state_t::LOW:
		return SDL_Color(151, 169, 225, 229);
	case logic_state_t::HIGH:
		return SDL_Color(143, 233, 127, 229);
	case logic_state_t::FLOATING:
		return SDL_Color(176, 164, 255, 229);
	default:
		return SDL_Color(255, 167, 164, 229);
	}
}

void SdlRenderer::renderConnection(FPosition aPos, FPosition bPos, FVector aControlOffset, FVector bControlOffset, logic_state_t state) {

	// sdlRenderer->setPen(QPen(getStateColor(state), scalePixelCount(30.0f), Qt::SolidLine, Qt::RoundCap));
	// SDL_SetRenderDrawColor(sdlRenderer, );

	SDL_FPoint start = gridToSDL(aPos);
	SDL_FPoint end = gridToSDL(bPos);

	// SDL_FPoint c1 = (abs(aControlOffset.dx) > abs(aControlOffset.dy)) ? gridToSDL(aPos + FVector(aControlOffset.dx * 1.3f, 0)) : gridToSDL(aPos + FVector(0, aControlOffset.dy * 1.3f));
	// SDL_FPoint c2 = (abs(bControlOffset.dx) > abs(bControlOffset.dy)) ? gridToSDL(bPos + FVector(bControlOffset.dx * 1.3f, 0)) : gridToSDL(bPos + FVector(0, bControlOffset.dy * 1.3f));


	// sdlRenderer->drawLine(start, end);

	SDL_DrawThickLine(sdlRenderer, start.x + x, start.y + y, end.x + x, end.y + y, scalePixelCount(30.f), getStateColor(state));

	// SDL_RendererPath myPath;
	// myPath.moveTo(start);
	// myPath.cubicTo(c1, c2, end);
	// sdlRenderer->drawPath(myPath);
}

const float edgeDis = 0.48f;
const float sideShift = 0.25f;

void SdlRenderer::renderConnection(Position aPos, const Block* a, Position bPos, const Block* b, logic_state_t state) {
	FVector centerOffset(0.5f, 0.5f);

	if (a == b) {
		drawText(gridToSDL(aPos.free() + centerOffset), "S", 30, getStateColor(state));
		return;
	}

	FVector aSocketOffset(0.0f, 0.0f);
	FVector bSocketOffset(0.0f, 0.0f);

	if (a) {
		switch (a->getRotation()) {
		case Rotation::ZERO: aSocketOffset = { edgeDis, sideShift }; break;
		case Rotation::NINETY: aSocketOffset = { -sideShift, edgeDis }; break;
		case Rotation::ONE_EIGHTY: aSocketOffset = { -edgeDis, -sideShift }; break;
		case Rotation::TWO_SEVENTY: aSocketOffset = { sideShift, -edgeDis }; break;
		}
	}

	if (b) {
		switch (b->getRotation()) {
		case Rotation::ZERO: bSocketOffset = { -edgeDis, -sideShift }; break;
		case Rotation::NINETY: bSocketOffset = { sideShift, -edgeDis }; break;
		case Rotation::ONE_EIGHTY: bSocketOffset = { edgeDis, sideShift }; break;
		case Rotation::TWO_SEVENTY: bSocketOffset = { -sideShift, edgeDis }; break;
		}
	}

	renderConnection(aPos.free() + centerOffset + aSocketOffset, bPos.free() + centerOffset + bSocketOffset, aSocketOffset, bSocketOffset, state);
}

void SdlRenderer::renderConnection(Position aPos, Position bPos, logic_state_t state) {
	// Socket offsets will be retrieved data later, this code will go
	const Block* a = circuit->getBlockContainer()->getBlock(aPos);
	const Block* b = circuit->getBlockContainer()->getBlock(bPos);

	renderConnection(aPos, a, bPos, b, state);
}

void SdlRenderer::renderConnection(Position aPos, FPosition bPos, logic_state_t state) {
	FVector centerOffset(0.5f, 0.5f);
	FVector aSocketOffset(0.0f, 0.0f);

	// Socket offsets will be retrieved data later, this code will go
	const Block* a = circuit->getBlockContainer()->getBlock(aPos);

	if (a) {
		switch (a->getRotation()) {
		case Rotation::ZERO: aSocketOffset = { edgeDis, sideShift }; break;
		case Rotation::NINETY: aSocketOffset = { -sideShift, edgeDis }; break;
		case Rotation::ONE_EIGHTY: aSocketOffset = { -edgeDis, -sideShift }; break;
		case Rotation::TWO_SEVENTY: aSocketOffset = { sideShift, -edgeDis }; break;
		}
	}

	renderConnection(aPos.free() + centerOffset + aSocketOffset, bPos, aSocketOffset, FVector(0.0f, 0.0f), state);
}

SDL_FPoint SdlRenderer::gridToSDL(FPosition position) {
	assert(viewManager);

	Vec2 viewPos = viewManager->gridToView(position);
	return SDL_FPoint(viewPos.x * w, viewPos.y * h);
}

SDL_FPoint SdlRenderer::gridToSDL(FVector vector) {
	assert(viewManager);

	Vec2 viewPos = viewManager->gridToView(vector);
	return SDL_FPoint(viewPos.x * w, viewPos.y * h);
}

// element -----------------------------

// selection
ElementID SdlRenderer::addSelectionElement(const SelectionElement& selection) {
	ElementID newID = currentID++;

	Position topLeft = selection.topLeft;
	Position bottomRight = selection.bottomRight;

	// fix coordinates if incorrect
	if (topLeft.x > bottomRight.x) {
		int temp = topLeft.x;
		topLeft.x = bottomRight.x;
		bottomRight.x = temp;
	}
	if (topLeft.y > bottomRight.y) {
		int temp = topLeft.y;
		topLeft.y = bottomRight.y;
		bottomRight.y = temp;
	}

	// add to lists
	if (!selection.inverted) selectionElements[newID] = { topLeft, bottomRight, selection.inverted };
	else invertedSelectionElements[newID] = { topLeft, bottomRight, selection.inverted };

	return newID;
}

ElementID SdlRenderer::addSelectionElement(const SelectionObjectElement& selection) {
	ElementID newID = currentID++;
	selectionObjectElements.emplace(newID, selection);
	return newID;
}

void SdlRenderer::removeSelectionElement(ElementID selection) {
	selectionObjectElements.erase(selection);
	selectionElements.erase(selection);
	invertedSelectionElements.erase(selection);
}

// block preview
ElementID SdlRenderer::addBlockPreview(const BlockPreview& blockPreview) {
	ElementID newID = currentID++;
	blockPreviews[newID] = blockPreview;
	return newID;
}

void SdlRenderer::removeBlockPreview(ElementID blockPreview) {
	blockPreviews.erase(blockPreview);
}

// connection preview
ElementID SdlRenderer::addConnectionPreview(const ConnectionPreview& connectionPreview) {
	ElementID newID = currentID++;

	connectionPreviews[newID] = { connectionPreview.input, connectionPreview.output };

	return newID;
}

void SdlRenderer::removeConnectionPreview(ElementID connectionPreview) {
	connectionPreviews.erase(connectionPreview);
}

// half connection preview
ElementID SdlRenderer::addHalfConnectionPreview(const HalfConnectionPreview& halfConnectionPreview) {
	ElementID newID = currentID++;

	halfConnectionPreviews[newID] = { halfConnectionPreview.input, halfConnectionPreview.output };

	return newID;
}

void SdlRenderer::removeHalfConnectionPreview(ElementID halfConnectionPreview) {
	halfConnectionPreviews.erase(halfConnectionPreview);
}

// confetti
void SdlRenderer::spawnConfetti(FPosition start) {

}


// helpers
void SdlRenderer::drawArrow(const SDL_FPoint& start, const SDL_FPoint& end, float size, const SDL_Color& color) {
	// // Draw main line
	// size = scalePixelCount(size);
	// sdlRenderer->save();
	// sdlRenderer->setPen(QPen(color, 3.f * size, Qt::SolidLine, Qt::RoundCap));
	// auto vec = QVector2D(start - end);
	// vec.normalize();
	// sdlRenderer->drawLine(start, end + vec.toPointF() * size * 10);
	// sdlRenderer->restore();
	// sdlRenderer->setBrush(color);

	// // Draw arrowhead
	// QLineF line(start, end);
	// double angle = -line.angle() + 180;

	// // Calculate arrowhead points
	// double arrow_size = 10 * size;
	// SDL_FPoint p1 = line.p2();
	// SDL_FPoint arrow_p1(
	// 	p1.x() - arrow_size * std::cos((angle + 150) * M_PI / 180),
	// 	p1.y() - arrow_size * std::sin((angle + 150) * M_PI / 180)
	// );

	// SDL_FPoint arrow_p2(
	// 	p1.x() - arrow_size * std::cos((angle - 150) * M_PI / 180),
	// 	p1.y() - arrow_size * std::sin((angle - 150) * M_PI / 180)
	// );

	// // Draw arrowhead as a polygon
	// QPolygonF arrow_head;
	// arrow_head << p1 << arrow_p1 << arrow_p2;
	// sdlRenderer->drawPolygon(arrow_head);
}

void SdlRenderer::drawText(const SDL_FPoint& center, const std::string& text, float size, const SDL_Color& color) {
	// if (scalePixelCount(size * 4) <= 1) return;
	// sdlRenderer->setPen(QPen(color));
	// QFont font = sdlRenderer->font();
	// font.setPixelSize(scalePixelCount(size * 4));
	// sdlRenderer->setFont(font);
	// sdlRenderer->drawText(QRectF(center + SDL_FPoint(-100, -100), center + SDL_FPoint(100, 100)), text, QTextOption(Qt::AlignCenter));
}