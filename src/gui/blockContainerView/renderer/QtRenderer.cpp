#include <QPainterPath>
#include <QDateTime>
#include <QDebug>

#include <algorithm>
#include <memory>
#include <set>

#include "QtRenderer.h"
#include "backend/block/block.h"
#include "backend/connection/connectionEnd.h"
#include "backend/evaluator/logicState.h"
#include "backend/position/position.h"
#include "backend/defs.h"
#include "gui/blockContainerView/renderer/renderer.h"
#include "util/vec2.h"
#include "backend/address.h"

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
        tileSetInfo = std::make_unique<TileSet<BlockType>>(Vec2Int(256, 128));
        tileSetInfo->addRegion(BlockType::NONE, { 0, 0 }, { 32, 32 });
        tileSetInfo->addRegion(BlockType::BLOCK, { 32, 0 }, { 32, 32 });
        tileSetInfo->addRegion(BlockType::CUSTOM, { 32, 0 }, { 32, 32 });
        tileSetInfo->addRegion(BlockType::TYPE_COUNT, { 32, 0 }, { 32, 32 });
        tileSetInfo->addRegion(BlockType::AND, { 64, 0 }, { 32, 32 });
        tileSetInfo->addRegion(BlockType::OR, { 96, 0 }, { 32, 32 });
        tileSetInfo->addRegion(BlockType::XOR, { 128, 0 }, { 32, 32 });
        tileSetInfo->addRegion(BlockType::NAND, { 160, 0 }, { 32, 32 });
        tileSetInfo->addRegion(BlockType::NOR, { 192, 0 }, { 32, 32 });
        tileSetInfo->addRegion(BlockType::XNOR, { 224, 0 }, { 32, 32 });
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

void QtRenderer::render(QPainter* painter) {
    // error checking
    assert(viewManager);
    if (tileSet.isNull() || tileSetInfo == nullptr) {
        painter->drawText(QRect(0, 0, w, h), Qt::AlignCenter, "No tileSet found");
        qDebug() << "ERROR: QTRenderer has no tileSet, cnanot proceed with render.";
        return;
    }

    // render lambdas ---
    auto renderCell = [&](FPosition position, BlockType type) -> void {
        QPointF point = gridToQt(position);
        QPointF pointBR = gridToQt(position + FPosition(1.0f, 1.0f));

        TileRegion tsRegion = tileSetInfo->getRegion(type);
        QRectF tileSetRect(QPointF(tsRegion.pixelPosition.x, tsRegion.pixelPosition.y),
            QSizeF(tsRegion.pixelSize.x, tsRegion.pixelSize.y));

        painter->drawPixmap(QRectF(point, pointBR),
            tileSet,
            tileSetRect);
        };
    // --- end of render lambdas

    // get states
    std::vector<Address> blockAddresses;
    for (const auto& block : *(blockContainer->getBlockContainer())) {
        blockAddresses.push_back(Address(block.second.getPosition()));
    }
    std::vector<logic_state_t> blockStates;
    if (evaluator) {
        blockStates = evaluator->getBulkStates(blockAddresses);
    } else {
        blockStates = std::vector<logic_state_t>(blockAddresses.size());
        std::fill(blockStates.begin(), blockStates.end(), false);
    }
    
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
    int stateIndex = 0;
    for (const auto& block : *(blockContainer->getBlockContainer())) { 
        if (block.second.getPosition().withinArea(topLeftBound, bottomRightBound) || block.second.getLargestPosition().withinArea(topLeftBound, bottomRightBound)) {
            renderBlock(painter, block.second.type(), block.second.getPosition(), block.second.getRotation(), blockStates[stateIndex]);
        }
        stateIndex++;
    }
        
    // render block previews
    for (const auto& preview : blockPreviews) {
        renderBlock(painter, preview.second.type, preview.second.position, preview.second.rotation);
    }

    // render connections
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    stateIndex = 0;
    for (const auto& block : *(blockContainer->getBlockContainer())) {
        bool state = blockStates[stateIndex];
        for (connection_end_id_t id = 0; id <= block.second.getConnectionContainer().getMaxConnectionId(); id++) {
            // continue if input, we only want outputs
            if (block.second.isConnectionInput(id)) continue;

            Position pos = block.second.getConnectionPosition(id).first;
            for (auto connectionIter : block.second.getConnectionContainer().getConnections(id)) {
                const Block* other = blockContainer->getBlockContainer()->getBlock(connectionIter.getBlockId());
                Position otherPos = other->getConnectionPosition(connectionIter.getConnectionId()).first;
                renderConnection(painter, pos, otherPos, state);
            }
        }
        stateIndex++;
    }
    painter->restore();

    // render connection previews
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    for (const auto& preview : connectionPreviews)
    {
        renderConnection(painter, preview.second.input, preview.second.output);
    }
    // render half connection previews
    for (const auto& preview : halfConnectionPreviews)
    {
        renderConnection(painter, preview.second.input, preview.second.output);
    }
    painter->restore();
    
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
    TileRegion tsRegion = tileSetInfo->getRegion(type);
    QRectF tileSetRect(QPointF(tsRegion.pixelPosition.x, tsRegion.pixelPosition.y + state * 32),
        QSizeF(tsRegion.pixelSize.x, tsRegion.pixelSize.y));

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

void QtRenderer::renderConnection(QPainter* painter, FPosition aPos, FPosition bPos, FPosition aControlOffset, FPosition bControlOffset, bool state) {
    painter->setPen(QPen(QColor(state ? 7910911 : 2507161), 25.0f / viewManager->getViewHeight()));

    QPointF start = gridToQt(aPos);
    QPointF end = gridToQt(bPos);
    QPointF c1 = gridToQt(aPos + aControlOffset);
    QPointF c2 = gridToQt(bPos + bControlOffset);

    QPainterPath myPath;
    myPath.moveTo(start);
    myPath.cubicTo(c1, c2, end);
    painter->drawPath(myPath);
}

void QtRenderer::renderConnection(QPainter* painter, Position aPos, Position bPos, bool state) {
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

    renderConnection(painter, aPos.free() + centerOffset + aSocketOffset, bPos.free() + centerOffset + bSocketOffset, aSocketOffset, bSocketOffset, state);
}

void QtRenderer::renderConnection(QPainter* painter, Position aPos, FPosition bPos, bool state) {
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

    renderConnection(painter, aPos.free() + centerOffset + aSocketOffset, bPos, aSocketOffset, FPosition(0.0f, 0.0f), state);
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

    halfConnectionPreviews[newID] = {halfConnectionPreview.input, halfConnectionPreview.output};

    return newID;
}

void QtRenderer::removeHalfConnectionPreview(ElementID halfConnectionPreview) {
    halfConnectionPreviews.erase(halfConnectionPreview);
}

// confetti
void QtRenderer::spawnConfetti(FPosition start) {

}
