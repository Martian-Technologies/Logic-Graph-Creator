#include "logicRenderingUtils.h"

constexpr float edgeDistance = 0.48f;
constexpr float sideShift = 0.25f;

FVector getOutputOffset(Position position, Circuit* circuit) {
	return getOutputOffset(circuit->getBlockContainer()->getBlock(position)->getRotation());
}

FVector getInputOffset(Position position, Circuit* circuit) {
	return getInputOffset(circuit->getBlockContainer()->getBlock(position)->getRotation());return getOutputOffset(circuit->getBlockContainer()->getBlock(position)->getRotation());
}

FVector getOutputOffset(Rotation rotation) {
	FVector offset = { 0.5, 0.5 };
	
	switch (rotation) {
	case Rotation::ZERO: offset += { edgeDistance, sideShift }; break;
	case Rotation::NINETY: offset += { -sideShift, edgeDistance }; break;
	case Rotation::ONE_EIGHTY: offset += { -edgeDistance, -sideShift }; break;
	case Rotation::TWO_SEVENTY: offset += { sideShift, -edgeDistance }; break;
	}
	
	return offset;
}

FVector getInputOffset(Rotation rotation) {
	FVector offset = { 0.5, 0.5 };
	
	switch (rotation) {
	case Rotation::ZERO: offset += { -edgeDistance, -sideShift }; break;
	case Rotation::NINETY: offset += { sideShift, -edgeDistance }; break;
	case Rotation::ONE_EIGHTY: offset += { edgeDistance, sideShift }; break;
	case Rotation::TWO_SEVENTY: offset += { -sideShift, edgeDistance }; break;
	}
	
	return offset;
}
