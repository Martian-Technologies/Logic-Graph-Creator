#ifndef singleConnectTool_h
#define singleConnectTool_h

#include "../circuitTool.h"

class SingleConnectTool : public CircuitTool {
public:
	void activate() override final {
		CircuitTool::activate();
		registerFunction("tool primary activate", std::bind(&SingleConnectTool::makeConnection, this, std::placeholders::_1));
		registerFunction("tool secondary activate", std::bind(&SingleConnectTool::cancelConnection, this, std::placeholders::_1));
	}

	void updateElements() override final;

	inline void reset() override final { clicked = false; elementCreator.clear(); }
	bool makeConnection(const Event* event);
	bool cancelConnection(const Event* event);

private:
	bool clicked = false;
	Position clickPosition;
};

#endif /* singleConnectTool_h */
