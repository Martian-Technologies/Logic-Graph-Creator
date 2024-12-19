#ifndef renderer_h
#define renderer_h

#include "backend/container/blockContainer.h"
#include "backend/defs.h"
#include "backend/position/position.h"
#include "gui/gridGUI/viewMannager.h"
#include "color.h"

#include <vector>

// This is the base class for the renderer objects. Every subclass should
// probably include its own initalization method.

// placeholder system for submitting sprites using BlockType, not sure how they
// will be handled
// 
// also slightly placeholder system for submission, I'm not
// exactly sure how lines/sprites/tints/etc should be handled. It's probably
// ineffecient to require submission every frame, maybe they will be
// able to be submitted and removed with handle or something

class Renderer
{
 public:
	
	// main flow
	virtual void render() = 0;
    virtual void updateView(ViewMannager* viewManager, int w, int h) = 0;

    // blockContainer submission (every time it needs an update)
    // in the future this might be handled with a diff
    virtual void resubmitBlockContainer(BlockContainer* blockContainer) = 0;
    
	// submission (every render (for now))
	virtual void submitLine(const std::vector<FPosition>& line, float width) = 0;
	virtual void submitSprite(BlockType type, const FPosition& position) = 0;
	virtual void submitBlock(BlockType type, const Position& position) = 0;
    virtual void submitTint(const Position& position, Color c, float a) = 0;
}; 

#endif // renderer_h
