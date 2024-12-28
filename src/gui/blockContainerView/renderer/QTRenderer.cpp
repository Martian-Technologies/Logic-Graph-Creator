#include <qvectornd.h>
#include <qpainter.h>
#include <qbrush.h>
#include <qdebug.h>
#include <qpoint.h>
#include <qsize.h>

#include <memory>
#include <unordered_set>

#include "backend/position/position.h"
#include "backend/defs.h"
#include "util/vector2.h"
#include "QTRenderer.h"


QTRenderer::QTRenderer()
    : w(0), h(0), blockContainer(nullptr), tileSetInfo(nullptr)
{
    
}

void QTRenderer::initializeTileSet(const std::string& filePath)
{   
    if (filePath != "") {
        tileSet = QPixmap(filePath.c_str());
        
        if (tileSet.isNull()) {
            qDebug() << "ERROR: tileSet image could not be loaded from file." << filePath;
        }

        // create tileSet
        tileSetInfo = std::make_unique<TileSet<BlockType>>(Vec2Int(256,128));
        tileSetInfo->addRegion(BlockType::NONE, {0, 0}, {32, 32});
        tileSetInfo->addRegion(BlockType::BLOCK, {32, 0}, {32, 32});
        tileSetInfo->addRegion(BlockType::CUSTOM, {32, 0}, {32, 32});
        tileSetInfo->addRegion(BlockType::TYPE_COUNT, {32, 0}, {32, 32});
        tileSetInfo->addRegion(BlockType::AND, {64, 0}, {32, 32});
        tileSetInfo->addRegion(BlockType::OR, {96, 0}, {32, 32});
        tileSetInfo->addRegion(BlockType::XOR, {128, 0}, {32, 32});
        tileSetInfo->addRegion(BlockType::NAND, {160, 0}, {32, 32});
        tileSetInfo->addRegion(BlockType::NOR, {192, 0}, {32, 32});
        tileSetInfo->addRegion(BlockType::XNOR, {224, 0}, {32, 32});
    }
}

void QTRenderer::resize(int w, int h)
{
    this->w = w;
    this->h = h;
}

void QTRenderer::render(QPainter* painter)
{
    // error checking
    if (tileSet.isNull() || tileSetInfo == nullptr)
    {
        painter->drawText(QRect(0, 0, w, h), Qt::AlignCenter, "No tileSet found");
        qDebug() << "ERROR: QTRenderer has no tileSet, cnanot proceed with render.";
        return;
    }

    painter->setRenderHint(QPainter::Antialiasing, true);

    // helper lambda
    auto gridToQt = [&](FPosition position) -> QPoint {
        std::pair<float,float> viewPos = viewManager->gridToView(position);
        return QPoint(viewPos.first * w, viewPos.second * h);
    };

    auto renderBlock = [&](const Block* block) -> void {
        Position gridSize(block->widthNoRotation(), block->heightNoRotation());

        // block
        QPoint topLeft = gridToQt(block->getPosition().free());
        QPoint bottomRight = gridToQt((block->getPosition() + gridSize).free());
        int width = bottomRight.x() - topLeft.x();
        int height = bottomRight.y() - topLeft.y();
        QPoint center = topLeft + QPoint(width / 2, height / 2);

        // get tile set coordinate
        TileRegion tsRegion = tileSetInfo->getRegion(block->type());
        QRect tileSetRect(QPoint(tsRegion.pixelPosition.x, tsRegion.pixelPosition.y),
                           QSize(tsRegion.pixelSize.x, tsRegion.pixelSize.y));

        // get rotation angle
        // TODO - maybe this should be moved somewhere for util, or maybe not
        qreal angle = 0.0;
        switch (block->getRotation())
        {
        case Rotation::ZERO:
            angle = 0.0f;
            break;
        case Rotation::NINETY:
            angle = 90.0f;
            break;
        case Rotation::ONE_EIGHTY:
            angle = 180.0f;
            break;
        case Rotation::TWO_SEVENTY:
            angle = 270.0f;
            break;
        }

        qDebug() << block->getRotation();
        
        // rotate and position painter to center of block
        painter->save();
        painter->translate(center);
        painter->rotate(angle);

        // draw the block from the center
        QRect drawRect = QRect(QPoint(-width/2,-height/2), QSize(width,height));
        painter->drawPixmap(drawRect,
                            tileSet,
                            tileSetRect);
        
        painter->restore();
    };
    
    auto renderCell = [&](FPosition position, BlockType type) -> void {
        QPoint point = gridToQt(position);
        QPoint pointBR = gridToQt(position + FPosition(1.0f, 1.0f));

        TileRegion tsRegion = tileSetInfo->getRegion(type);
        QRectF tileSetRect(QPointF(tsRegion.pixelPosition.x, tsRegion.pixelPosition.y),
                           QSizeF(tsRegion.pixelSize.x, tsRegion.pixelSize.y));
        
        painter->drawPixmap(QRectF(point, pointBR),
                            tileSet,
                            tileSetRect);
    };

    // get bounds
    Position topLeft = viewManager->getTopLeft().snap();
    Position bottomRight = viewManager->getBottomRight().snap();
    
    // render grid
    std::unordered_set<const Block*> blocksToRender;
    for (int c = topLeft.x; c <= bottomRight.x; ++c)
    {
        for (int r = topLeft.y; r <= bottomRight.y; ++r)
        {          
            const Block* block = blockContainer->getBlockContainer()->getBlock(Position(c,r));
            
            if (block) blocksToRender.insert(block);
            else renderCell(FPosition(c,r), BlockType::NONE);
        }
    }

    for (const Block* block : blocksToRender)
    {
        renderBlock(block);
    }
    
    // render blocks
    // render sprites
    // render tints
    // render lines
}

void QTRenderer::setBlockContainer(BlockContainerWrapper* blockContainer)
{
    this->blockContainer = blockContainer;
}

void QTRenderer::updateView(ViewManager* viewManager)
{
    this->viewManager = viewManager;
}

// effects -----------------------------
// line
LineID QTRenderer::addLine(const std::vector<FPosition>& positions, float width)
{
    return 0;
}

void QTRenderer::updateLinePosition(LineID line, int index, FPosition position)
{
    
}

void QTRenderer::updateLinePositions(LineID line, std::vector<FPosition>& positions)
{
    
}

void QTRenderer::updateLineWidth(LineID line, float width)
{
    
}

void QTRenderer::removeLine(LineID line)
{
    
}

// tint
TintID QTRenderer::addTint(Position position, Color color)
{
    return 0;
}

TintID QTRenderer::addTint(FPosition start, float width, float height, Color color)
{
    return 0;
}

void QTRenderer::updateTintColor(TintID tint, Color color)
{
    
}

void QTRenderer::updateTintRect(Position start, float width, float height)
{
    
}

void QTRenderer::removeTint(TintID tint)
{
    
}

// confetti
void QTRenderer::addConfetti(FPosition start)
{
    
}
