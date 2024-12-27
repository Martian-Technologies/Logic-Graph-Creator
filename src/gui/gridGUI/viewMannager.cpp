#include <QApplication>
#include <qlogging.h>

#include "viewMannager.h"
#include "backend/position/position.h"

bool ViewMannager::scroll(float dx, float dy) {
    if (scrollZoom) {
        viewHeight *= 1.0f - dy/200.0f;
    } else {
        viewCenter.x -= dx;
        viewCenter.y -= dy;
    }

    applyLimits();
    emitViewChanged();

    return true;
}

bool ViewMannager::pinch(float delta)
{
    viewHeight *= 1.0f + delta;

    applyLimits();
    emitViewChanged();

    return true;
}

bool ViewMannager::pointerDown() {
    if (QGuiApplication::keyboardModifiers().testFlag(Qt::AltModifier)) {
        doPointerMovement = true;
        return true;
    }
    return false;
}

bool ViewMannager::pointerUp() {
    if (QGuiApplication::keyboardModifiers().testFlag(Qt::AltModifier) && doPointerMovement) {
        doPointerMovement = false;
        return true;
    }
    return false;
}

bool ViewMannager::pointerMove(float viewX, float viewY) {
    if (!pointerActive) return false;
    
    float pointerDiffX = pointerViewX - viewX;
    float pointerDiffY = pointerViewY - viewY;
    pointerViewX = viewX;
    pointerViewY = viewY;
    
    if (doPointerMovement) {
        viewCenter.x += pointerDiffX * getViewWidth();
        viewCenter.y += pointerDiffY * getViewHeight();

        applyLimits();
        emitViewChanged();
        
        return true;
    }
    
    return false;
}

bool ViewMannager::press(int key) {
    switch (key) {
        // check for inputs
    }
    return false;
}

bool ViewMannager::release(int key) {
    switch (key) {
        // check for inputs
    }
    
    return false;
}

void ViewMannager::keyMove(float dirX, float dirY, float dt)
{
    float moveAmount = moveSpeed * getViewWidth();
    viewCenter.x += dirX * moveAmount * dt;
    viewCenter.y += dirY * moveAmount * dt;

    applyLimits();
    emitViewChanged();
}

void ViewMannager::pointerEnterView(float viewX, float viewY) {
    pointerActive = true;

    pointerMove(viewX, viewY);
}

void ViewMannager::pointerExitView(float viewX, float viewY) {
    pointerActive = false;

    pointerMove(viewX, viewY);
}

void ViewMannager::applyLimits() {
    if (viewHeight > 150.0f) viewHeight = 150.0f;
    if (viewHeight < 0.5f) viewHeight = 0.5f;
    if (viewCenter.x > 10000000) viewCenter.x = 10000000;
    if (viewCenter.x < -10000000) viewCenter.x = -10000000;
    if (viewCenter.y > 10000000) viewCenter.y = 10000000;
    if (viewCenter.y < -10000000) viewCenter.y = -10000000;
}

std::pair<float, float> ViewMannager::gridToView(FPosition position) const
{
    position -= viewCenter;
    position += FPosition(getViewWidth() / 2.0f, getViewHeight() / 2.0f);
    position.x /= getViewWidth();
    position.y /= getViewHeight();
    return { position.x, position.y };
}
