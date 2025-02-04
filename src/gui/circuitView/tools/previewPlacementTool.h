#ifndef previewPlacementTool_h
#define previewPlacementTool_h

#include "circuitTool.h"
#include "computerAPI/circuits/circuitFileManager.h"

class PreviewPlacementTool : public CircuitTool {
public:
    PreviewPlacementTool() = default;

    void initialize(ToolManagerEventRegister& toolManagerEventRegister) override {
        CircuitTool::initialize(toolManagerEventRegister);
        toolManagerEventRegister.registerFunction("pointer move", 
            std::bind(&PreviewPlacementTool::pointerMove, this, std::placeholders::_1));
        toolManagerEventRegister.registerFunction("view zoom", 
            std::bind(&PreviewPlacementTool::pointerMove, this, std::placeholders::_1));

        toolManagerEventRegister.registerFunction("tool primary activate", 
            std::bind(&PreviewPlacementTool::commitPlacement, this, std::placeholders::_1));
        toolManagerEventRegister.registerFunction("tool secondary activate", 
            std::bind(&PreviewPlacementTool::cancelPlacement, this, std::placeholders::_1));
        toolManagerEventRegister.registerFunction("pointer enter view",
            std::bind(&PreviewPlacementTool::enterBlockView, this, std::placeholders::_1));
        toolManagerEventRegister.registerFunction("pointer exit view",
            std::bind(&PreviewPlacementTool::exitBlockView, this, std::placeholders::_1));
    }

    void startPreview(const ParsedCircuit& circuitData) {
        parsedCircuit = circuitData;
        parsedCircuit.minPos *= -1;
        updatePreviewElements();
    }

    void reset() override { clearPreview(); }

    bool pointerMove(const Event* event);
    bool commitPlacement(const Event* event);
    bool cancelPlacement(const Event* event);
    bool enterBlockView(const Event* event);
    bool exitBlockView(const Event* event);

private:
    void updatePreviewElements();
    void clearPreview();
    bool validatePlacement() const;

    ParsedCircuit parsedCircuit;
    Position currentPosition;
    bool usingTool = true;
    bool continueRender = true;
};
#endif /* previewPlacementTool_h */ 
