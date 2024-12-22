#include <QApplication>

#include "viewMannager.h"
#include "backend/position/position.h"

void ViewMannager::resize(int width, int height)
{
    screenWidth = width;
    screenHeight = height;

    processUpdate(false);
}

bool ViewMannager::scroll(float dx, float dy) {
    if (usingMouse) {
        viewHeight *= 1.0f - dy/200.0f;
    } else {
        viewCenter.x -= dx*getScreenToView();
        viewCenter.y -= dy*getScreenToView();
    }

    applyLimits();
    processUpdate();

    return true;
}

bool ViewMannager::pinch(float delta)
{
    viewHeight *= 1.0f + delta;

    applyLimits();
    processUpdate();

    return true;
}

bool ViewMannager::mouseDown() {
    if (QGuiApplication::keyboardModifiers().testFlag(Qt::AltModifier)) {
        doMouseMovement = true;
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

bool ViewMannager::mouseMove(FPosition position) {
    FPosition oldMousePosition = screenMousePosition;
    screenMousePosition = position;
    
    if (doMouseMovement) {
        FPosition diff = (oldMousePosition - screenMousePosition) * getScreenToView();
        viewCenter += diff;

        applyLimits();
        processUpdate();
        
        return true;
    }
    else
    {
        processUpdate(false);
        return false;
        
    }
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

void ViewMannager::move(float dx, float dy, float dt)
{
    viewCenter.x += dx * dt;
    viewCenter.y += dy * dt;

    applyLimits();
    processUpdate();
}

void ViewMannager::applyLimits() {
    if (viewHeight > 150.0f) viewHeight = 150.0f;
    if (viewHeight < 0.5f) viewHeight = 0.5f;
    if (viewCenter.x > 10000000) viewCenter.x = 10000000;
    if (viewCenter.x < -10000000) viewCenter.x = -10000000;
    if (viewCenter.y > 10000000) viewCenter.y = 10000000;
    if (viewCenter.y < -10000000) viewCenter.y = -10000000;
}

void ViewMannager::processUpdate(bool viewChanged) {
    Position updatedGridPos = gridPos(screenMousePosition);
    
    if (updatedGridPos != gridMousePosition)
    {
        gridMousePosition = updatedGridPos;

        if (hoverChangedListener) hoverChangedListener(gridMousePosition);
    }
    
    if (viewChanged) viewChangedListener();
}

FPosition ViewMannager::viewPos(const FPosition& screenPos) const {
    return (screenPos - FPosition(screenWidth/2.0f, screenHeight/2.0f)) * getScreenToView() + viewCenter;
}


FPosition ViewMannager::screenPos(const FPosition& viewPos) const {
    return (viewPos - viewCenter) * getViewToScreen() + FPosition(screenWidth/2.0f, screenHeight/2.0f);
}
