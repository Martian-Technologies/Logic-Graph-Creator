#ifndef viewManager_h
#define viewManager_h

#include "gui/circuitView/events/eventRegister.h"
#include "backend/position/position.h"
#include "util/vec2.h"

// TODO - there are one million magic numbers that should probably be settings
// TODO - eliminate traces of QT
// TODO - clear up difference between usingMouse, doMouseMovement, etc

class ViewManager {
public:
	ViewManager() : viewCenter(), viewHeight(8.0f), aspectRatio(16.0f / 9.0f) { }

	inline void initialize(EventRegister& eventRegister) {
		eventRegister.registerFunction("view zoom", std::bind(&ViewManager::zoom, this, std::placeholders::_1));
		eventRegister.registerFunction("view pan", std::bind(&ViewManager::pan, this, std::placeholders::_1));
		eventRegister.registerFunction("view attach anchor", std::bind(&ViewManager::attachAnchor, this, std::placeholders::_1));
		eventRegister.registerFunction("view dettach anchor", std::bind(&ViewManager::dettachAnchor, this, std::placeholders::_1));
		eventRegister.registerFunction("pointer move", std::bind(&ViewManager::pointerMove, this, std::placeholders::_1));
		eventRegister.registerFunction("pointer enter view", std::bind(&ViewManager::pointerEnterView, this, std::placeholders::_1));
		eventRegister.registerFunction("pointer exit view", std::bind(&ViewManager::pointerExitView, this, std::placeholders::_1));
	}

	bool zoom(const Event* event);
	bool pan(const Event* event);
	bool attachAnchor(const Event* event);
	bool dettachAnchor(const Event* event);
	bool pointerMove(const Event* event);
	bool pointerEnterView(const Event* event);
	bool pointerExitView(const Event* event);

	// view
	inline void setAspectRatio(float value) { aspectRatio = value; viewChanged(); }
	inline void setViewCenter(FPosition value) { viewCenter = value; viewChanged(); }
	inline void setViewHeight(float value) { viewHeight = value; viewChanged(); }

	inline FPosition getViewCenter() const { return viewCenter; }
	inline float getViewHeight() const { return viewHeight; }
	inline float getViewWidth() const { return viewHeight * aspectRatio; }
	inline FPosition getTopLeft() const { return viewCenter - FPosition(getViewWidth() / 2.0f, viewHeight / 2.0f); }
	inline FPosition getBottomRight() const { return viewCenter + FPosition(getViewWidth() / 2.0f, viewHeight / 2.0f); }
	inline const FPosition& getPointerPosition() const { return pointerPosition; }
	inline float getAspectRatio() const { return aspectRatio; }

	// coordinate system conversion
	inline FPosition viewToGrid(Vec2 view) const { return getTopLeft() + FPosition(getViewWidth() * view.x, getViewHeight() * view.y); }
	Vec2 gridToView(FPosition position) const; // temporary until matrix
	// glm::mat4 getViewMatrix() const;
	// glm::mat4 getPerspectiveMatrix() const;

	// events
	inline void connectViewChanged(const std::function<void()>& func) { viewChangedListener = func; }

private:
	void applyLimits();
	inline void viewChanged() { pointerPosition = viewToGrid(pointerViewPosition); if (viewChangedListener) viewChangedListener(); }

	// pointer
	bool doPointerMovement = false;
	bool pointerActive = false;
	FPosition pointerPosition;
	Vec2 pointerViewPosition;

	// view
	FPosition viewCenter;
	float viewHeight;
	float aspectRatio;

	// event
	std::function<void()> viewChangedListener;
};

#endif /* viewManager_h */
