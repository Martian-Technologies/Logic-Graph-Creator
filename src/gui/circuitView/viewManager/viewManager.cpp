#include <QApplication>

#include "viewManager.h"
#include "gui/circuitView/events/customEvents.h"

bool ViewManager::zoom(const Event* event) {
	const DeltaEvent* deltaEvent = event->cast<DeltaEvent>();
	if (!deltaEvent) return false;

	// adjust zoom level
	viewScale *= std::pow(2.f, -deltaEvent->getDelta());
	applyLimits();

	// keep pointer position the same
	FPosition newPointerPosition = viewToGrid(pointerViewPosition);
	FVector pointerChange = newPointerPosition - pointerPosition;
	viewCenter -= pointerChange;
	applyLimits();

	viewChanged();
	return true;
}

bool ViewManager::pan(const Event* event) {
	const DeltaXYEvent* deltaXYEvent = event->cast<DeltaXYEvent>();
	if (!deltaXYEvent) return false;
	viewCenter.x -= deltaXYEvent->getDeltaX();
	viewCenter.y -= deltaXYEvent->getDeltaY();
	applyLimits();
	viewChanged();
	return true;
}

bool ViewManager::attachAnchor(const Event* event) {
	if (doPointerMovement) return false;
	doPointerMovement = true;
	return true;
}

bool ViewManager::dettachAnchor(const Event* event) {
	if (!doPointerMovement) return false;
	doPointerMovement = false;
	return true;
}

bool ViewManager::pointerMove(const Event* event) {
	if (!pointerActive) return false;
	const PositionEvent* positionEvent = event->cast<PositionEvent>();
	if (!positionEvent) return false;

	pointerViewPosition = gridToView(positionEvent->getFPosition());

	if (doPointerMovement) {
		FVector delta = pointerPosition - positionEvent->getFPosition();
		if (delta.manhattenlength() < 0.001f) return false; // no change in pointer pos
		viewCenter += delta;
		applyLimits();
		viewChanged();
		return false;
	}

	pointerPosition = positionEvent->getFPosition();

	return false;
}

bool ViewManager::pointerEnterView(const Event* event) {
	pointerActive = true;
	pointerMove(event);
	return false;
}

bool ViewManager::pointerExitView(const Event* event) {
	pointerActive = false;
	pointerMove(event);
	return false;
}

void ViewManager::applyLimits() {
	if (viewScale > 150.0f) viewScale = 150.0f;
	if (viewScale < 0.5f) viewScale = 0.5f;
	if (viewCenter.x > 10000000) viewCenter.x = 10000000;
	if (viewCenter.x < -10000000) viewCenter.x = -10000000;
	if (viewCenter.y > 10000000) viewCenter.y = 10000000;
	if (viewCenter.y < -10000000) viewCenter.y = -10000000;
}

Vec2 ViewManager::gridToView(FPosition position) const {
	return Vec2(
		(position.x - viewCenter.x) / getViewWidth() + 0.5f,
		(position.y - viewCenter.y) / getViewHeight() + 0.5f
	);
}

Vec2 ViewManager::gridToView(FVector vector) const {
	return Vec2(
		(vector.dx) / getViewWidth(),
		(vector.dy) / getViewHeight()
	);
}
