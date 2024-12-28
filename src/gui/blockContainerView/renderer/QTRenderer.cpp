#include <qvectornd.h>
#include <qpainter.h>
#include <qbrush.h>
#include <qdebug.h>
#include <qpoint.h>
#include <qsize.h>

#include <memory>
#include <set>

#include "backend/connection/connectionEnd.h"
#include "backend/position/position.h"
#include "backend/defs.h"
#include "util/vector2.h"
#include "QTRenderer.h"


QTRenderer::QTRenderer()
    : w(0), h(0), blockContainer(nullptr), tileSetInfo(nullptr) {

}

void QTRenderer::initializeTileSet(const std::string& filePath) {
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

void QTRenderer::resize(int w, int h) {
    this->w = w;
    this->h = h;
}

void QTRenderer::render(QPainter* painter) {
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
    painter->setPen(QPen(Qt::blue, 40.0f / viewManager->getViewHeight()));
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

                painter->drawLine(gridToQt(pos.free() + centerOffset + socketOffset), gridToQt(otherPos.free() + centerOffset + otherSocketOffset));
            }
        }
    }
    painter->restore();

    // render tints
    // render lines
}

void QTRenderer::setBlockContainer(BlockContainerWrapper* blockContainer) {
    this->blockContainer = blockContainer;
}

void QTRenderer::updateView(ViewManager* viewManager) {
    this->viewManager = viewManager;
}

QPointF QTRenderer::gridToQt(FPosition position) {
    assert(viewManager);

    Vec2 viewPos = viewManager->gridToView(position);
    return QPointF(viewPos.x * w, viewPos.y * h);
}

// effects -----------------------------
// line
LineID QTRenderer::addLine(const std::vector<FPosition>& positions, float width) {
    return 0;
}

void QTRenderer::updateLinePosition(LineID line, int index, FPosition position) {

}

void QTRenderer::updateLinePositions(LineID line, std::vector<FPosition>& positions) {

}

void QTRenderer::updateLineWidth(LineID line, float width) {

}

void QTRenderer::removeLine(LineID line) {

}

// tint
TintID QTRenderer::addTint(Position position, Color color) {
    return 0;
}

TintID QTRenderer::addTint(FPosition start, float width, float height, Color color) {
    return 0;
}

void QTRenderer::updateTintColor(TintID tint, Color color) {

}

void QTRenderer::updateTintRect(Position start, float width, float height) {

}

void QTRenderer::removeTint(TintID tint) {

}

// confetti
void QTRenderer::addConfetti(FPosition start) {

}
