#include "logicToucher.h"

bool LogicToucher::press(const Event* event) {
	if (!circuit || !evaluatorStateInterface) return false;
	const PositionEvent* positionEvent = event->cast<PositionEvent>();
	if (!positionEvent) return false;

	if (clicked && false) {
		return false;
	} else {
		clickPosition = lastPointerPosition;
		if (circuit->getBlockContainer()->checkCollision(clickPosition)) {
			bool stateToSet = true;
			evaluatorStateInterface->setState(Address(clickPosition), !evaluatorStateInterface->getBoolState(Address(clickPosition)));
		}
		clicked = true;
		return true;
	}
}

bool LogicToucher::unpress(const Event* event) {
	if (!circuit || !evaluatorStateInterface) return false;
	if (clicked) {
		const Block* block = circuit->getBlockContainer()->getBlock(clickPosition);
		if (block && block->type() == BlockType::BUTTON) {
			evaluatorStateInterface->setState(Address(clickPosition), false);
		}
		clicked = false;
		return true;
	}
	return false;
}

bool LogicToucher::pointerMove(const Event* event) {
	if (!circuit || !evaluatorStateInterface) return false;

	if (clicked && lastPointerPosition != clickPosition) {
		const Block* block = circuit->getBlockContainer()->getBlock(clickPosition);
		if (block && block->type() == BlockType::BUTTON) {
			evaluatorStateInterface->setState(Address(clickPosition), false);
		}
		clickPosition = lastPointerPosition;
		if (circuit->getBlockContainer()->checkCollision(clickPosition)) {
			evaluatorStateInterface->setState(Address(clickPosition), !evaluatorStateInterface->getBoolState(Address(clickPosition)));
		}
	}

	return false;
}
