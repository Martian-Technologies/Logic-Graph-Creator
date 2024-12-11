#include <QApplication>

#include "viewMannager.h"

void ViewMannager::scroll(float dx, float dy, float pixToView) {
    if (usingMouse) {
        viewWidth *= 1.f - dy/200.f;
        lastMouseX = 100.f;
    } else {
        viewCenterX -= dx*pixToView;
        viewCenterY -= dy*pixToView;
    }
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
    if (doMouseMovement) {
        if (lastMouseX != 100.f) {
            viewCenterX += lastMouseX - x;
            viewCenterY += lastMouseY - y;
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

bool ViewMannager::update(float dt, float pixToView) {
    if (movingLeft || movingRight || movingUp || movingDown) {
        if (movingLeft) viewCenterX -= speed * dt * pixToView;
        if (movingRight) viewCenterX += speed * dt * pixToView;
        if (movingUp) viewCenterY -= speed * dt * pixToView;
        if (movingDown) viewCenterY += speed * dt * pixToView;
        applyLimits();
        return true;
    }
    return false;
}

void ViewMannager::applyLimits() {
    if (viewWidth > 201) viewWidth = 201;
    if (viewWidth < 0.5f) viewWidth = 0.5f;
    if (viewCenterX > 10000000) viewCenterX = 10000000;
    if (viewCenterX < -10000000) viewCenterX = -10000000;
    if (viewCenterY > 10000000) viewCenterY = 10000000;
    if (viewCenterY < -10000000) viewCenterY = -10000000;
}
