#ifndef logicRenderingUtils_h
#define logicRenderingUtils_h

#include "backend/circuit/circuit.h"

FVector getOutputOffset(Position position, Circuit* circuit);
FVector getInputOffset(Position position, Circuit* circuit);
FVector getOutputOffset(BlockType blockType, Orientation orientation);
FVector getInputOffset(BlockType blockType, Orientation orientation);

#endif
