#include <QPainterPath>
#include <QDateTime>
#include <QPainter>
#include <QDebug>

#include <memory>
#include <set>

#include "QtRenderer.h"
#include "backend/connection/connectionEnd.h"
#include "backend/position/position.h"
#include "backend/defs.h"
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
    Position topLeft = viewManager->getTopLeft().snap();
    Position bottomRight = viewManager->getBottomRight().snap();

    // render grid and collect blocks + connections
    std::set<const Block*> blocksToRender;
    for (int x = topLeft.x; x <= bottomRight.x; ++x) {
        for (int y = topLeft.y; y <= bottomRight.y; ++y) {
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
    // 4e75a6 and 78b5ff
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(QPen(QColor( (QDateTime::currentSecsSinceEpoch() % 2 == 1) ? 2507161 : 7910911 ), 40.0f / viewManager->getViewHeight()));
    for (const auto& block : *(blockContainer->getBlockContainer())) {
        for (connection_end_id_t id = 0; id <= block.second.getConnectionContainer().getMaxConnectionId(); id++) {
            // return if input, we only want outputs
            if (block.second.isConnectionInput(id)) continue;
            for (auto connectionIter : block.second.getConnectionContainer().getConnections(id)) {
                const Block* other = blockContainer->getBlockContainer()->getBlock(connectionIter.getBlockId());
                Position pos = block.second.getConnectionPosition(id).first;
                Position otherPos = other->getConnectionPosition(connectionIter.getConnectionId()).first;

                FPosition centerOffset(0.5, 0.5f);
                FPosition socketOffset;
                FPosition otherSocketOffset;

                // Socket offsets will be retrieved data later, this code will go

                if (block.second.getRotation() == Rotation::ZERO) socketOffset = { 0.5f, 0.0f };
                if (block.second.getRotation() == Rotation::NINETY) socketOffset = { 0.0f, 0.5f };
                if (block.second.getRotation() == Rotation::ONE_EIGHTY) socketOffset = { -0.5f, 0.0f };
                if (block.second.getRotation() == Rotation::TWO_SEVENTY) socketOffset = { 0.0f, -0.5f };

                if (other->getRotation() == Rotation::ZERO) otherSocketOffset = { -0.5f, 0.0f };
                if (other->getRotation() == Rotation::NINETY) otherSocketOffset = { 0.0f, -0.5f };
                if (other->getRotation() == Rotation::ONE_EIGHTY) otherSocketOffset = { 0.5f, 0.0f };
                if (other->getRotation() == Rotation::TWO_SEVENTY) otherSocketOffset = { 0.0f, 0.5f };

                QPointF start = gridToQt(pos.free() + centerOffset + socketOffset);
                QPointF end = gridToQt(otherPos.free() + centerOffset + otherSocketOffset);
                QPointF c1 = gridToQt(pos.free() + centerOffset + socketOffset*2);
                QPointF c2 = gridToQt(otherPos.free() + centerOffset + otherSocketOffset*2);

                QPainterPath myPath;
                myPath.moveTo(start);
                myPath.cubicTo(c1, c2, end);
                painter->drawPath(myPath);

                // painter->drawLine(gridToQt(pos.free() + centerOffset + socketOffset), gridToQt(otherPos.free() + centerOffset + otherSocketOffset));
            }
        }
    }
    painter->restore();

    // render tints
    // render lines
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

void QtRenderer::removeElement(ElementID id) {

}

// tint
ElementID QtRenderer::addTint(Position position, Color color) {
    return 0;
}

ElementID QtRenderer::addTint(FPosition start, float width, float height, Color color) {
    return 0;
}

// block preview
ElementID QtRenderer::addBlockPreview(Position position, Rotation rotation, Color modulate, float alpha) {
    return 0;
}

// connection preview
ElementID QtRenderer::addConnectionPreview(Position inputCellPos, Position outputCellPos, Color modulate, float alpha) {
    return 0;
}

// confetti
void QtRenderer::addConfetti(FPosition start) {

}
