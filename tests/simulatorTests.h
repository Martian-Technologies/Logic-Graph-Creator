#ifndef simulatorTest_h
#define simulatorTest_h

#include <gtest/gtest.h>
#include "backend/evaluator/logicSimulator.h"

class SimulatorTest : public ::testing::Test {
protected:
    void SetUp() override;
    void TearDown() override;

    LogicSimulator simulator;
};

#endif /* simulatorTest_h */


