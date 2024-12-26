#ifndef tileSet_h
#define tileSet_h

#include <unordered_map>

#include "util/vector2.h"

// this class represents a free tileset
// tiles don't neccesarily need to fit a grid
// rects are mapped to the type T
// if flip coordinates is true, regions coordinate systems be flipped

struct TileRegion
{
    TileRegion(Vec2Int pixelPosition, Vec2Int pixelSize,
               Vec2 uvPosition, Vec2 uvSize)
        : pixelPosition(pixelPosition), pixelSize(pixelSize),
        uvPosition(uvPosition), uvSize(uvSize) {}
    
    Vec2Int pixelPosition;
    Vec2Int pixelSize;
    Vec2 uvPosition;
    Vec2 uvSize;
};

template <class T>
class TileSet
{
 public:
    TileSet(Vec2Int size)
        : size(size) {}

    void addRegion(T key, Vec2Int topLeft, Vec2Int pixelSize);
    const TileRegion& getRegion(T key, bool topLeft = true) const;
    
 private:
    Vec2Int size;

    std::unordered_map<T, TileRegion> topLeftRegions;
    std::unordered_map<T, TileRegion> bottomLeftRegions;
}; 

#endif
