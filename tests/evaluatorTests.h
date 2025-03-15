#ifndef evaulatorTests_h
#define evaulatorTests_h

#include <gtest/gtest.h>
#include "backend/circuit/circuitManager.h"
#include "backend/evaluator/evaluator.h"

class EvaluatorTest : public ::testing::Test {
public:
	EvaluatorTest() : circuitManager(&dataUpdateEventManager) {}

protected:
    void SetUp() override;
    void TearDown() override;
	DataUpdateEventManager dataUpdateEventManager;
    CircuitManager circuitManager;
    SharedCircuit circuit;
    SharedEvaluator evaluator;
    int i;
};

#endif /* evaulatorTests_h */
