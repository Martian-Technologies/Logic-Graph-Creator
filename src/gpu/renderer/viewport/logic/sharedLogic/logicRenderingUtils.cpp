#include "logicRenderingUtils.h"

constexpr float edgeDistance = 0.48f;
constexpr float sideShift = 0.25f;

FVector getOutputOffset(Position position, Circuit* circuit) {
	const Block* block = circuit->getBlockContainer()->getBlock(position);
	return getOutputOffset(block->type(), block->getOrientation());
}

FVector getInputOffset(Position position, Circuit* circuit) {
	const Block* block = circuit->getBlockContainer()->getBlock(position);
	return getInputOffset(block->type(), block->getOrientation());
}

FVector getOutputOffset(BlockType blockType, Orientation orientation) {
	FVector offset = { 0.5, 0.5 };
	if (blockType == BlockType::JUNCTION) return offset;
	
	return offset + orientation * FVector(edgeDistance, sideShift);
}

FVector getInputOffset(BlockType blockType, Orientation orientation) {
	FVector offset = { 0.5, 0.5 };
	if (blockType == BlockType::JUNCTION) return offset;
	
	return offset + orientation * FVector(-edgeDistance, -sideShift);
}
