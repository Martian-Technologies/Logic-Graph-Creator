#ifndef viewMannager_h
#define viewMannager_h

#include "backend/position/position.h"

// TODO - there are one million magic numbers that should probably be settings
// TODO - eliminate traces of QT

class ViewMannager {
public:
    ViewMannager(bool usingMouse, int screenW, int screenH)
		: usingMouse(usingMouse),
          screenHeight(screenH), screenWidth(screenW),
		  viewCenter({0.0f,0.0f}), viewHeight(8.0f),
		  movingLeft(false), movingRight(false), movingUp(false), movingDown(false) {}

    // inputs (all of these functions take in screen space values)
    void scroll(float dx, float dy);
    bool mouseMove(float dx, float dy);
    void pinch(float delta);
    bool mouseDown();
    bool mouseUp();
    bool press(int key);
    bool release(int key);

    bool update(float dt);

    // screen
    void resize(int width, int height);
    
    float getScreenWidth() const { return screenWidth; }
    float getScreenHeight() const { return screenHeight; }
    
    // view
    inline void setViewCenter(FPosition value) { viewCenter = value; }
    inline void setViewHeight(float value) { viewHeight = value; }

    inline float getAspectRatio() const { return (float)screenWidth / (float)screenHeight; }
    inline FPosition getViewCenter() const { return viewCenter; }
    inline float getViewHeight() const { return viewHeight; }
    inline float getViewWidth() const { return viewHeight * getAspectRatio(); }

    // conversion
    // TODO - generate view matrix
    inline float getViewToScreen() const { return (float)screenHeight / viewHeight; }
    inline float getScreenToView() const { return viewHeight / (float)screenHeight; }
    FPosition viewPos(const FPosition& screenPos) const;
    inline Position gridPos(const FPosition& screenPos) const { return viewPos(screenPos).snap(); }
    FPosition screenPos(const FPosition& viewPos) const;

private:
    void applyLimits();

    // setting
    float speed = 100.0f;
    bool doMouseMovement = false;
    bool usingMouse;

    // view
    FPosition viewCenter;
    float viewHeight;

    // screen
    int screenWidth, screenHeight;

    // other
    bool movingLeft;
    bool movingRight;
    bool movingUp;
    bool movingDown;
    float lastMouseX;
    float lastMouseY;
};

#endif /* viewMannager_h */
