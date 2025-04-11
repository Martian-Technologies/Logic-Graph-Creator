#ifndef positionTests_h
#define positionTests_h

#include <gtest/gtest.h>
#include "backend/position/position.h"

class PositionTest: public ::testing::Test {
protected:
    void SetUp() override;
    void TearDown() override;
};

#endif /* PositionTests_h */
