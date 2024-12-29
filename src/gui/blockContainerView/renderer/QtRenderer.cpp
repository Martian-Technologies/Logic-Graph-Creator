#include <QPainterPath>
#include <QDateTime>
#include <QPainter>
#include <QDebug>

#include <memory>
#include <qnamespace.h>
#include <set>

#include "QtRenderer.h"
#include "backend/block/block.h"
#include "backend/connection/connectionEnd.h"
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

    // get bounds
    Position topLeftBound = viewManager->getTopLeft().snap();
    Position bottomRightBound = viewManager->getBottomRight().snap();

    // render grid and collect blocks + connections
    std::set<const Block*> blocksToRender;
    for (int x = topLeftBound.x; x <= bottomRightBound.x; ++x) {
        for (int y = topLeftBound.y; y <= bottomRightBound.y; ++y) {
            const Block* block = blockContainer->getBlockContainer()->getBlock(Position(x, y));

            if (block) blocksToRender.insert(block);
            else renderCell(FPosition(x, y), BlockType::NONE);
        }
    }
    // get a list of positions of blocks we are rendering to get their states
    std::vector<Address> blockAddresses;
    for (const Block* block : blocksToRender) {
        blockAddresses.push_back(Address(block->getPosition()));
    }

    // get the states from the mainWindow's evaluator

    // render blocks
    for (const Block* block : blocksToRender) {
        renderBlock(painter, block->type(), block->getPosition(), block->getRotation());
    }

    // render block previews
    for (const auto& preview : blockPreviews) {
        renderBlock(painter, preview.second.type, preview.second.position, preview.second.rotation);
    }

    // render connections
    painter->save();
    setUpConnectionPainter(painter);
    for (const auto& block : *(blockContainer->getBlockContainer())) {
        for (connection_end_id_t id = 0; id <= block.second.getConnectionContainer().getMaxConnectionId(); id++) {
            // return if input, we only want outputs
            if (block.second.isConnectionInput(id)) continue;
            for (auto connectionIter : block.second.getConnectionContainer().getConnections(id)) {
                const Block* other = blockContainer->getBlockContainer()->getBlock(connectionIter.getBlockId());
                Position pos = block.second.getConnectionPosition(id).first;
                Position otherPos = other->getConnectionPosition(connectionIter.getConnectionId()).first;

                renderConnection(painter, &block.second, pos, other, otherPos, false);
            }
        }
    }
    painter->restore();

    // render connection previews
    painter->save();
    setUpConnectionPainter(painter);
    for (const auto& preview : connectionPreviews) {
        const Block* inputBlock = blockContainer->getBlockContainer()->getBlock(preview.second.input);
        const Block* outputBlock = blockContainer->getBlockContainer()->getBlock(preview.second.output);
        renderConnection(painter, inputBlock, preview.second.input, outputBlock, preview.second.output);
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

void QtRenderer::setUpConnectionPainter(QPainter* painter) {
    // 4e75a6 and 78b5ff
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(QPen(QColor((QDateTime::currentSecsSinceEpoch() % 2 == 1) ? 2507161 : 7910911), 25.0f / viewManager->getViewHeight()));
}

void QtRenderer::renderBlock(QPainter* painter, BlockType type, Position position, Rotation rotation) {

    Position gridSize(getBlockWidth(type), getBlockHeight(type));

    // block
    QPointF topLeft = gridToQt(position.free());
    QPointF bottomRight = gridToQt((position + gridSize).free());
    float width = bottomRight.x() - topLeft.x();
    float height = bottomRight.y() - topLeft.y();
    QPointF center = topLeft + QPointF(width / 2.0f, height / 2.0f);

    // get tile set coordinate
    TileRegion tsRegion = tileSetInfo->getRegion(type);
    QRectF tileSetRect(QPointF(tsRegion.pixelPosition.x, tsRegion.pixelPosition.y),
        QSizeF(tsRegion.pixelSize.x, tsRegion.pixelSize.y));

    // rotate and position painter to center of block
    painter->save();
    painter->translate(center);
    painter->rotate(getDegrees(rotation));

    // draw the block from the center
    QRectF drawRect = QRectF(QPointF(-width / 2.0f, -height / 2.0f), QSizeF(width, height));
    painter->drawPixmap(drawRect,
        tileSet,
        tileSetRect);

    painter->restore();
}

void QtRenderer::renderConnection(QPainter* painter, const Block* a, Position aPos, const Block* b, Position bPos, bool setupPainter) {
    FPosition centerOffset(0.5, 0.5f);
    FPosition aSocketOffset;
    FPosition bSocketOffset;

    // Socket offsets will be retrieved data later, this code will go
    if (a) {
        if (a->getRotation() == Rotation::ZERO) aSocketOffset = { 0.5f, 0.0f };
        if (a->getRotation() == Rotation::NINETY) aSocketOffset = { 0.0f, 0.5f };
        if (a->getRotation() == Rotation::ONE_EIGHTY) aSocketOffset = { -0.5f, 0.0f };
        if (a->getRotation() == Rotation::TWO_SEVENTY) aSocketOffset = { 0.0f, -0.5f };
    }
    else { aSocketOffset = { 0.0f, 0.0f }; }

    if (b) {
        if (b->getRotation() == Rotation::ZERO) bSocketOffset = { -0.5f, 0.0f };
        if (b->getRotation() == Rotation::NINETY) bSocketOffset = { 0.0f, -0.5f };
        if (b->getRotation() == Rotation::ONE_EIGHTY) bSocketOffset = { 0.5f, 0.0f };
        if (b->getRotation() == Rotation::TWO_SEVENTY) bSocketOffset = { 0.0f, 0.5f };
    }
    else { bSocketOffset = { 0.0f, 0.0f }; }


    QPointF start = gridToQt(aPos.free() + centerOffset + aSocketOffset);
    QPointF end = gridToQt(bPos.free() + centerOffset + bSocketOffset);
    QPointF c1 = gridToQt(aPos.free() + centerOffset + aSocketOffset * 2);
    QPointF c2 = gridToQt(bPos.free() + centerOffset + bSocketOffset * 2);

    QPainterPath myPath;
    myPath.moveTo(start);
    myPath.cubicTo(c1, c2, end);
    painter->drawPath(myPath);
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

// confetti
void QtRenderer::spawnConfetti(FPosition start) {

}
