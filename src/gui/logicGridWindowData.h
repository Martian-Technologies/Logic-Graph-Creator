#ifndef logicGridWindowData_h
#define logicGridWindowData_h

#include <QPoint>

#include "../backend/position.h"

class LogicGridWindowData {
public:
    inline LogicGridWindowData(float viewWidth, float width, float height, float viewCenterX, float viewCenterY) :
        viewWidth(viewWidth), width(width), height(height), viewCenterX(viewCenterX), viewCenterY(viewCenterY) {}

    inline float getViewWidth() const { return viewWidth; }
    inline float getViewHeight() const { return height / width * viewWidth; }
    inline float getViewToPix() const { return width / viewWidth; }
    inline float getPixToView() const { return viewWidth / width; }
    inline float getViewCenterX() const { return viewCenterX; }
    inline float getViewCenterY() const { return viewCenterY; }
    inline Position gridPos(QPoint point) const{
        return Position(
            downwardFloor(point.x() * getPixToView() - getViewWidth() / 2.f + viewCenterX),
            downwardFloor(point.y() * getPixToView() - getViewHeight() / 2.f + viewCenterY)
        );
    }
    inline QPoint windowPos(Position point, bool center = false) const {
        return QPoint(
            ((float)point.x + getViewWidth() / 2.f - viewCenterX + center * 0.5f) * getViewToPix(),
            ((float)point.y + getViewHeight() / 2.f - viewCenterY + center * 0.5f) * getViewToPix()
        );
    }

private:
    float viewWidth;
    float width;
    float height;
    float viewCenterX;
    float viewCenterY;
};

#endif /* logicGridWindowData_h */