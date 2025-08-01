#ifndef logicRenderingUtils_h
#define logicRenderingUtils_h

#include "backend/circuit/circuit.h"

FVector getOutputOffset(Position position, Circuit* circuit);
FVector getInputOffset(Position position, Circuit* circuit);
FVector getOutputOffset(Rotation rotation);
FVector getInputOffset(Rotation rotation);

#endif
