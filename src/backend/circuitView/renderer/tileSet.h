#ifndef tileSet_h
#define tileSet_h

#include "util/vec2.h"
#include "backend/evaluator/logicState.h"

class TileSetInfo {
public:
	TileSetInfo(int cellSize, int numCells)
		: cellPixelSize(cellSize, cellSize), numCells(numCells), cellUVSize(1.0f / numCells, 0.5f) { }

	inline Vec2Int getCellPixelSize() const { return cellPixelSize; }
	inline Vec2Int getTopLeftPixel(int index, logic_state_t state) const { return Vec2Int(index * cellPixelSize.x, static_cast<unsigned char>(state) * cellPixelSize.y); }

	inline Vec2 getCellUVSize() const { return cellUVSize; }
	inline Vec2 getBottomLeftUV(int index, bool state) const { return Vec2(index * cellUVSize.x, (!state) * cellUVSize.y); }

private:
	Vec2Int cellPixelSize;
	Vec2 cellUVSize;
	int numCells;
};


#endif /* tileSet_h */
