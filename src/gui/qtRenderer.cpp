#include <QElapsedTimer>
#include <QPainterPath>
#include <QDateTime>
#include <QVector2D>
#include <QString>
#include <QDebug>

#include "backend/circuitView/renderer/renderer.h"
#include "backend/evaluator/logicState.h"
#include "backend/address.h"
#include "qtRenderer.h"
#include "util/vec2.h"

QtRenderer::QtRenderer()
	: w(0), h(0), circuit(nullptr), tileSetInfo(nullptr) { }

void QtRenderer::initializeTileSet(const std::string& filePath) {
	if (filePath != "") {
		tileSet = QPixmap(filePath.c_str());

		if (tileSet.isNull()) {
			qDebug() << "ERROR: tileSet image could not be loaded from file." << QString::fromStdString(filePath);
		}

		// create tileSet
		tileSetInfo = std::make_unique<TileSetInfo>(256, 15);
	}
}

void QtRenderer::resize(int w, int h) {
	this->w = w;
	this->h = h;
}

void QtRenderer::setCircuit(Circuit* circuit) {
	this->circuit = circuit;
}

void QtRenderer::setEvaluator(Evaluator* evaluator) {
	this->evaluator = evaluator;
}

void QtRenderer::updateView(ViewManager* viewManager) {
	this->viewManager = viewManager;
}

void QtRenderer::updateCircuit(DifferenceSharedPtr diff) {

}

