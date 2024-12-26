#include "tileSet.h"
#include <unordered_map>

template <class T>
void TileSet<T>::addRegion(T key, Vec2Int topLeftPixel, Vec2Int pixelSize)
{
    Vec2Int bottomLeftPixel = Vec2Int(topLeftPixel.x, size.y - topLeftPixel.y - pixelSize.y);
    
    Vec2 topLeftUV = Vec2((float)topLeftPixel.x / size.x, (float)topLeftPixel.y / size.y);
    Vec2 bottomLeftUV = Vec2((float)bottomLeftPixel.x / size.x, (float)bottomLeftPixel.y / size.y);
    Vec2 uvSize = Vec2((float)pixelSize.x / size.x, (float)pixelSize.y / size.y);

    topLeftRegions[key] = TileRegion(topLeftPixel, pixelSize, topLeftUV, uvSize);
    bottomLeftRegions[key] = TileRegion(bottomLeftPixel, pixelSize, bottomLeftUV, uvSize);
}

template <class T>
const TileRegion& TileSet<T>::getRegion(T key, bool topLeft) const
{
    typename std::unordered_map<T, TileRegion>::const_iterator region;
    if (topLeft) {
        region = topLeftRegions.find(key);
        assert(region != topLeftRegions.end()); // no regions found
    }
    else {
        region = bottomLeftRegions.find(key);
        assert(region != bottomLeftRegions.end()); // no regions found
    }

    return region->second;
}
