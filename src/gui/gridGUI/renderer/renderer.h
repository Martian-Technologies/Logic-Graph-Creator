#ifndef renderer_h
#define renderer_h

#include "backend/defs.h"
#include "backend/position/position.h"
#include "color.h"

#include <vector>

// This is the base class for the renderer objects. Every subclass should
// probably include its own initalization method.

// placeholder system for submitting sprites, not sure how they will be handled
// TODO
// - [ ] Coodinate system conversion
// - [ ] QT renderer
// - [ ] Integration with rest of app


class Renderer
{
 public:
	
	// main flow
	virtual void render() = 0;
	virtual void resize(int w, int h) = 0;

	// submission
	virtual void submitLine(const std::vector<FreePosition>& line, float width) = 0;
	virtual void submitSprite(BlockType type, const FreePosition& position) = 0;
	virtual void submitBlock(BlockType type, const Position& position) = 0;
    virtual void submitTint(const Position& position, Color c, float a) = 0;
}; 

#endif // renderer_h
