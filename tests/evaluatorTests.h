#ifndef evaulatorTests_h
#define evaulatorTests_h

#include <gtest/gtest.h>
#include "backend/evaluator/evaluator.h"
#include "backend/circuit/circuit.h"

class EvaluatorTest : public ::testing::Test {
protected:
    void SetUp() override;
    void TearDown() override;
    void changeState(const Address& addr);
    void readState(const Address& addr);
    SharedCircuit circuit;
    SharedEvaluator evaluator;
    int i;
};

#endif /* evaulatorTests_h */
