#include "QTRenderer.h"
#include <qdebug.h>

QTRenderer::QTRenderer()
    : w(0), h(0)
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

void QTRenderer::updateView(ViewMannager* viewManager, int w, int h)
{
    this->w = w; this->h = h;
    
}

void QTRenderer::render()
{
    // error checking
    if (!painter)
    {
        qDebug() << "ERROR: QTRenderer has no painter, cannot proceed with render.";
        return;
    }
    if (tileMap.isNull())
    {
        painter->drawText(QRect(0, 0, w, h), Qt::AlignCenter, "No tileMap found");
        qDebug() << "ERROR: QTRenderer has no tileMap, cnanot proceed with render.";
        return;
    }

    // render grid
    // render blocks
    // render sprites
    // render tints
    // render lines
    
    // QT painters only work for one frame
    painter = nullptr;
}

void QTRenderer::resubmitBlockContainer(BlockContainer* blockContainer)
{
    
}

void QTRenderer::submitLine(const std::vector<FPosition>& line, float width)
{
    
}

void QTRenderer::submitSprite(BlockType type, const FPosition& position)
{
    
}

void QTRenderer::submitBlock(BlockType type, const Position& position)
{
    
}

void QTRenderer::submitTint(const Position &position, Color c, float a)
{
    
}
