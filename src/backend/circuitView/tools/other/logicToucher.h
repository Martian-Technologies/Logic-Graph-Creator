#ifndef logicToucher_h
#define logicToucher_h

#include "../circuitTool.h"

class LogicToucher : public CircuitTool {
public:
	void activate() override final {
		CircuitTool::activate();
		registerFunction("Tool Primary Activate", std::bind(&LogicToucher::press, this, std::placeholders::_1));
		registerFunction("tool primary deactivate", std::bind(&LogicToucher::unpress, this, std::placeholders::_1));
		registerFunction("Pointer Move", std::bind(&LogicToucher::pointerMove, this, std::placeholders::_1));
	}

	inline void reset() override final { clicked = false; elementCreator.clear(); }
	bool press(const Event* event);
	bool unpress(const Event* event);
	bool pointerMove(const Event* event);

private:

	bool clicked = false;
	Position clickPosition;
};

#endif /* logicToucher_h */
