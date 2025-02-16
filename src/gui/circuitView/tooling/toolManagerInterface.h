#ifndef toolManagerInterface_h
#define toolManagerInterface_h

#include "circuitTool.h"

class ToolManager;

class ToolManagerInterface {
public:
	inline ToolManagerInterface(ToolManager* toolManager = nullptr) : toolManager(toolManager) { }

	void pushTool(SharedCircuitTool newTool);
	void popTool();

private:
	ToolManager* toolManager;
};

#endif /* toolManagerInterface_h */
