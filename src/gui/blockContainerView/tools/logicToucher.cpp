#include "logicToucher.h"
#include <iostream>

bool LogicToucher::press(const Event* event) {
    if (!blockContainer || !evaluatorStateInterface) return false;
    const PositionEvent* positionEvent = event->cast<PositionEvent>();
    if (!positionEvent) return false;

    if (clicked && false) {
        return false;
    }
    else {
        clickPosition = positionEvent->getPosition();
        const Block* block = blockContainer->getBlockContainer()->getBlock(clickPosition);
        if (block) {
            unsigned int v = block->getDataValue<unsigned int, 0>();
            std::cout << v+1 << std::endl;
            blockContainer->trySetBlockDataValue<unsigned int, 0>(clickPosition, v+1);
            // bool stateToSet = true;
            // if (block->type() == BlockType::SWITCH) {
            //     stateToSet = !evaluatorStateInterface->getState(Address(clickPosition));
            // }
            // evaluatorStateInterface->setState(Address(clickPosition), stateToSet);
            // std::cout << "press " << positionEvent->getPosition().toString() << std::endl;
        }
        // clicked = true;
        return true;
    }
}

bool LogicToucher::unpress(const Event* event) {
    if (!blockContainer || !evaluatorStateInterface) return false;
    if (clicked) {
        // const Block* block = blockContainer->getBlockContainer()->getBlock(clickPosition);
        // if (block && block->type() == BlockType::BUTTON) {
        //     evaluatorStateInterface->setState(Address(clickPosition), false);
        //     // std::cout << "unpress " << clickPosition.toString() << std::endl;
        // }
        // clicked = false;
        return true;
    }
    return false;
}

bool LogicToucher::pointerMove(const Event* event) {
    if (!blockContainer || !evaluatorStateInterface) return false;
    const PositionEvent* positionEvent = event->cast<PositionEvent>();
    if (!positionEvent) return false;

    if (clicked && positionEvent->getPosition() != clickPosition) {
        const Block* block = blockContainer->getBlockContainer()->getBlock(clickPosition);
        if (block && block->type() == BlockType::BUTTON) {
            evaluatorStateInterface->setState(Address(clickPosition), false);
            // std::cout << "unpress " << clickPosition.toString() << std::endl;
        }
        clickPosition = positionEvent->getPosition();
        block = blockContainer->getBlockContainer()->getBlock(clickPosition);
        if (block) {
            evaluatorStateInterface->setState(Address(clickPosition), true);
            // std::cout << "press " << clickPosition.toString() << std::endl;
        }
        return true;
    }

    return false;
}
