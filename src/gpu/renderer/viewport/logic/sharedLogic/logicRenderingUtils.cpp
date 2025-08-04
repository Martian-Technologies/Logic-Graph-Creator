#include "logicRenderingUtils.h"

constexpr float edgeDistance = 0.48f;
constexpr float sideShift = 0.25f;

FVector getOutputOffset(Position position, Circuit* circuit) {
	const Block* block = circuit->getBlockContainer()->getBlock(position);
	return getOutputOffset(block->type(), block->getRotation());
}

FVector getInputOffset(Position position, Circuit* circuit) {
	const Block* block = circuit->getBlockContainer()->getBlock(position);
	return getInputOffset(block->type(), block->getRotation());
}

FVector getOutputOffset(BlockType blockType, Rotation rotation) {
	FVector offset = { 0.5, 0.5 };
	if (blockType == BlockType::JUNCTION) return offset;
	
	switch (rotation) {
	case Rotation::ZERO: offset += { edgeDistance, sideShift }; break;
	case Rotation::NINETY: offset += { -sideShift, edgeDistance }; break;
	case Rotation::ONE_EIGHTY: offset += { -edgeDistance, -sideShift }; break;
	case Rotation::TWO_SEVENTY: offset += { sideShift, -edgeDistance }; break;
	}
	
	return offset;
}

FVector getInputOffset(BlockType blockType, Rotation rotation) {
	FVector offset = { 0.5, 0.5 };
	if (blockType == BlockType::JUNCTION) return offset;
	
	switch (rotation) {
	case Rotation::ZERO: offset += { -edgeDistance, -sideShift }; break;
	case Rotation::NINETY: offset += { sideShift, -edgeDistance }; break;
	case Rotation::ONE_EIGHTY: offset += { edgeDistance, sideShift }; break;
	case Rotation::TWO_SEVENTY: offset += { -sideShift, edgeDistance }; break;
	}
	
	return offset;
}
