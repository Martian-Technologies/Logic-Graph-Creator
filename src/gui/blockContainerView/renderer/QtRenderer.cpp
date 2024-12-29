#include <QPainterPath>
#include <QDateTime>
#include <QPainter>
#include <QDebug>

#include <memory>
#include <qnamespace.h>
#include <set>

#include "QtRenderer.h"
#include "backend/connection/connectionEnd.h"
#include "backend/position/position.h"
#include "backend/defs.h"
#include "gui/blockContainerView/renderer/renderer.h"
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

void QtRenderer::render(QPainter* painter) {
    // error checking
    assert(viewManager);
    if (tileSet.isNull() || tileSetInfo == nullptr) {
        painter->drawText(QRect(0, 0, w, h), Qt::AlignCenter, "No tileSet found");
        qDebug() << "ERROR: QTRenderer has no tileSet, cnanot proceed with render.";
        return;
    }

    // init
    // painter->setRenderHint(QPainter::Antialiasing, true);

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

    auto renderBlock = [&](const Block* block) -> void {
        Position gridSize(block->widthNoRotation(), block->heightNoRotation());

        // block
        QPointF topLeft = gridToQt(block->getPosition().free());
        QPointF bottomRight = gridToQt((block->getPosition() + gridSize).free());
        float width = bottomRight.x() - topLeft.x();
        float height = bottomRight.y() - topLeft.y();
        QPointF center = topLeft + QPointF(width / 2.0f, height / 2.0f);

        // get tile set coordinate
        TileRegion tsRegion = tileSetInfo->getRegion(block->type());
        QRectF tileSetRect(QPointF(tsRegion.pixelPosition.x, tsRegion.pixelPosition.y),
            QSizeF(tsRegion.pixelSize.x, tsRegion.pixelSize.y));

        // rotate and position painter to center of block
        painter->save();
        painter->translate(center);
        painter->rotate(getDegrees(block->getRotation()));

        // draw the block from the center
        QRectF drawRect = QRectF(QPointF(-width / 2.0f, -height / 2.0f), QSizeF(width, height));
        painter->drawPixmap(drawRect,
            tileSet,
            tileSetRect);

        painter->restore();
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

    // render blocks
    for (const Block* block : blocksToRender) {
        renderBlock(block);
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

    // render selections
    painter->save();
    painter->setPen(Qt::NoPen);
    // normal selection
    QColor transparentBlue(0, 0, 255, 64);
    painter->setBrush(transparentBlue);
    for (const auto& selection : selectionElements) {
        FPosition topLeft = selection.second.topLeft.free();
        FPosition bottomRight = selection.second.bottomRight.free() + FPosition(1.0f,1.0f);
        painter->drawRect(QRectF(gridToQt(topLeft),gridToQt(bottomRight)));
    }
    // inverted selections
    QColor transparentRed(255, 0, 0, 64);
    painter->setBrush(transparentRed);
    for (const auto& selection : invertedSelectionElements) {
        painter->drawRect(QRectF(gridToQt(selection.second.topLeft.free()),gridToQt(selection.second.bottomRight.free())));
    }
    painter->restore();
    
    // render lines
}

void QtRenderer::setUpConnectionPainter(QPainter* painter) {
    // 4e75a6 and 78b5ff
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(QPen(QColor( (QDateTime::currentSecsSinceEpoch() % 2 == 1) ? 2507161 : 7910911 ), 25.0f / viewManager->getViewHeight()));
}

void QtRenderer::renderConnection(QPainter* painter, const Block* a, Position aPos, const Block* b, Position bPos, bool setupPainter) {
    if (setupPainter)
    {
        painter->save();
        setUpConnectionPainter(painter);
    }
    
    FPosition centerOffset(0.5, 0.5f);
    FPosition socketOffset;
    FPosition bSocketOffset;

    // Socket offsets will be retrieved data later, this code will go

    if (a->getRotation() == Rotation::ZERO) socketOffset = { 0.5f, 0.0f };
    if (a->getRotation() == Rotation::NINETY) socketOffset = { 0.0f, 0.5f };
    if (a->getRotation() == Rotation::ONE_EIGHTY) socketOffset = { -0.5f, 0.0f };
    if (a->getRotation() == Rotation::TWO_SEVENTY) socketOffset = { 0.0f, -0.5f };

    if (b->getRotation() == Rotation::ZERO) bSocketOffset = { -0.5f, 0.0f };
    if (b->getRotation() == Rotation::NINETY) bSocketOffset = { 0.0f, -0.5f };
    if (b->getRotation() == Rotation::ONE_EIGHTY) bSocketOffset = { 0.5f, 0.0f };
    if (b->getRotation() == Rotation::TWO_SEVENTY) bSocketOffset = { 0.0f, 0.5f };

    QPointF start = gridToQt(aPos.free() + centerOffset + socketOffset);
    QPointF end = gridToQt(bPos.free() + centerOffset + bSocketOffset);
    QPointF c1 = gridToQt(aPos.free() + centerOffset + socketOffset*2);
    QPointF c2 = gridToQt(bPos.free() + centerOffset + bSocketOffset*2);

    QPainterPath myPath;
    myPath.moveTo(start);
    myPath.cubicTo(c1, c2, end);
    painter->drawPath(myPath);

    if (setupPainter) painter->restore();
}

void QtRenderer::setBlockContainer(BlockContainerWrapper* blockContainer) {
    this->blockContainer = blockContainer;
}

void QtRenderer::updateView(ViewManager* viewManager) {
    this->viewManager = viewManager;
}

QPointF QtRenderer::gridToQt(FPosition position) {
    assert(viewManager);

    Vec2 viewPos = viewManager->gridToView(position);
    return QPointF(viewPos.x * w, viewPos.y * h);
}

// element -----------------------------

// selection
ElementID QtRenderer::addSelectionElement(Position topLeft, Position bottomRight, bool inverted) {
    ElementID newID = currentID++;

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
    if (!inverted) selectionElements[newID] = {topLeft, bottomRight, inverted};
    else invertedSelectionElements[newID] = {topLeft, bottomRight, inverted};

    return newID;
}

void QtRenderer::removeSelectionElement(ElementID selection) {
    selectionElements.erase(selection);
    invertedSelectionElements.erase(selection);
}

// block preview
ElementID QtRenderer::addBlockPreview(Position position, Rotation rotation, Color modulate, float alpha) {
    return 0;
}

void QtRenderer::removeBlockPreview(ElementID blockPreview) {
    
}

// connection preview
ElementID QtRenderer::addConnectionPreview(Position input, Position output, Color modulate, float alpha) {
    return 0;
}

void QtRenderer::removeConnectionPreview(ElementID connectionPreview) {
    
}

// confetti
void QtRenderer::spawnConfetti(FPosition start) {
    
}
