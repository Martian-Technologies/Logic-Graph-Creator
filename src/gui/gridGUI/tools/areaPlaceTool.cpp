#include "backend/container/blockContainerEditor.h"
#include "../effects/cellSelectionEffect.h"
#include "areaPlaceTool.h"
// #include <iostream> // needed for stat print

bool AreaPlaceTool::leftPress(const Position& pos) {
    if (!blockContainer) return false;
    switch (click) {
        case 'n':
            click = 'c';
            clickPosition = pos;
            return true;
        case 'c':
            BlockContainerEditor::tryInsertOverArea(*blockContainer, clickPosition, pos, rotation, selectedBlock);
            if (!effectDisplayer.hasEffect(0)) effectDisplayer.addEffect(CellSelectionEffect(0, 0, pos));
            effectDisplayer.getEffect<CellSelectionEffect>(0)->changeSelection(pos);
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
            if (!effectDisplayer.hasEffect(0)) effectDisplayer.addEffect(CellSelectionEffect(0, 0, pos));
            effectDisplayer.getEffect<CellSelectionEffect>(0)->changeSelection(pos);
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
        if (!effectDisplayer.hasEffect(0)) effectDisplayer.addEffect(CellSelectionEffect(0, 0, pos));
        effectDisplayer.getEffect<CellSelectionEffect>(0)->changeSelection(pos);
        return true;
    default:
        if (!effectDisplayer.hasEffect(0)) effectDisplayer.addEffect(CellSelectionEffect(0, 0, pos));
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
