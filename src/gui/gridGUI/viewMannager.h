#ifndef viewMannager_h
#define viewMannager_h

#include "backend/position/position.h"
#include <functional>

// TODO - there are one million magic numbers that should probably be settings
// TODO - eliminate traces of QT
// TODO - clear up difference between usingMouse, doMouseMovement, etc

class ViewMannager {
public:
    ViewMannager(bool scrollZoom, int screenW, int screenH)
		: scrollZoom(scrollZoom),
		  viewCenter(0.0f,0.0f), viewHeight(8.0f), aspectRatio(16.0f/9.0f) {}

    // input events, returning true if the event is accepted
    bool scroll(float dx, float dy); // TODO - I'm unsure of the coordinate system for scroll + pinch, they should take in something view-adjacent to make sure they feel the same at all screen sizes
    bool pinch(float delta);
    bool pointerDown();
    bool pointerUp();
    bool press(int key);
    bool release(int key);
    bool pointerMove(float viewX, float viewY); // this function takes in view coordinates
    void keyMove(float dirX, float dirY, float dt); // this function takes in the input direction
    void pointerEnterView(float viewX, float viewY);
    void pointerExitView(float viewX, float viewY);

    // view
    inline void setAspectRatio(float value) { aspectRatio = value; emitViewChanged(); }
    inline void setViewCenter(FPosition value) { viewCenter = value; emitViewChanged(); }
    inline void setViewHeight(float value) { viewHeight = value; emitViewChanged(); }

    inline FPosition getViewCenter() const { return viewCenter; }
    inline float getViewHeight() const { return viewHeight; }
    inline float getViewWidth() const { return viewHeight * aspectRatio; }
    inline FPosition getTopLeft() const { return viewCenter - FPosition(getViewWidth() / 2.0f, viewHeight / 2.0f); }
    inline FPosition getBottomRight() const { return viewCenter + FPosition(getViewWidth() / 2.0f, viewHeight / 2.0f); }
    inline FPosition getPointerPosition() const { return viewToGrid(pointerViewX, pointerViewY); }
    
    // coordinate system conversion
    inline FPosition viewToGrid(float viewX, float viewY) const { return getTopLeft() + FPosition(getViewWidth() * viewX, getViewHeight() * viewY); }
    std::pair<float, float> gridToView(FPosition position) const; // temporary until matrix
    // glm::mat4 getViewMatrix() const;
    // glm::mat4 getPerspectiveMatrix() const;

    // events
    inline void connectViewChanged(const std::function<void()>& func) { viewChangedListener = func; }

private:
    void applyLimits();
    inline void emitViewChanged() { if (viewChangedListener) viewChangedListener(); }

    // setting
    float moveSpeed = 1.0f;
    bool scrollZoom;

    // pointer
    bool doPointerMovement = false;
    bool pointerActive = false;
    float pointerViewX = 0.0f;
    float pointerViewY = 0.0f;

    // view
    FPosition viewCenter;
    float viewHeight;
    float aspectRatio;
    
    // event
    std::function<void()> viewChangedListener;
};

#endif /* viewMannager_h */
