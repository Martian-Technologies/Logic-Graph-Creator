#ifndef viewMannager_h
#define viewMannager_h

#include "backend/position/position.h"
#include <functional>

// TODO - there are one million magic numbers that should probably be settings
// TODO - eliminate traces of QT
// TODO - clear up difference between usingMouse, doMouseMovement, etc

class ViewMannager {
public:
    ViewMannager(bool usingMouse, int screenW, int screenH)
		: usingMouse(usingMouse),
          screenHeight(screenH), screenWidth(screenW),
		  viewCenter(0.0f,0.0f), viewHeight(8.0f), mouseInView(false),
          screenMousePosition(0.0f, 0.0f), gridMousePosition(0, 0) {}

    // input events, returning true if the event is accepted (all take in screen space values)
    bool scroll(float dx, float dy);
    bool pinch(float delta);
    bool mouseMove(FPosition position);
    bool mouseDown();
    bool mouseUp();
    bool press(int key);
    bool release(int key);
    void move(float dx, float dy, float dt);
    void mouseEnterView();
    void mouseExitView();
    

    // screen ----
    void resize(int width, int height);
    
    float getScreenWidth() const { return screenWidth; }
    float getScreenHeight() const { return screenHeight; }
    
    // view ------
    inline void setViewCenter(FPosition value) { viewCenter = value; }
    inline void setViewHeight(float value) { viewHeight = value; }

    inline Position getHoverPosition() const { return gridMousePosition; }
    inline FPosition getViewCenter() const { return viewCenter; }
    inline float getAspectRatio() const { return (float)screenWidth / (float)screenHeight; }
    inline float getViewHeight() const { return viewHeight; }
    inline float getViewWidth() const { return viewHeight * getAspectRatio(); }
    
    // coordinate system conversion
    // TODO - generate view matrix
    inline float getViewToScreen() const { return (float)screenHeight / viewHeight; }
    inline float getScreenToView() const { return viewHeight / (float)screenHeight; }
    FPosition viewPos(const FPosition& screenPos) const;
    inline Position gridPos(const FPosition& screenPos) const { return viewPos(screenPos).snap(); }
    FPosition screenPos(const FPosition& viewPos) const;

    // events
    inline void connectViewChanged(const std::function<void()>& func) { viewChangedListener = func; }
    inline void connectHoverChanged(const std::function<void(Position)>& func) { hoverChangedListener = func; }

private:
    void applyLimits();
    void processUpdate(bool viewChanged = true);

    // setting
    float speed = 100.0f;
    bool doMouseMovement = false;
    bool usingMouse;
    bool mouseInView;

    // view
    FPosition viewCenter;
    float viewHeight;
    Position gridMousePosition;

    // screen
    int screenWidth, screenHeight;
    FPosition screenMousePosition;
    
    // event
    std::function<void()> viewChangedListener;
    std::function<void(Position)> hoverChangedListener;
};

#endif /* viewMannager_h */
