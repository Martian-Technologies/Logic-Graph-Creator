#include <QElapsedTimer>
#include <QPainterPath>
#include <QDateTime>
#include <QVector2D>
#include <QDebug>

#include <memory>
#include <set>

#include "QtRenderer.h"
#include "backend/block/block.h"
#include "backend/connection/connectionEnd.h"
#include "backend/evaluator/logicState.h"
#include "backend/position/position.h"
#include "backend/defs.h"
#include "gui/blockContainerView/renderer/renderer.h"
#include "backend/address.h"
#include "util/vec2.h"

QtRenderer::QtRenderer()
    : w(0), h(0), blockContainer(nullptr), tileSetInfo(nullptr) {

}

void QtRenderer::initializeTileSet(const std::string& filePath) {
    if (filePath != "") {
        tileSet = QPixmap(filePath.c_str());

        if (tileSet.isNull()) {
            qDebug() << "ERROR: tileSet image could not be loaded from file." << filePath;
        }

        // create tileSet
        tileSetInfo = std::make_unique<TileSetInfo>(128, 14);
    }
}

void QtRenderer::resize(int w, int h) {
    this->w = w;
    this->h = h;
}

void QtRenderer::setBlockContainer(BlockContainerWrapper* blockContainer) {
    this->blockContainer = blockContainer;
}

void QtRenderer::setEvaluator(Evaluator* evaluator) {
    this->evaluator = evaluator;
}

void QtRenderer::updateView(ViewManager* viewManager) {
    this->viewManager = viewManager;
}

void QtRenderer::updateBlockContainer(DifferenceSharedPtr diff) {

}

