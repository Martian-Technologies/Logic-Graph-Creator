#ifndef circuitTests_h
#define circuitTests_h

#include <gtest/gtest.h>
#include "backend/circuit/circuitManager.h"

class CircuitTest: public ::testing::Test {
public:
	CircuitTest() : circuitManager(&dataUpdateEventManager) {}

protected:
    void SetUp() override;
    void TearDown() override;
	DataUpdateEventManager dataUpdateEventManager;
	CircuitManager circuitManager;
    SharedCircuit circuit = nullptr;
    int i;
};

#endif /* circuitTests_h */
