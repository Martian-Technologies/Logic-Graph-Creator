#include "blockContainerTools.h"

bool BlockContainerTools::leftPress(Position pos) {
    if (!blockContainer) return false; // if the blockContainer has not been set yet
    switch (tool) {
    case SELECT:
        break;
    case SINGLE_PLACE:
        addAction("place", pos);
        return selectedBlock != NONE && blockContainer->tryInsertBlock(pos, getBlockClass(selectedBlock));
    case AREA_PLACE:
        break;
    default:
        break;
    }
    return false;
}

bool BlockContainerTools::leftRelease(Position pos) {
    if (!blockContainer) return false; // if the blockContainer has not been set yet
    switch (tool) {
    case SELECT:
        break;
    case SINGLE_PLACE:
        switch (actionStack.size()) {
        case 1:
            if (actionStack.back().name == "place") {
                actionStack.pop_back();
                return true;
            }
        case 2:
            if (actionStack.back().name == "place") actionStack.pop_back();
            else {
                actionStack.front() = actionStack.back();
                actionStack.pop_back();
            }
            return true;
        }
    case AREA_PLACE:
        break;
    default:
        break;
    }
    return false;
}

bool BlockContainerTools::rightPress(Position pos) {
    if (!blockContainer) return false; // if the blockContainer has not been set yet
    switch (tool) {
    case SELECT:
        break;
    case SINGLE_PLACE:
        addAction("remove", pos);
        return blockContainer->tryRemoveBlock(pos);
    case AREA_PLACE:
        break;
    default:
        break;
    }
    return false;
}

bool BlockContainerTools::rightRelease(Position pos) {
    if (!blockContainer) return false; // if the blockContainer has not been set yet
    switch (tool) {
    case SELECT:
        break;
    case SINGLE_PLACE:
        switch (actionStack.size()) {
        case 1:
            if (actionStack.back().name == "remove") {
                actionStack.pop_back();
                return true;
            }
            break;
        case 2:
            if (actionStack.back().name == "remove") actionStack.pop_back();
            else {
                actionStack.front() = actionStack.back();
                actionStack.pop_back();
            }
            return true;
        }
        break;
    case AREA_PLACE:
        break;
    default:
        break;
    }
    return false;
}

bool BlockContainerTools::mouseMove(Position pos) {
    if (!blockContainer) return false; // if the blockContainer has not been set yet
    switch (tool) {
    case SELECT:
        break;
    case SINGLE_PLACE:
        if (actionStack.size() > 0) {
            if (actionStack.back().name == "place")
                return selectedBlock != NONE && blockContainer->tryInsertBlock(pos, getBlockClass(selectedBlock));
            return blockContainer->tryRemoveBlock(pos);
        }
    case AREA_PLACE:
        break;
    default:
        break;
        return false;
    }
}

bool BlockContainerTools::keyPress(int keyId) {
    if (!blockContainer) return false; // if the blockContainer has not been set yet
    switch (tool) {
    case SELECT:
        break;
    case SINGLE_PLACE:
        break;
    case AREA_PLACE:
        break;
    default:
        break;
    }
    return false;
}

bool BlockContainerTools::keyRelease(int keyId) {
    if (!blockContainer) return false; // if the blockContainer has not been set yet
    switch (tool) {
    case SELECT:
        break;
    case SINGLE_PLACE:
        break;
    case AREA_PLACE:
        break;
    default:
        break;
    }
    return false;
}

