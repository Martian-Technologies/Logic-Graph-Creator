#ifndef tileSet_h
#define tileSet_h

#include <unordered_map>

#include "util/vec2.h"

// this class represents a free tileset
// tiles don't neccesarily need to fit a grid
// rects are mapped to the type T
// it expects coordinates from top-left, and it can't output either top-left or bottom-left

struct TileRegion {
    TileRegion() : pixelPosition(0, 0), pixelSize(0, 0), uvPosition(0.0f, 0.0f), uvSize(0.0f, 0.0f) {}
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
class TileSet {
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

template <class T>
void TileSet<T>::addRegion(T key, Vec2Int topLeftPixel, Vec2Int pixelSize) {
    // get the position in "bottom-left" coordinate system
    Vec2Int bottomLeftPixel = Vec2Int(topLeftPixel.x, size.y - topLeftPixel.y - pixelSize.y);

    Vec2 topLeftUV = Vec2((float)topLeftPixel.x / size.x, (float)topLeftPixel.y / size.y);
    Vec2 bottomLeftUV = Vec2((float)bottomLeftPixel.x / size.x, (float)bottomLeftPixel.y / size.y);
    Vec2 uvSize = Vec2((float)pixelSize.x / size.x, (float)pixelSize.y / size.y);

    topLeftRegions[key] = TileRegion(topLeftPixel, pixelSize, topLeftUV, uvSize);
    bottomLeftRegions[key] = TileRegion(bottomLeftPixel, pixelSize, bottomLeftUV, uvSize);
}

template <class T>
const TileRegion& TileSet<T>::getRegion(T key, bool topLeft) const {
    typename std::unordered_map<T, TileRegion>::const_iterator region;
    if (topLeft) {
        region = topLeftRegions.find(key);
        assert(region != topLeftRegions.end()); // no regions found
    } else {
        region = bottomLeftRegions.find(key);
        assert(region != bottomLeftRegions.end()); // no regions found
    }

    return region->second;
}


#endif
