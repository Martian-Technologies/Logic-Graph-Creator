#ifndef singleConnectTool_h
#define singleConnectTool_h

#include "circuitTool.h"

class SingleConnectTool : public CircuitTool {
public:
	void initialize(ToolManagerEventRegister& toolManagerEventRegister) override final {
		CircuitTool::initialize(toolManagerEventRegister);
		toolManagerEventRegister.registerFunction("tool primary activate", std::bind(&SingleConnectTool::makeConnection, this, std::placeholders::_1));
		toolManagerEventRegister.registerFunction("tool secondary activate", std::bind(&SingleConnectTool::cancelConnection, this, std::placeholders::_1));
		toolManagerEventRegister.registerFunction("pointer move", std::bind(&SingleConnectTool::pointerMove, this, std::placeholders::_1));
		toolManagerEventRegister.registerFunction("pointer enter view", std::bind(&SingleConnectTool::enterBlockView, this, std::placeholders::_1));
		toolManagerEventRegister.registerFunction("pointer exit view", std::bind(&SingleConnectTool::exitBlockView, this, std::placeholders::_1));
	}

	inline void reset() override final { clicked = false; elementCreator.clear(); }
	bool makeConnection(const Event* event);
	bool cancelConnection(const Event* event);
	bool pointerMove(const Event* event);
	bool enterBlockView(const Event* event);
	bool exitBlockView(const Event* event);

private:
	void updateElements(FPosition pointerPosition);

	bool clicked = false;
	Position clickPosition;
};

#endif /* singleConnectTool_h */
