#include "areaPlaceTool.h"
#include "../../../backend/blockContainerEditor.h"
#include "../effects/cellSelectionEffect.h"

bool AreaPlaceTool::leftPress(const Position& pos) {
    if (!blockContainer) return false;
    switch (click) {
        case 'n':
            click = 'c';
            clickPosition = pos;
            return true;
        case 'c':
            BlockContainerEditor::tryInsertOverArea(*blockContainer, clickPosition, pos, rotation, selectedBlock);
            click = 'n';
            return true;
        default:
            return false;
    }
    return false;
}

bool AreaPlaceTool::leftRelease(const Position& pos) {
    if (!blockContainer) return false;
    return false;
}

bool AreaPlaceTool::rightPress(const Position& pos) {
    if (!blockContainer) return false;
    switch (click) {
        case 'n':
            click = 'c';
            clickPosition = pos;
            return true;
        case 'c':
            BlockContainerEditor::tryRemoveOverArea(*blockContainer, clickPosition, pos);
            click = 'n';
            return true;
        default:
            return false;
    }
    return false;
}

bool AreaPlaceTool::rightRelease(const Position& pos) {
    if (!blockContainer) return false;
    return false;
}

bool AreaPlaceTool::mouseMove(const Position& pos) {
    if (!blockContainer) return false;
    switch (click) {
    case 'n':
        effectDisplayer.getEffect<CellSelectionEffect>(0)->changeSelection(pos);
        return true;
    default:
        effectDisplayer.getEffect<CellSelectionEffect>(0)->changeSelection(clickPosition, pos);
        return true;
    }
    return false;
}

bool AreaPlaceTool::enterBlockView(const Position& pos) {
    if (!blockContainer) return false;
    if (effectDisplayer.hasEffect(0)) return false;
    effectDisplayer.addEffect(CellSelectionEffect(0, 0, pos));
    return true;
}

bool AreaPlaceTool::exitBlockView(const Position& pos) {
    if (!blockContainer) return false;
    if (!effectDisplayer.hasEffect(0)) return false;
    effectDisplayer.removeEffect(0);
    return true;
}

bool AreaPlaceTool::keyPress(int keyId) {
    if (keyId == Qt::Key_Q) {
        rotation = rotate(rotation, false);
        return true;
    }
    if (keyId == Qt::Key_E) {
        rotation = rotate(rotation, true);
        return true;
    }
    return false;
}
