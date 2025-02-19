#include "previewPlacementTool.h"
#include "backend/backend.h"
#include "gui/circuitView/renderer/elementCreator.h"
#include <QMessageBox>

// Preview is only shown for the primary parsed circuit, not the dependencies that will be created in a different circuit
void PreviewPlacementTool::updatePreviewElements() {
    if (!usingTool || !continueRender) return;

    elementCreator.clear();
    
    // note that getMinPos will return an FVector but will have valid integer coordinates.
    Vector totalOffset = (parsedCircuit->getMinPos()*-1) + Vector(currentPosition.x, currentPosition.y);
    //bool isValid = validatePlacement(); // possible change preview colors if invalid

    // only displays block previews, not connections
    for (const auto& [id, block] : parsedCircuit->getBlocks()) {
        Position displayPos = block.pos.snap() + totalOffset;
        elementCreator.addBlockPreview(BlockPreview(block.type, displayPos, block.rotation));
    }
}

bool PreviewPlacementTool::validatePlacement() const {
    Vector totalOffset = (parsedCircuit->getMinPos()*-1) + Vector(currentPosition.x, currentPosition.y);

    for (const auto& [id, block] : parsedCircuit->getBlocks()) {
        Position testPos = block.pos.snap() + totalOffset;
        if (circuit->getBlockContainer()->checkCollision(testPos)) {
            return false;
        }
    }
    return true;
}

bool PreviewPlacementTool::pointerMove(const Event* event) {
    if (!usingTool) return true;

    if (const PositionEvent* posEvent = event->cast<PositionEvent>()) {
        currentPosition = posEvent->getPosition();
        updatePreviewElements();
        return true;
    }
    return false;
}

// Places the primary parsed circuit in the current circuit.
// Places all dependencies on their own circuits, unless we are merging circuits into one.
bool PreviewPlacementTool::commitPlacement(const Event* event) {
    if (!usingTool) return true;

    if (!validatePlacement()) {
        QMessageBox::warning(nullptr, "Collision Detected", "Cannot place circuit in occupied positions");
        continueRender = true;
        return false;
    }

    Vector totalOffset = (parsedCircuit->getMinPos()*-1) + Vector(currentPosition.x, currentPosition.y);
    std::unordered_map<block_id_t, block_id_t> realIds;

    for (const auto& [oldId, block] : parsedCircuit->getBlocks()) {
        Position targetPos = block.pos.snap() + totalOffset;
        block_id_t newId;
        if (!circuit->tryInsertBlock(targetPos, block.rotation, block.type)) {
            qWarning("Failed to insert block.");
        }
        std::cout << "Inserted block. ID=" << oldId << ", Rot=" << rotationToString(block.rotation) << ", Type=" << blockTypeToString(block.type) <<  std::endl;
        realIds[oldId] = circuit->getBlockContainer()->getBlock(targetPos)->id();;
    }

    for (const auto& conn : parsedCircuit->getConns()) {
        const ParsedCircuit::BlockData* b = parsedCircuit->getBlock(conn.outputBlockId);
        if (!b){
            qWarning("Could not get block from parsed circuit");
            break;
        }
        if (isConnectionInput(b->type, conn.outputId)){
            // skip inputs
            continue;
        }
        std::cout << "connecting [block=" << conn.outputBlockId << ", id=" << conn.outputId << " --> " << "block=" << conn.inputBlockId << ", id=" << conn.inputId << "]\n";

        ConnectionEnd output(realIds[conn.outputBlockId], conn.outputId);
        ConnectionEnd input(realIds[conn.inputBlockId], conn.inputId);
        if (!circuit->tryCreateConnection(output, input)) {
            qWarning("Failed to create connection.");
        }
    }

    // Place all dependencies in their own circuits
    // Note that this is done here so that we recursively place all dependencies of dependencies
    if (backend) {
        std::unordered_map<std::string, std::shared_ptr<ParsedCircuit>> deps = parsedCircuit->getDependencies();
        for (auto itr = deps.begin(); itr != deps.end(); ++itr){
            circuit_id_t id = backend->createCircuit();
            backend->createEvaluator(id);

            Circuit* depCircuit = backend->getCircuit(id).get();

            loadParsedCircuit(itr->second);
            setCircuit(depCircuit);
            commitPlacement(nullptr);
            reUse();

            depCircuit->setSaved();
            depCircuit->setSaveFilePath(itr->second->getFilePath());
            std::cout << "Loaded and marked circuit as saved: " << itr->second->getFilePath() << '\n';
        }
    } else {
        std::cout << "Backed is not initialized to place the dependencies\n";
    }

    clearPreview();
    usingTool = continueRender = false;
    return true;
}

bool PreviewPlacementTool::cancelPlacement(const Event* event) {
    if (!usingTool) return true;

    clearPreview();
    usingTool = continueRender = false;
    return true;
}

void PreviewPlacementTool::clearPreview() {
    elementCreator.clear();
}

bool PreviewPlacementTool::enterBlockView(const Event* event) {
    if (!usingTool) return true;
    continueRender = true;

    if (const PositionEvent* posEvent = event->cast<PositionEvent>()) {
        currentPosition = posEvent->getPosition();
        updatePreviewElements();
        return true;
    }
    return false;
}

bool PreviewPlacementTool::exitBlockView(const Event* event) {
    if (!usingTool) return true;
    continueRender = false;

    elementCreator.clear();
    return true;
}
