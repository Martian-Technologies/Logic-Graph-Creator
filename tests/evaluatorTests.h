#ifndef evaulatorTests_h
#define evaulatorTests_h

#include <gtest/gtest.h>
#include "backend/circuit/circuitManager.h"
#include "backend/evaluator/evaluator.h"

class EvaluatorTest : public ::testing::Test {
protected:
    void SetUp() override;
    void TearDown() override;
    CircuitManager circuitManager;
    SharedCircuit circuit;
    SharedEvaluator evaluator;
    int i;
};

#endif /* evaulatorTests_h */
