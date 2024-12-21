#include <QApplication>

#include "viewMannager.h"
#include "backend/position/position.h"

void ViewMannager::resize(int width, int height)
{
    screenWidth = width;
    screenHeight = height;
}

void ViewMannager::scroll(float dx, float dy) {
    if (usingMouse) {
        viewHeight *= 1.0f - dy/200.0f;
        lastMouseX = 100.f;
    } else {
        viewCenter.x -= dx*getScreenToView();
        viewCenter.y -= dy*getScreenToView();
    }
    applyLimits();
}

void ViewMannager::pinch(float delta)
{
    viewHeight *= 1.0f + delta;
    applyLimits();
}

bool ViewMannager::mouseDown() {
    if (QGuiApplication::keyboardModifiers().testFlag(Qt::AltModifier)) {
        doMouseMovement = true;
        lastMouseX = 100.f;
        return true;
    }
    return false;
}

bool ViewMannager::mouseUp() {
    if (QGuiApplication::keyboardModifiers().testFlag(Qt::AltModifier) && doMouseMovement) {
        doMouseMovement = false;
        return true;
    }
    return false;
}

bool ViewMannager::mouseMove(float x, float y) {
    x *= getScreenToView();
    y *= getScreenToView();
    
    if (doMouseMovement) {
        if (lastMouseX != 100.0f) {
            viewCenter.x += lastMouseX - x;
            viewCenter.y += lastMouseY - y;
            applyLimits();
        }
        lastMouseX = x;
        lastMouseY = y;
        return true;
    }
    return false;
}

bool ViewMannager::press(int key) {
    switch (key) {
    case Qt::Key_Left:
        movingLeft = true;
        return true;
    case Qt::Key_Right:
        movingRight = true;
        return true;
    case Qt::Key_Up:
        movingUp = true;
        return true;
    case Qt::Key_Down:
        movingDown = true;
        return true;
    }
    return false;
}

bool ViewMannager::release(int key) {
    switch (key) {
    case Qt::Key_Left:
        if (movingLeft) {
            movingLeft = false;
            return true;
        }
        break;
    case Qt::Key_Right:
        if (movingRight) {
            movingRight = false;
            return true;
        }
        break;
    case Qt::Key_Up:
        if (movingUp) {
            movingUp = false;
            return true;
        }
        break;
    case Qt::Key_Down:
        if (movingDown) {
            movingDown = false;
            return true;
        }
        break;
    }
    return false;
}

bool ViewMannager::update(float dt) {
    if (movingLeft || movingRight || movingUp || movingDown) {
        if (movingLeft) viewCenter.x -= speed * dt * getScreenToView();
        if (movingRight) viewCenter.x += speed * dt * getScreenToView();
        if (movingUp) viewCenter.y -= speed * dt * getScreenToView();
        if (movingDown) viewCenter.y += speed * dt * getScreenToView();
        applyLimits();
        return true;
    }
    return false;
}

void ViewMannager::applyLimits() {
    if (viewHeight > 150.0f) viewHeight = 150.0f;
    if (viewHeight < 0.5f) viewHeight = 0.5f;
    if (viewCenter.x > 10000000) viewCenter.x = 10000000;
    if (viewCenter.x < -10000000) viewCenter.x = -10000000;
    if (viewCenter.y > 10000000) viewCenter.y = 10000000;
    if (viewCenter.y < -10000000) viewCenter.y = -10000000;
}

FPosition ViewMannager::viewPos(const FPosition& screenPos) const {
    return (screenPos - FPosition(screenWidth/2.0f, screenHeight/2.0f)) * getScreenToView() + viewCenter;
}

FPosition ViewMannager::screenPos(const FPosition& viewPos) const {
    return (viewPos - viewCenter) * getViewToScreen() + FPosition(screenWidth/2.0f, screenHeight/2.0f);
}