void QtRenderer::render(QPainter* painter) {
	if (!circuit) {
		painter->drawText(QRect(0, 0, w, h), Qt::AlignCenter, "No circuit set");
		return;
	}

	// error checking
	assert(viewManager);
	if (tileSet.isNull() || tileSetInfo == nullptr) {
		painter->drawText(QRect(0, 0, w, h), Qt::AlignCenter, "No tileSet found");
		// qDebug() << "ERROR: QTRenderer has no tileSet, can not proceed with render.";
		return;
	}

	QElapsedTimer timer;
	timer.start();

	// --- end of render lambdas

	Vec2Int emptyTilePoint = tileSetInfo->getTopLeftPixel(0, logic_state_t::LOW);
	Vec2Int emptyTileSize = tileSetInfo->getCellPixelSize();
	QRectF emptyTileSetRect(QPointF(emptyTilePoint.x, emptyTilePoint.y), QSizeF(emptyTileSize.x, emptyTileSize.y));

	// get bounds
	Position topLeftBound = viewManager->getTopLeft().snap();
	Position bottomRightBound = viewManager->getBottomRight().snap();

	if (evaluator) {
		// get states
		std::vector<Address> blockAddresses;
		std::vector<const Block*> blocks;
		for (const auto& block : *(circuit->getBlockContainer())) {
			blockAddresses.push_back(Address(block.second.getPosition()));
			blocks.push_back(&(block.second));
		}
		std::vector<logic_state_t> blockStates = evaluator->getBulkStates(blockAddresses);

		// render grid
		for (int x = topLeftBound.x; x <= bottomRightBound.x; ++x) {
			for (int y = topLeftBound.y; y <= bottomRightBound.y; ++y) {
				QPointF point = gridToQt(FPosition(x, y));
				QPointF pointBR = gridToQt(FPosition(x + 1, y + 1));
				painter->drawPixmap(QRectF(point, pointBR), tileSet, emptyTileSetRect);
			}
		}
		// render blocks
		painter->setRenderHint(QPainter::SmoothPixmapTransform);
		for (unsigned int i = 0; i < blocks.size(); i++) {
			if (blocks[i]->getPosition().withinArea(topLeftBound, bottomRightBound) || blocks[i]->getLargestPosition().withinArea(topLeftBound, bottomRightBound)) {
				renderBlock(painter, blocks[i]->type(), blocks[i]->getPosition(), blocks[i]->getRotation(), blockStates[i]);
			}
		}

		// render block previews
		painter->setOpacity(0.4f);
		for (const auto& preview : blockPreviews) {
			renderBlock(painter, preview.second.type, preview.second.position, preview.second.rotation, logic_state_t::LOW);
		}
		painter->setOpacity(1.0f);
		painter->setRenderHint(QPainter::SmoothPixmapTransform, false);

		// render connections
		painter->save();
		painter->setOpacity(0.9f);
		// painter->setRenderHint(QPainter::Antialiasing);
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
						renderConnection(painter, pos, blocks[i], otherPos, other, state);
					}
				}
			}
		}
		// render connection previews
		for (const auto& preview : connectionPreviews) {
			renderConnection(painter, preview.second.input, preview.second.output, logic_state_t::LOW);
		}
		// render half connection previews
		for (const auto& preview : halfConnectionPreviews) {
			renderConnection(painter, preview.second.input, preview.second.output, logic_state_t::LOW);
		}
		painter->restore();
	} else {
		// render grid
		for (int x = topLeftBound.x; x <= bottomRightBound.x; ++x) {
			for (int y = topLeftBound.y; y <= bottomRightBound.y; ++y) {
				QPointF point = gridToQt(FPosition(x, y));
				QPointF pointBR = gridToQt(FPosition(x + 1, y + 1));
				painter->drawPixmap(QRectF(point, pointBR), tileSet, emptyTileSetRect);
			}
		}

		painter->setRenderHint(QPainter::SmoothPixmapTransform);
		std::vector<const Block*> blocks;
		for (const auto& block : *(circuit->getBlockContainer())) {
			if (block.second.getPosition().withinArea(topLeftBound, bottomRightBound) || block.second.getLargestPosition().withinArea(topLeftBound, bottomRightBound)) {
				renderBlock(painter, block.second.type(), block.second.getPosition(), block.second.getRotation(), logic_state_t::UNDEFINED);
			}
			blocks.push_back(&(block.second));
		}

		// render block previews
		painter->setOpacity(0.4f);
		for (const auto& preview : blockPreviews) {
			renderBlock(painter, preview.second.type, preview.second.position, preview.second.rotation, logic_state_t::UNDEFINED);
		}
		painter->setOpacity(1.0f);
		painter->setRenderHint(QPainter::SmoothPixmapTransform, false);

		// render connections
		painter->save();
		painter->setOpacity(0.9f);
		painter->setRenderHint(QPainter::Antialiasing);
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
						renderConnection(painter, pos, blocks[i], otherPos, other, logic_state_t::UNDEFINED);
					}
				}
			}
		}
		// render connection previews
		for (const auto& preview : connectionPreviews) {
			renderConnection(painter, preview.second.input, preview.second.output, logic_state_t::UNDEFINED);
		}
		// render half connection previews
		for (const auto& preview : halfConnectionPreviews) {
			renderConnection(painter, preview.second.input, preview.second.output, logic_state_t::UNDEFINED);
		}
		painter->restore();
	}

	// render selections
	painter->save();
	painter->setPen(Qt::NoPen);
	// normal selection
	QColor transparentBlue(0, 0, 255, 64);
	painter->setBrush(transparentBlue);
	for (const auto& selection : selectionElements) {
		FPosition topLeft = selection.second.topLeft.free();
		FPosition bottomRight = selection.second.bottomRight.free() + FVector(1.0f, 1.0f);
		painter->drawRect(QRectF(gridToQt(topLeft), gridToQt(bottomRight)));
	}
	// inverted selections
	QColor transparentRed(255, 0, 0, 64);
	painter->setBrush(transparentRed);
	for (const auto& selection : invertedSelectionElements) {
		FPosition topLeft = selection.second.topLeft.free();
		FPosition bottomRight = selection.second.bottomRight.free() + FVector(1.0f, 1.0f);
		painter->drawRect(QRectF(gridToQt(topLeft), gridToQt(bottomRight)));
	}
	// selection object
	for (const auto selection : selectionObjectElements) {
		renderSelection(painter, selection.second.selection, selection.second.renderMode);
	}
	painter->restore();
	// painter->save();
	// for (int x = topLeftBound.x; x <= bottomRightBound.x; ++x) {
	// 	for (int y = topLeftBound.y; y <= bottomRightBound.y; ++y) {
	// 		drawText(painter, gridToQt(FPosition(x, y)+FVector(0.3f, 0.16f)), QString::fromStdString(Position(x, y).toString()), 15, QColor("#97A9E1"));
	// 	}
	// }
	// painter->restore();

	lastFrameTime = timer.nsecsElapsed() / 1e6f;
}

