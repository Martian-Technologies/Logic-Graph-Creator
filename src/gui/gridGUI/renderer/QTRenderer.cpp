#include "QTRenderer.h"
#include <qdebug.h>

void QTRenderer::initialize(const std::string &tileMap) {}

void QTRenderer::render()
{
	if (!painter)
	{
		qDebug() << "ERROR: QTRenderer has no painter, cannot proceed.";
		return;
	}
	// check for tilemap

	// render grid
	// render blocks
	// render sprites
	// render lines
	// render tints
	
	// QT painters only work for one frame
	painter = nullptr;
}

void QTRenderer::submitLine(const std::vector<FreePosition>& line, float width)
{
	
}

void QTRenderer::submitSprite(BlockType type, const FreePosition& position)
{
	
}

void QTRenderer::submitBlock(BlockType type, const Position& position)
{
	
}

void QTRenderer::submitTint(const Position &position, Color c, float a)
{
	
}
