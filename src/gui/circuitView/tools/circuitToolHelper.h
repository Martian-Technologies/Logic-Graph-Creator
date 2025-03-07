#ifndef circuitToolHelper_h
#define circuitToolHelper_h

#include "circuitTool.h"

class ToolStackInterface;

class CircuitToolHelper : public CircuitTool {
public:
	CircuitToolHelper() { helper = true; }
	inline void restart() { reset(); }
};

#endif /* circuitToolHelper_h */
