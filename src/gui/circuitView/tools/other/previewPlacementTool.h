#ifndef previewPlacementTool_h
#define previewPlacementTool_h

#include "../circuitTool.h"
#include "backend/circuit/parsedCircuit.h"
#include "computerAPI/circuits/circuitFileManager.h"
class Backend;

class PreviewPlacementTool : public CircuitTool {
public:
    PreviewPlacementTool() = default;

    void activate() override {
        CircuitTool::activate();
        registerFunction("tool primary activate", std::bind(&PreviewPlacementTool::commitPlacement, this, std::placeholders::_1));
        registerFunction("tool secondary activate", std::bind(&PreviewPlacementTool::cancelPlacement, this, std::placeholders::_1));
    }
	void updateElements()  override final;

    void loadParsedCircuit(std::shared_ptr<ParsedCircuit> circuitData) {
        parsedCircuit = circuitData;
    }

    void setBackend(Backend* backend){
        this->backend = backend;
    }

    void startPreview() {
        updateElements();
    }

    void reset() override { clearPreview(); }
    void reUse() { usingTool = continueRender = true; }

    bool commitPlacement(const Event* event);
    bool cancelPlacement(const Event* event);

private:
    void clearPreview();
    bool validatePlacement() const;

    std::shared_ptr<ParsedCircuit> parsedCircuit;
    Backend* backend;
    bool usingTool = true;
    bool continueRender = true;
};
#endif /* previewPlacementTool_h */ 