void QtRenderer::render(QPainter* painter) {
    // error checking
    assert(viewManager);
    if (tileSet.isNull() || tileSetInfo == nullptr) {
        painter->drawText(QRect(0, 0, w, h), Qt::AlignCenter, "No tileSet found");
        qDebug() << "ERROR: QTRenderer has no tileSet, can not proceed with render.";
        return;
    }

    QElapsedTimer timer;
    timer.start();

    // render lambdas ---
    auto renderCell = [&](FPosition position, BlockType type) -> void {
        QPointF point = gridToQt(position);
        QPointF pointBR = gridToQt(position + FPosition(1.0f, 1.0f));

        Vec2Int tilePoint = tileSetInfo->getTopLeftPixel(type, false);
        Vec2Int tileSize = tileSetInfo->getCellPixelSize();

        QRectF tileSetRect(QPointF(tilePoint.x, tilePoint.y),
            QSizeF(tileSize.x, tileSize.y));

        painter->drawPixmap(QRectF(point, pointBR),
            tileSet,
            tileSetRect);
        };
    // --- end of render lambdas

    if (evaluator) {
        // get states
        std::vector<Address> blockAddresses;
        std::vector<const Block*> blocks;
        for (const auto& block : *(blockContainer->getBlockContainer())) {
            blockAddresses.push_back(Address(block.second.getPosition()));
            blocks.push_back(&(block.second));
        }
        std::vector<logic_state_t> blockStates = evaluator->getBulkStates(blockAddresses);

        // get bounds
        Position topLeftBound = viewManager->getTopLeft().snap();
        Position bottomRightBound = viewManager->getBottomRight().snap();

        // render grid
        std::set<const Block*> blocksToRender;
        for (int x = topLeftBound.x; x <= bottomRightBound.x; ++x) {
            for (int y = topLeftBound.y; y <= bottomRightBound.y; ++y) {
                renderCell(FPosition(x, y), BlockType::NONE);
            }
        }

        // render blocks
        for (unsigned int i = 0; i < blocks.size(); i++) {
            if (blocks[i]->getPosition().withinArea(topLeftBound, bottomRightBound) || blocks[i]->getLargestPosition().withinArea(topLeftBound, bottomRightBound)) {
                renderBlock(painter, blocks[i]->type(), blocks[i]->getPosition(), blocks[i]->getRotation(), blockStates[i]);
            }
        }

        // render block previews
        painter->setOpacity(0.4f);
        for (const auto& preview : blockPreviews) {
            renderBlock(painter, preview.second.type, preview.second.position, preview.second.rotation);
        }
        painter->setOpacity(1.0f);


        // render connections
        std::vector<QLineF> connectionLinesOff;
        std::vector<QLineF> connectionLinesOn;
        for (unsigned int i = 0; i < blocks.size(); i++) {
            if (connectionLinesOff.size() + connectionLinesOn.size() >= lineRendingLimit) break;
            bool state = blockStates[i];
            for (connection_end_id_t id = 0; id <= blocks[i]->getConnectionContainer().getMaxConnectionId(); id++) {
                // continue if input, we only want outputs
                if (blocks[i]->isConnectionInput(id)) continue;

                Position pos = blocks[i]->getConnectionPosition(id).first;
                for (auto connectionIter : blocks[i]->getConnectionContainer().getConnections(id)) {
                    const Block* other = blockContainer->getBlockContainer()->getBlock(connectionIter.getBlockId());
                    Position otherPos = other->getConnectionPosition(connectionIter.getConnectionId()).first;
                    renderConnection(painter, pos, blocks[i], otherPos, other, state ? connectionLinesOn : connectionLinesOff);
                }
            }
        }
        // render connection previews
        for (const auto& preview : connectionPreviews) {
            renderConnection(painter, preview.second.input, preview.second.output, connectionLinesOff);
        }
        // render half connection previews
        for (const auto& preview : halfConnectionPreviews) {
            renderConnection(painter, preview.second.input, preview.second.output, connectionLinesOff);
        }
        painter->save();
        painter->setOpacity(0.8f);
        // painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(QPen(QColor(2507161), 25.0f / viewManager->getViewHeight()));
        painter->drawLines(&connectionLinesOff[0], connectionLinesOff.size());
        painter->setPen(QPen(QColor(7910911), 25.0f / viewManager->getViewHeight()));
        painter->drawLines(&connectionLinesOn[0], connectionLinesOn.size());
        painter->restore();
    } else {
        // get bounds
        Position topLeftBound = viewManager->getTopLeft().snap();
        Position bottomRightBound = viewManager->getBottomRight().snap();

        // render grid
        std::set<const Block*> blocksToRender;
        for (int x = topLeftBound.x; x <= bottomRightBound.x; ++x) {
            for (int y = topLeftBound.y; y <= bottomRightBound.y; ++y) {
                renderCell(FPosition(x, y), BlockType::NONE);
            }
        }

        std::vector<Address> blockAddresses;
        std::vector<const Block*> blocks;
        for (const auto& block : *(blockContainer->getBlockContainer())) {
            if (block.second.getPosition().withinArea(topLeftBound, bottomRightBound) || block.second.getLargestPosition().withinArea(topLeftBound, bottomRightBound)) {
                renderBlock(painter, block.second.type(), block.second.getPosition(), block.second.getRotation());
            }
            blocks.push_back(&(block.second));
        }

        // render block previews
        painter->setOpacity(0.4f);
        for (const auto& preview : blockPreviews) {
            renderBlock(painter, preview.second.type, preview.second.position, preview.second.rotation);
        }
        painter->setOpacity(1.0f);


        // render connections
        std::vector<QLineF> connectionLines;
        for (unsigned int i = 0; i < blocks.size(); i++) {
            if (connectionLines.size() >= lineRendingLimit) break;
            for (connection_end_id_t id = 0; id <= blocks[i]->getConnectionContainer().getMaxConnectionId(); id++) {
                // continue if input, we only want outputs
                if (blocks[i]->isConnectionInput(id)) continue;

                Position pos = blocks[i]->getConnectionPosition(id).first;
                for (auto connectionIter : blocks[i]->getConnectionContainer().getConnections(id)) {
                    const Block* other = blockContainer->getBlockContainer()->getBlock(connectionIter.getBlockId());
                    Position otherPos = other->getConnectionPosition(connectionIter.getConnectionId()).first;
                    renderConnection(painter, pos, blocks[i], otherPos, other, connectionLines);
                }
            }
        }
        // render connection previews
        for (const auto& preview : connectionPreviews) {
            renderConnection(painter, preview.second.input, preview.second.output, connectionLines);
        }
        // render half connection previews
        for (const auto& preview : halfConnectionPreviews) {
            renderConnection(painter, preview.second.input, preview.second.output, connectionLines);
        }
        painter->save();
        painter->setOpacity(0.8f);
        // painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(QPen(QColor(2507161), 25.0f / viewManager->getViewHeight()));
        painter->drawLines(&connectionLines[0], connectionLines.size());
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
        FPosition bottomRight = selection.second.bottomRight.free() + FPosition(1.0f, 1.0f);
        painter->drawRect(QRectF(gridToQt(topLeft), gridToQt(bottomRight)));
    }
    // inverted selections
    QColor transparentRed(255, 0, 0, 64);
    painter->setBrush(transparentRed);
    for (const auto& selection : invertedSelectionElements) {
        FPosition topLeft = selection.second.topLeft.free();
        FPosition bottomRight = selection.second.bottomRight.free() + FPosition(1.0f, 1.0f);
        painter->drawRect(QRectF(gridToQt(topLeft), gridToQt(bottomRight)));
    }
    // selection object
    for (const auto selection : selectionObjectElements) {
        renderSelection(painter, selection.second.selection, selection.second.renderMode);
    }
    painter->restore();

    lastFrameTime = timer.nsecsElapsed() / 1e6f;
}

