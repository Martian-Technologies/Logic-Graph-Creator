#include "QTRenderer.h"
#include "backend/position/position.h"
#include <qbrush.h>
#include <qdebug.h>
#include <qlogging.h>
#include <qpainter.h>
#include <qpoint.h>
#include <qsize.h>

QTRenderer::QTRenderer()
    : w(0), h(0), blockContainer(nullptr)
{
}

void QTRenderer::initialize(const std::string &filePath)
{
    if (filePath != "") {
        tileMap = QPixmap(filePath.c_str());
        
        if (tileMap.isNull()) {
            qDebug() << "ERROR (LogicGridWindow::loadTileMap) was not able to load tileMap" << filePath;
        }
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
    if (tileMap.isNull())
    {
        painter->drawText(QRect(0, 0, w, h), Qt::AlignCenter, "No tileMap found");
        qDebug() << "ERROR: QTRenderer has no tileMap, cnanot proceed with render.";
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

            QRectF gridTileRect(QPointF(0.0f,0.0f),QSizeF(32.0f,32.0f));
            
            painter->drawPixmap(QRectF(point, pointBR),
                                tileMap,
                                gridTileRect);
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
