#ifndef circuitToolHelper_h
#define circuitToolHelper_h

#include "circuitTool.h"

class ToolManager;
class ToolManagerInterface;

class CircuitToolHelper : public CircuitTool {
public:
	CircuitToolHelper() { helper = true; }
};

#endif /* circuitToolHelper_h */
