#ifndef viewMannager_h
#define viewMannager_h

class ViewMannager {
public:
    ViewMannager(bool usingMouse = true)
		: usingMouse(usingMouse),
		  viewCenterX(0), viewCenterY(0), viewWidth(10),
		  movingLeft(false), movingRight(false), movingUp(false), movingDown(false) {}

    inline void pinch(float delta) { viewWidth *= 1 + delta; applyLimits(); }
    void scroll(float dx, float dy, float pixToView);
    bool mouseDown();
    bool mouseUp();
    bool mouseMove(float dx, float dy);
    bool press(int key);
    bool release(int key);

    bool update(float dt, float pixToView);

    inline void setViewCenterX(float value) { viewCenterX = value; }
    inline void setViewCenterY(float value) { viewCenterY = value; }
    inline void setViewWidth(float value) { viewWidth = value; }

    inline float getViewCenterX() const { return viewCenterX; }
    inline float getViewCenterY() const { return viewCenterY; }
    inline float getViewWidth() const { return viewWidth; }

private:
    void applyLimits();

    // setting
    float speed = 100.0f;
    bool doMouseMovement = false;
    bool usingMouse;

    // view
    float viewCenterX;
    float viewCenterY;
    float viewWidth;

    // other
    bool movingLeft;
    bool movingRight;
    bool movingUp;
    bool movingDown;
    float lastMouseX;
    float lastMouseY;
};

#endif /* viewMannager_h */
