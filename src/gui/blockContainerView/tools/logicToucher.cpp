#include "logicToucher.h"
#include <iostream>

bool LogicToucher::press(const Event* event) {
	if (!blockContainer || !evaluatorStateInterface) return false;
	const PositionEvent* positionEvent = event->cast<PositionEvent>();
	if (!positionEvent) return false;

	if (clicked && false) {
		return false;
	} else {
		clickPosition = positionEvent->getPosition();
		const Block* block = blockContainer->getBlockContainer()->getBlock(clickPosition);
		if (block) {
			bool stateToSet = true;
			evaluatorStateInterface->setState(Address(clickPosition), !evaluatorStateInterface->getState(Address(clickPosition)));
			// std::cout << "press " << positionEvent->getPosition().toString() << std::endl;
		}
		clicked = true;
		return true;
	}
}

bool LogicToucher::unpress(const Event* event) {
	if (!blockContainer || !evaluatorStateInterface) return false;
	if (clicked) {
		const Block* block = blockContainer->getBlockContainer()->getBlock(clickPosition);
		if (block && block->type() == BlockType::BUTTON) {
			evaluatorStateInterface->setState(Address(clickPosition), false);
			// std::cout << "unpress " << clickPosition.toString() << std::endl;
		}
		clicked = false;
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
			evaluatorStateInterface->setState(Address(clickPosition), !evaluatorStateInterface->getState(Address(clickPosition)));
			// std::cout << "press " << clickPosition.toString() << std::endl;
		}
		return true;
	}

	return false;
}
