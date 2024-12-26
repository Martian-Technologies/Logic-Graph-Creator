#include "../../gridGUI/effects/cellSelectionEffect.h"
#include "areaPlaceTool.h"

bool AreaPlaceTool::startPlaceBlock(const Event& event) {
    if (!blockContainer) return false;
    const PositionEvent& positionEvent = event.cast<PositionEvent>();
    switch (click) {
        case 'n':
            click = 'c';
            clickPosition = positionEvent.getPosition();
            return true;
        case 'c':
            blockContainer->tryInsertOverArea(clickPosition, positionEvent.getPosition(), rotation, selectedBlock);
            if (!effectDisplayer.hasEffect(0)) effectDisplayer.addEffect(CellSelectionEffect(0, 0, positionEvent.getPosition()));
            effectDisplayer.getEffect<CellSelectionEffect>(0)->changeSelection(positionEvent.getPosition());
            click = 'n';
            // left here for stats when needed
            // TODO: make a stat tool
            // std::cout << "blocks: " << blockContainer->getBlockCount() << "    cells: " << blockContainer->getCellCount() << std::endl;
            return true;
        default:
            return false;
    }
    return false;
}

bool AreaPlaceTool::startDeleteBlocks(const Event& event) {
    if (!blockContainer) return false;
    const PositionEvent& positionEvent = event.cast<PositionEvent>();
    switch (click) {
        case 'n':
            click = 'c';
            clickPosition = positionEvent.getPosition();
            return true;
        case 'c':
            blockContainer->tryRemoveOverArea(clickPosition, positionEvent.getPosition());
            if (!effectDisplayer.hasEffect(0)) effectDisplayer.addEffect(CellSelectionEffect(0, 0, positionEvent.getPosition()));
            effectDisplayer.getEffect<CellSelectionEffect>(0)->changeSelection(positionEvent.getPosition());
            click = 'n';
            return true;
        default:
            return false;
    }
    return false;
}

bool AreaPlaceTool::pointerMove(const Event& event) {
    if (!blockContainer) return false;
    const PositionEvent& positionEvent = event.cast<PositionEvent>();
    switch (click) {
    case 'n':
        if (!effectDisplayer.hasEffect(0)) effectDisplayer.addEffect(CellSelectionEffect(0, 0, positionEvent.getPosition()));
        effectDisplayer.getEffect<CellSelectionEffect>(0)->changeSelection(positionEvent.getPosition());
        return true;
    default:
        if (!effectDisplayer.hasEffect(0)) effectDisplayer.addEffect(CellSelectionEffect(0, 0, positionEvent.getPosition()));
        effectDisplayer.getEffect<CellSelectionEffect>(0)->changeSelection(clickPosition, positionEvent.getPosition());
        return true;
    }
    return false;
}

bool AreaPlaceTool::enterBlockView(const Event& event) {
    if (!blockContainer) return false;
    const PositionEvent& positionEvent = event.cast<PositionEvent>();
    if (effectDisplayer.hasEffect(0)) return false;
    effectDisplayer.addEffect(CellSelectionEffect(0, 0, positionEvent.getPosition()));
    return true;
}

bool AreaPlaceTool::exitBlockView(const Event& event) {
    if (!blockContainer) return false;
    const PositionEvent& positionEvent = event.cast<PositionEvent>();
    if (!effectDisplayer.hasEffect(0)) return false;
    effectDisplayer.removeEffect(0);
    return true;
}

// bool AreaPlaceTool::keyPress(int keyId) {
//     if (keyId == Qt::Key_Q) {
//         rotation = rotate(rotation, false);
//         return true;
//     }
//     if (keyId == Qt::Key_E) {
//         rotation = rotate(rotation, true);
//         return true;
//     }
//     return false;
// }
