#include "QTRenderer.h"
#include "backend/defs.h"
#include "backend/position/position.h"
#include "gui/gridGUI/renderer/tileSet.h"
#include "util/vector2.h"
#include <memory>
#include <qbrush.h>
#include <qdebug.h>
#include <qlogging.h>
#include <qpainter.h>
#include <qpoint.h>
#include <qsize.h>
#include <qvectornd.h>

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

    // helper lambda
    auto gridToQt = [&](FPosition position) -> QPoint {
        std::pair<float,float> viewPos = viewManager->gridToView(position);
        return QPoint(viewPos.first * w, viewPos.second * h);
    };

    // get bounds
    Position topLeft = viewManager->getTopLeft().snap();
    Position bottomRight = viewManager->getBottomRight().snap();
    
    // render grid
    for (int c = topLeft.x; c <= bottomRight.x; ++c)
    {
        for (int r = topLeft.y; r <= bottomRight.y; ++r)
        {
            QPoint point = gridToQt(FPosition(c, r));
            QPoint pointBR = gridToQt(FPosition(c+1, r+1));

            TileRegion tsRegion = tileSetInfo->getRegion(BlockType::NONE);
            QRectF tileSetRect(QPointF(tsRegion.pixelPosition.x, tsRegion.pixelPosition.y),
                        QSizeF(tsRegion.pixelSize.x, tsRegion.pixelSize.y));
            
            painter->drawPixmap(QRectF(point, pointBR),
                                tileSet,
                                tileSetRect);
        }
    }

    // test grid points
    painter->drawEllipse(gridToQt({0.0f, 0.0f}), 10, 10);
    painter->drawEllipse(gridToQt({0.0f, 3.0f}), 10, 10);
    painter->drawEllipse(gridToQt({2.0f, 1.5f}), 10, 10);
    
    // render blocks
    // render sprites
    // render tints
    // render lines
}

void QTRenderer::setBlockContainer(BlockContainer* blockContainer)
{
    this->blockContainer = blockContainer;
}

void QTRenderer::updateView(ViewMannager* viewManager)
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
