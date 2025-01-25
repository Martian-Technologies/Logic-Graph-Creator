#ifndef circuitTests_h
#define circuitTests_h

#include <gtest/gtest.h>
#include "backend/circuit/circuit.h"

class CircuitTest: public ::testing::Test {
protected:
    void SetUp() override;
    void TearDown() override;
    SharedCircuit circuit;
    int i;
};

#endif /* circuitTests_h */