const QColor arrowColorOrder[] = {
    QColor(255, 0, 0, 80),
    QColor(0, 255, 0, 80),
    QColor(0, 0, 255, 80),
    QColor(255, 255, 0, 80),
    QColor(255, 0, 255, 80),
    QColor(0, 255, 255, 80),
    QColor(255, 255, 255, 80),
    QColor(127, 0, 0, 80),
    QColor(0, 127, 0, 80),
    QColor(0, 0, 127, 80),
    QColor(127, 127, 0, 80),
    QColor(127, 0, 127, 80),
    QColor(0, 127, 127, 80),
    QColor(127, 127, 127, 80),
    QColor(255, 127, 0, 80),
    QColor(255, 0, 127, 80),
    QColor(0, 255, 127, 80),
    QColor(127, 255, 0, 80),
    QColor(127, 0, 255, 80),
    QColor(0, 127, 255, 80),
    QColor(255, 127, 127, 80),
    QColor(127, 255, 127, 80),
    QColor(127, 127, 255, 80),
    QColor(255, 255, 127, 80),
    QColor(255, 127, 255, 80),
    QColor(127, 255, 255, 80)
};

void QtRenderer::renderSelection(QPainter* painter, const SharedSelection selection, SelectionObjectElement::RenderMode mode, unsigned int depth) {
    switch (mode) {
    case SelectionObjectElement::RenderMode::SELECTION:
    {
        SharedCellSelection cellSelection = selectionCast<CellSelection>(selection);
        if (cellSelection) {
            painter->setBrush(QColor(0, 0, 255, 64));
            painter->drawRect(QRectF(gridToQt(cellSelection->getPosition().free()), gridToQt((cellSelection->getPosition() + Position(1, 1)).free())));
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
            painter->drawRect(QRectF(gridToQt(cellSelection->getPosition().free()), gridToQt((cellSelection->getPosition() + Position(1, 1)).free())));
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
            painter->drawRect(QRectF(gridToQt(cellSelection->getPosition().free()), gridToQt((cellSelection->getPosition() + Position(1, 1)).free())));
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
                    height ++;
                    dSel = selectionCast<DimensionalSelection>(sel);
                }
                for (int i = 1; i < projectionSelection->size(); i++) {
                    QPointF start = gridToQt(orgin.free() + FPosition(0.5f, 0.5f));
                    orgin += projectionSelection->getStep();
                    QPointF end = gridToQt(orgin.free() + FPosition(0.5f, 0.5f));
                    drawArrow(painter, start, end, 20.f / viewManager->getViewHeight(), arrowColorOrder[height % 26]);
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

void QtRenderer::renderBlock(QPainter* painter, BlockType type, Position position, Rotation rotation, bool state) {

    Position gridSize(getBlockWidth(type), getBlockHeight(type));

    // block
    QPointF topLeft = gridToQt(position.free());
    QPointF bottomRight = gridToQt((position + gridSize).free());
    float width = bottomRight.x() - topLeft.x();
    float height = bottomRight.y() - topLeft.y();
    QPointF center = topLeft + QPointF(width / 2.0f, height / 2.0f);

    // get tile set coordinate
    Vec2Int tilePoint = tileSetInfo->getTopLeftPixel(type, state);
    Vec2Int tileSize = tileSetInfo->getCellPixelSize();

    QRectF tileSetRect(QPointF(tilePoint.x, tilePoint.y),
        QSizeF(tileSize.x, tileSize.y));

    // rotate and position painter to center of block
    painter->translate(center);
    painter->rotate(getDegrees(rotation));

    // draw the block from the center
    QRectF drawRect = QRectF(QPointF(-width / 2.0f, -height / 2.0f), QSizeF(width, height));
    painter->drawPixmap(drawRect,
        tileSet,
        tileSetRect);

    // undo transformations
    painter->rotate(-getDegrees(rotation));
    painter->translate(-center);
}

void QtRenderer::renderConnection(QPainter* painter, FPosition aPos, FPosition bPos, FPosition aControlOffset, FPosition bControlOffset, std::vector<QLineF>& lines) {
    QPointF start = gridToQt(aPos);
    QPointF end = gridToQt(bPos);


    // QPointF c1 = gridToQt(aPos + aControlOffset);
    // QPointF c2 = gridToQt(bPos + bControlOffset);

    lines.push_back(QLineF(start, end));

    // painter->drawLine(start, end);

    // QPainterPath myPath;
    // myPath.moveTo(start);
    // myPath.cubicTo(c1, c2, end);
    // painter->drawPath(myPath);
}

void QtRenderer::renderConnection(QPainter* painter, Position aPos, const Block* a, Position bPos, const Block* b, std::vector<QLineF>& lines) {
    FPosition centerOffset(0.5f, 0.5f);
    FPosition aSocketOffset(0.0f, 0.0f);
    FPosition bSocketOffset(0.0f, 0.0f);

    if (a) {
        switch (a->getRotation()) {
        case Rotation::ZERO: aSocketOffset = { 0.5f, 0.0f }; break;
        case Rotation::NINETY: aSocketOffset = { 0.0f, 0.5f }; break;
        case Rotation::ONE_EIGHTY: aSocketOffset = { -0.5f, 0.0f }; break;
        case Rotation::TWO_SEVENTY: aSocketOffset = { 0.0f, -0.5f }; break;
        }
    }

    if (b) {
        switch (b->getRotation()) {
        case Rotation::ZERO: bSocketOffset = { -0.5f, 0.0f }; break;
        case Rotation::NINETY: bSocketOffset = { 0.0f, -0.5f }; break;
        case Rotation::ONE_EIGHTY: bSocketOffset = { 0.5f, 0.0f }; break;
        case Rotation::TWO_SEVENTY: bSocketOffset = { 0.0f, 0.5f }; break;
        }
    }

    renderConnection(painter, aPos.free() + centerOffset + aSocketOffset, bPos.free() + centerOffset + bSocketOffset, aSocketOffset, bSocketOffset, lines);
}

void QtRenderer::renderConnection(QPainter* painter, Position aPos, Position bPos, std::vector<QLineF>& lines) {
    FPosition centerOffset(0.5f, 0.5f);
    FPosition aSocketOffset(0.0f, 0.0f);
    FPosition bSocketOffset(0.0f, 0.0f);

    // Socket offsets will be retrieved data later, this code will go
    const Block* a = blockContainer->getBlockContainer()->getBlock(aPos);
    const Block* b = blockContainer->getBlockContainer()->getBlock(bPos);

    if (a) {
        if (a->getRotation() == Rotation::ZERO) aSocketOffset = { 0.5f, 0.0f };
        if (a->getRotation() == Rotation::NINETY) aSocketOffset = { 0.0f, 0.5f };
        if (a->getRotation() == Rotation::ONE_EIGHTY) aSocketOffset = { -0.5f, 0.0f };
        if (a->getRotation() == Rotation::TWO_SEVENTY) aSocketOffset = { 0.0f, -0.5f };
    }

    if (b) {
        if (b->getRotation() == Rotation::ZERO) bSocketOffset = { -0.5f, 0.0f };
        if (b->getRotation() == Rotation::NINETY) bSocketOffset = { 0.0f, -0.5f };
        if (b->getRotation() == Rotation::ONE_EIGHTY) bSocketOffset = { 0.5f, 0.0f };
        if (b->getRotation() == Rotation::TWO_SEVENTY) bSocketOffset = { 0.0f, 0.5f };
    }

    renderConnection(painter, aPos.free() + centerOffset + aSocketOffset, bPos.free() + centerOffset + bSocketOffset, aSocketOffset, bSocketOffset, lines);
}

void QtRenderer::renderConnection(QPainter* painter, Position aPos, FPosition bPos, std::vector<QLineF>& lines) {
    FPosition centerOffset(0.5f, 0.5f);
    FPosition aSocketOffset(0.0f, 0.0f);

    // Socket offsets will be retrieved data later, this code will go
    const Block* a = blockContainer->getBlockContainer()->getBlock(aPos);

    if (a) {
        if (a->getRotation() == Rotation::ZERO) aSocketOffset = { 0.5f, 0.0f };
        if (a->getRotation() == Rotation::NINETY) aSocketOffset = { 0.0f, 0.5f };
        if (a->getRotation() == Rotation::ONE_EIGHTY) aSocketOffset = { -0.5f, 0.0f };
        if (a->getRotation() == Rotation::TWO_SEVENTY) aSocketOffset = { 0.0f, -0.5f };
    }

    renderConnection(painter, aPos.free() + centerOffset + aSocketOffset, bPos, aSocketOffset, FPosition(0.0f, 0.0f), lines);
}

QPointF QtRenderer::gridToQt(FPosition position) {
    assert(viewManager);

    Vec2 viewPos = viewManager->gridToView(position);
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

void QtRenderer::drawArrow(QPainter* painter, const QPointF& start, const QPointF& end, float scale, const QColor& color) {
    // Draw main line
    painter->save();
    painter->setPen(QPen(color, 3.f * scale));
    auto vec = QVector2D(start - end);
    vec.normalize();
    painter->drawLine(start, end + vec.toPointF() * scale*10);
    painter->restore();
    painter->setBrush(color);

    // Draw arrowhead
    QLineF line(start, end);
    double angle = -line.angle() + 180;

    // Calculate arrowhead points
    double arrow_size = 10 * scale;
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
