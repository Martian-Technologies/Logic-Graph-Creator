#ifndef evaluatorLocalityVectorGenerator_h
#define evaluatorLocalityVectorGenerator_h

#include "evalTypedef.h"

#include "util/localityAllocator.h"

typedef LocalityVectorGenerator<simulator_id_t, 256, 12, 512*sizeof(simulator_id_t)> EvaluatorLocalityVectorGenerator;

#endif /* evaluatorLocalityVectorGenerator_h */
