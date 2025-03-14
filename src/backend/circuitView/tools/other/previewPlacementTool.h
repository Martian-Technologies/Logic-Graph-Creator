#ifndef previewPlacementTool_h
#define previewPlacementTool_h

#include "../circuitTool.h"
#include "backend/circuit/parsedCircuit.h"
#include "computerAPI/circuits/circuitFileManager.h"
class Backend;

class PreviewPlacementTool : public CircuitTool {
public:
    PreviewPlacementTool() = default;

    void activate() override final {
        CircuitTool::activate();
        registerFunction("tool primary activate", std::bind(&PreviewPlacementTool::commitPlacement, this, std::placeholders::_1));
        registerFunction("tool secondary activate", std::bind(&PreviewPlacementTool::cancelPlacement, this, std::placeholders::_1));
    }
	void updateElements() override final;

    void setParsedCircuit(SharedParsedCircuit circuitData) {
        parsedCircuit = circuitData;
		active = true;
    }

    void startPreview() {
        updateElements();
    }

    void reset() override final { clearPreview(); }

    bool commitPlacement(const Event* event);
    bool cancelPlacement(const Event* event);

private:
    void clearPreview();
    bool validatePlacement() const;

    SharedParsedCircuit parsedCircuit;
    bool active = true;
};

#endif /* previewPlacementTool_h */ 
