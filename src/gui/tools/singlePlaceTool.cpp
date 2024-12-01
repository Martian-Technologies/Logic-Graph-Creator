#include "singlePlaceTool.h"

#include <QApplication>

bool SinglePlaceTool::leftPress(Position pos) {
    if (!blockContainer) return false;
    qDebug() << clicks[0] << clicks[1];
    switch (clicks[0]) {
    case 'n':
        clicks[0] = 'p';
        if (selectedBlock != NONE) blockContainer->tryInsertBlock(pos, getBlockClass(selectedBlock));
        return true;
    case 'p':
        return false;
    case 'r':
        if (clicks[1] == 'n') {
            clicks[1] = 'p';
            if (selectedBlock != NONE) blockContainer->tryInsertBlock(pos, getBlockClass(selectedBlock));
            return true;
        }
        return false;
    }
    return false;
}

bool SinglePlaceTool::leftRelease(Position pos) {
    if (!blockContainer) return false;
    // this logic is to allow holding right then left then releasing left and it to start deleting
    switch (clicks[0]) {
    case 'n':
        return false;
    case 'p':
        if (clicks[1] == 'r') {
            clicks[0] = 'r';
            clicks[1] = 'n';
        } else {
            clicks[0] = 'n';
        }
        return true;
    case 'r':
        if (clicks[1] == 'p') {
            clicks[1] = 'n';
            return true;
        }
        return false;
    }
    return false;
}

bool SinglePlaceTool::rightPress(Position pos) {
    if (!blockContainer) return false;
    switch (clicks[0]) {
    case 'n':
        clicks[0] = 'r';
        blockContainer->tryRemoveBlock(pos);
        return true;
    case 'r':
        return false;
    case 'p':
        if (clicks[1] == 'n') {
            clicks[1] = 'r';
            blockContainer->tryRemoveBlock(pos);
            return true;
        }
        return false;
    }
    return false;
}

bool SinglePlaceTool::rightRelease(Position pos) {
    if (!blockContainer) return false;
    // this logic is to allow holding left then right then releasing right and it to start placing
    switch (clicks[0]) {
    case 'n':
        return false;
    case 'r':
        if (clicks[1] == 'p') {
            clicks[0] = 'p';
            clicks[1] = 'n';
        } else {
            clicks[0] = 'n';
        }
        return true;
    case 'p':
        if (clicks[1] == 'r') {
            clicks[1] = 'n';
            return true;
        }
        return false;
    }
    return false;
}

bool SinglePlaceTool::mouseMove(Position pos) {
    if (!blockContainer) return false;
    switch (clicks[0]) {
    case 'n':
        return false;
    case 'r':
        if (clicks[1] == 'p') {
            if (selectedBlock != NONE) blockContainer->tryInsertBlock(pos, getBlockClass(selectedBlock));
            return selectedBlock != NONE;
        }
        blockContainer->tryRemoveBlock(pos);
        return true;
    case 'p':
        if (clicks[1] == 'r') {
            blockContainer->tryRemoveBlock(pos);
            return true;
        }
        if (selectedBlock != NONE) blockContainer->tryInsertBlock(pos, getBlockClass(selectedBlock));
        return selectedBlock != NONE;
    }
    return false;
}
