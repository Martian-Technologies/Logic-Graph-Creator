#ifndef pasteTool_h
#define pasteTool_h

#include "../circuitTool.h"
#include "backend/circuit/parsedCircuit.h"
#include "computerAPI/circuits/circuitFileManager.h"
class Backend;

class PasteTool : public CircuitTool {
public:
    void activate() override final;
	void updateElements() override final;
    bool place(const Event* event);

private:
    bool validatePlacement() const;
};

#endif /* pasteTool_h */ 
