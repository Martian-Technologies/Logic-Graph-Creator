#include <QPainterPath>
#include <QDateTime>
#include <QDebug>
#include <QElapsedTimer>

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
        tileSetInfo = std::make_unique<TileSetInfo>(32, 13);
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
    painter->restore();

    lastFrameTime = timer.nsecsElapsed() / 1e6f;
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

void QtRenderer::removeSelectionElement(ElementID selection) {
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
