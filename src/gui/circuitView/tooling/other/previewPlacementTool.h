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
        registerFunction("pointer move", std::bind(&PreviewPlacementTool::pointerMove, this, std::placeholders::_1));
        registerFunction("view zoom", std::bind(&PreviewPlacementTool::pointerMove, this, std::placeholders::_1));

        registerFunction("tool primary activate", std::bind(&PreviewPlacementTool::commitPlacement, this, std::placeholders::_1));
        registerFunction("tool secondary activate", std::bind(&PreviewPlacementTool::cancelPlacement, this, std::placeholders::_1));
        registerFunction("pointer enter view",std::bind(&PreviewPlacementTool::enterBlockView, this, std::placeholders::_1));
        registerFunction("pointer exit view",std::bind(&PreviewPlacementTool::exitBlockView, this, std::placeholders::_1));
    }

    void loadParsedCircuit(std::shared_ptr<ParsedCircuit> circuitData) {
        parsedCircuit = circuitData;
    }

    void setBackend(Backend* backend){
        this->backend = backend;
    }

    void startPreview() {
        updatePreviewElements();
    }

    void reset() override { clearPreview(); }
    void reUse() { usingTool = continueRender = true; }

    bool pointerMove(const Event* event);
    bool commitPlacement(const Event* event);
    bool cancelPlacement(const Event* event);
    bool enterBlockView(const Event* event);
    bool exitBlockView(const Event* event);

private:
    void updatePreviewElements();
    void clearPreview();
    bool validatePlacement() const;

    std::shared_ptr<ParsedCircuit> parsedCircuit;
    Backend* backend;
    Position currentPosition;
    bool usingTool = true;
    bool continueRender = true;
};
#endif /* previewPlacementTool_h */ 
