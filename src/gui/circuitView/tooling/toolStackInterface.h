#ifndef toolStackInterface_h
#define toolStackInterface_h

#include "circuitTool.h"

class ToolStack;

class ToolStackInterface {
public:
	inline ToolStackInterface(ToolStack* toolStack = nullptr) : toolStack(toolStack) { }

	void pushTool(SharedCircuitTool newTool);
	void popTool();

private:
	ToolStack* toolStack;
};

#endif /* toolStackInterface_h */