const QColor arrowColorOrder[] = {
	QColor(255, 0, 0, 180),
	QColor(0, 255, 0, 180),
	QColor(0, 0, 255, 180),
	QColor(255, 255, 0, 180),
	QColor(255, 0, 255, 180),
	QColor(0, 255, 255, 180),
	QColor(255, 255, 255, 180),
	QColor(127, 0, 0, 180),
	QColor(0, 127, 0, 180),
	QColor(0, 0, 127, 180),
	QColor(127, 127, 0, 180),
	QColor(127, 0, 127, 180),
	QColor(0, 127, 127, 180),
	QColor(127, 127, 127, 180),
	QColor(255, 127, 0, 180),
	QColor(255, 0, 127, 180),
	QColor(0, 255, 127, 180),
	QColor(127, 255, 0, 180),
	QColor(127, 0, 255, 180),
	QColor(0, 127, 255, 180),
	QColor(255, 127, 127, 180),
	QColor(127, 255, 127, 180),
	QColor(127, 127, 255, 180),
	QColor(255, 255, 127, 180),
	QColor(255, 127, 255, 180),
	QColor(127, 255, 255, 180)
};

void QtRenderer::renderSelection(QPainter* painter, const SharedSelection selection, SelectionObjectElement::RenderMode mode, unsigned int depth) {
	switch (mode) {
	case SelectionObjectElement::RenderMode::SELECTION:
	{
		SharedCellSelection cellSelection = selectionCast<CellSelection>(selection);
		if (cellSelection) {
			painter->setBrush(QColor(0, 0, 255, 64));
			painter->drawRect(QRectF(gridToQt(cellSelection->getPosition().free()), gridToQt((cellSelection->getPosition() + Vector(1)).free())));
			return;
		}
		SharedDimensionalSelection dimensionalSelection = selectionCast<DimensionalSelection>(selection);
		if (dimensionalSelection) {
			for (int i = 0; i < dimensionalSelection->size(); i++) {
				renderSelection(painter, dimensionalSelection->getSelection(i), mode, depth + 1);
			}
			return;
		}
		return;
	}
	case SelectionObjectElement::RenderMode::SELECTION_INVERTED:
	{
		SharedCellSelection cellSelection = selectionCast<CellSelection>(selection);
		if (cellSelection) {
			painter->setBrush(QColor(255, 0, 0, 64));
			painter->drawRect(QRectF(gridToQt(cellSelection->getPosition().free()), gridToQt((cellSelection->getPosition() + Vector(1)).free())));
			return;
		}
		SharedDimensionalSelection dimensionalSelection = selectionCast<DimensionalSelection>(selection);
		if (dimensionalSelection) {
			for (int i = 0; i < dimensionalSelection->size(); i++) {
				renderSelection(painter, dimensionalSelection->getSelection(i), mode, depth + 1);
			}
			return;
		}
		return;
	}
	case SelectionObjectElement::RenderMode::ARROWS:
	{
		SharedCellSelection cellSelection = selectionCast<CellSelection>(selection);
		if (cellSelection) {
			painter->setBrush(QColor(0, 0, 255, 64));
			painter->drawRect(QRectF(gridToQt(cellSelection->getPosition().free()), gridToQt((cellSelection->getPosition() + Vector(1)).free())));
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
					QPointF start = gridToQt(orgin.free() + FVector(0.5f, 0.5f));
					orgin += projectionSelection->getStep();
					QPointF end = gridToQt(orgin.free() + FVector(0.5f, 0.5f));
					drawArrow(painter, start, end, 16.0f, arrowColorOrder[height % 26]);
				}
				renderSelection(painter, dimensionalSelection->getSelection(0), mode, depth + 1);
			} else {
				for (int i = 0; i < dimensionalSelection->size(); i++) {
					renderSelection(painter, dimensionalSelection->getSelection(i), mode, depth + 1);
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

void QtRenderer::renderBlock(QPainter* painter, BlockType type, Position position, Rotation rotation, logic_state_t state) {
	// block
	Vector blockSize(circuit->getBlockContainer()->getBlockDataManager()->getBlockSize(type));

	Vector blockOriginOffset = rotateVectorWithArea(
		Vector(0),
		blockSize,
		rotation
	);

	QPointF size = gridToQt(blockSize.free());
	QPointF rotationPoint = gridToQt((position + blockOriginOffset).free() + FVector(0.5f, 0.5f));
	// get tile set coordinate
	if (type > 13) {
		type = BlockType::LIGHT;
	}
	Vec2Int tilePoint = tileSetInfo->getTopLeftPixel(type+1, state);
	Vec2Int tileSize = tileSetInfo->getCellPixelSize();

	QRectF tileSetRect(QPointF(tilePoint.x, tilePoint.y), QSizeF(tileSize.x, tileSize.y));

	// rotate and position painter to center of block
	painter->translate(rotationPoint);
	painter->rotate(getDegrees(rotation));

	// draw the block from the center
	QRectF drawRect = QRectF(gridToQt(FVector(-0.5f, -0.5f)), QSizeF(size.x(), size.y()));
	painter->drawPixmap(drawRect, tileSet, tileSetRect);

	// undo transformations
	painter->rotate(-getDegrees(rotation));
	painter->translate(-rotationPoint);
}

const char* connectionOFF = "#97A9E1";
const char* connectionON = "#8FE97F";
const char* connectionFLOATING = "#B0A4FF";
const char* connectionUNDEFINED = "#FFA7A4";

QColor QtRenderer::getStateColor(logic_state_t state) {
	switch (state) {
	case logic_state_t::LOW:
		return QColor(connectionOFF);
	case logic_state_t::HIGH:
		return QColor(connectionON);
	case logic_state_t::FLOATING:
		return QColor(connectionFLOATING);
	default:
		return QColor(connectionUNDEFINED);
	}
}

void QtRenderer::renderConnection(QPainter* painter, FPosition aPos, FPosition bPos, FVector aControlOffset, FVector bControlOffset, logic_state_t state) {

	painter->setPen(QPen(getStateColor(state), scalePixelCount(30.0f), Qt::SolidLine, Qt::RoundCap));

	QPointF start = gridToQt(aPos);
	QPointF end = gridToQt(bPos);

	QPointF c1 = (abs(aControlOffset.dx) > abs(aControlOffset.dy)) ? gridToQt(aPos + FVector(aControlOffset.dx * 1.3f, 0)) : gridToQt(aPos + FVector(0, aControlOffset.dy * 1.3f));
	QPointF c2 = (abs(bControlOffset.dx) > abs(bControlOffset.dy)) ? gridToQt(bPos + FVector(bControlOffset.dx * 1.3f, 0)) : gridToQt(bPos + FVector(0, bControlOffset.dy * 1.3f));


	// painter->drawLine(start, end);

	QPainterPath myPath;
	myPath.moveTo(start);
	myPath.cubicTo(c1, c2, end);
	painter->drawPath(myPath);
}

const float edgeDis = 0.48f;
const float sideShift = 0.25f;

void QtRenderer::renderConnection(QPainter* painter, Position aPos, const Block* a, Position bPos, const Block* b, logic_state_t state) {
	FVector centerOffset(0.5f, 0.5f);

	if (a == b) {
		drawText(painter, gridToQt(aPos.free() + centerOffset), "S", 30, getStateColor(state));
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

	renderConnection(painter, aPos.free() + centerOffset + aSocketOffset, bPos.free() + centerOffset + bSocketOffset, aSocketOffset, bSocketOffset, state);
}

void QtRenderer::renderConnection(QPainter* painter, Position aPos, Position bPos, logic_state_t state) {
	// Socket offsets will be retrieved data later, this code will go
	const Block* a = circuit->getBlockContainer()->getBlock(aPos);
	const Block* b = circuit->getBlockContainer()->getBlock(bPos);

	renderConnection(painter, aPos, a, bPos, b, state);
}

void QtRenderer::renderConnection(QPainter* painter, Position aPos, FPosition bPos, logic_state_t state) {
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

	renderConnection(painter, aPos.free() + centerOffset + aSocketOffset, bPos, aSocketOffset, FVector(0.0f, 0.0f), state);
}

QPointF QtRenderer::gridToQt(FPosition position) {
	assert(viewManager);

	Vec2 viewPos = viewManager->gridToView(position);
	return QPointF(viewPos.x * w, viewPos.y * h);
}

QPointF QtRenderer::gridToQt(FVector vector) {
	assert(viewManager);

	Vec2 viewPos = viewManager->gridToView(vector);
	return QPointF(viewPos.x * w, viewPos.y * h);
}

// element -----------------------------

// selection
ElementID QtRenderer::addSelectionElement(const SelectionElement& selection) {
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

ElementID QtRenderer::addSelectionElement(const SelectionObjectElement& selection) {
	ElementID newID = currentID++;
	selectionObjectElements.emplace(newID, selection);
	return newID;
}

void QtRenderer::removeSelectionElement(ElementID selection) {
	selectionObjectElements.erase(selection);
	selectionElements.erase(selection);
	invertedSelectionElements.erase(selection);
}

// block preview
ElementID QtRenderer::addBlockPreview(const BlockPreview& blockPreview) {
	ElementID newID = currentID++;
	blockPreviews[newID] = blockPreview;
	return newID;
}

void QtRenderer::removeBlockPreview(ElementID blockPreview) {
	blockPreviews.erase(blockPreview);
}

// connection preview
ElementID QtRenderer::addConnectionPreview(const ConnectionPreview& connectionPreview) {
	ElementID newID = currentID++;

	connectionPreviews[newID] = { connectionPreview.input, connectionPreview.output };

	return newID;
}

void QtRenderer::removeConnectionPreview(ElementID connectionPreview) {
	connectionPreviews.erase(connectionPreview);
}

// half connection preview
ElementID QtRenderer::addHalfConnectionPreview(const HalfConnectionPreview& halfConnectionPreview) {
	ElementID newID = currentID++;

	halfConnectionPreviews[newID] = { halfConnectionPreview.input, halfConnectionPreview.output };

	return newID;
}

void QtRenderer::removeHalfConnectionPreview(ElementID halfConnectionPreview) {
	halfConnectionPreviews.erase(halfConnectionPreview);
}

// confetti
void QtRenderer::spawnConfetti(FPosition start) {

}


// helpers
void QtRenderer::drawArrow(QPainter* painter, const QPointF& start, const QPointF& end, float size, const QColor& color) {
	// Draw main line
	size = scalePixelCount(size);
	painter->save();
	painter->setPen(QPen(color, 3.f * size, Qt::SolidLine, Qt::RoundCap));
	auto vec = QVector2D(start - end);
	vec.normalize();
	painter->drawLine(start, end + vec.toPointF() * size * 10);
	painter->restore();
	painter->setBrush(color);

	// Draw arrowhead
	QLineF line(start, end);
	double angle = -line.angle() + 180;

	// Calculate arrowhead points
	double arrow_size = 10 * size;
	QPointF p1 = line.p2();
	QPointF arrow_p1(
		p1.x() - arrow_size * std::cos((angle + 150) * M_PI / 180),
		p1.y() - arrow_size * std::sin((angle + 150) * M_PI / 180)
	);

	QPointF arrow_p2(
		p1.x() - arrow_size * std::cos((angle - 150) * M_PI / 180),
		p1.y() - arrow_size * std::sin((angle - 150) * M_PI / 180)
	);

	// Draw arrowhead as a polygon
	QPolygonF arrow_head;
	arrow_head << p1 << arrow_p1 << arrow_p2;
	painter->drawPolygon(arrow_head);
}

void QtRenderer::drawText(QPainter* painter, const QPointF& center, const QString& text, float size, const QColor& color) {
	if (scalePixelCount(size * 4) <= 1) return;
	painter->setPen(QPen(color));
	QFont font = painter->font();
	font.setPixelSize(scalePixelCount(size * 4));
	painter->setFont(font);
	painter->drawText(QRectF(center + QPointF(-100, -100), center + QPointF(100, 100)), text, QTextOption(Qt::AlignCenter));
}
