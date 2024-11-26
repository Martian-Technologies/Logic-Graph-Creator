#include "blockContainerTools.h"

bool BlockContainerTools::leftPress(Position pos) {
    if (!blockContainer) return false; // if the blockContainer has not been set yet
    switch (tool) {
    case SELECT:
        break;
    case SINGLE_PLACE:
        addMouseAction("place", pos);
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
        // this logic is to allow holding right then left then releasing left and it to start deleting
        switch (mouseActionStack.size()) {
        case 1:
            if (mouseActionStack.back().name == "place") {
                mouseActionStack.pop_back();
                return true;
            }
        case 2:
            if (mouseActionStack.back().name == "place") mouseActionStack.pop_back();
            else {
                mouseActionStack.front() = mouseActionStack.back();
                mouseActionStack.pop_back();
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
        addMouseAction("remove", pos);
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
        // this logic is to allow holding left then right then releasing right and it to start placing
        switch (mouseActionStack.size()) {
        case 1:
            if (mouseActionStack.back().name == "remove") {
                mouseActionStack.pop_back();
                return true;
            }
            break;
        case 2:
            if (mouseActionStack.back().name == "remove") mouseActionStack.pop_back();
            else {
                mouseActionStack.front() = mouseActionStack.back();
                mouseActionStack.pop_back();
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
        if (mouseActionStack.size() > 0) {
            if (mouseActionStack.back().name == "place")
                return selectedBlock != NONE && blockContainer->tryInsertBlock(pos, getBlockClass(selectedBlock));
            return blockContainer->tryRemoveBlock(pos);
        }
    case AREA_PLACE:
        break;
    default:
        break;
    }
    return false;
}

bool BlockContainerTools::keyPress(int keyId) {
    if (!blockContainer) return false; // if the blockContainer has not been set yet
    switch (tool) {
    case SELECT:
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
    default:
        break;
    }
    return false;
}

