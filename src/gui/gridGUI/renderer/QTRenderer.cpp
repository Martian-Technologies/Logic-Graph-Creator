#include "QTRenderer.h"
#include <qdebug.h>

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

    // render grid
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
    
}

// effects -----------------------------
// line
LineID QTRenderer::addLine(const std::vector<FPosition>& positions, float width)
{
    
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
    
}

TintID QTRenderer::addTint(FPosition start, float width, float height, Color color)
{
    
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
