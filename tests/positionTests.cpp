#include "positionTests.h"
#include "util/uuid.h"

void PositionTest::SetUp() {
}

void PositionTest::TearDown() {

}

TEST_F(PositionTest, VectorBasicOperations) {
    Vector a(5,5);

    ASSERT_TRUE(a == Vector(5,5));
    ASSERT_FALSE(a != Vector(5,5));
}

TEST_F(PositionTest, VectorFunctions) {
    Vector a(5,5);
    std::cout << a.length() << ", " << sqrt(50) << std::endl;
    ASSERT_TRUE(a.toString() == "<5, 5>");
    ASSERT_TRUE(a.manhattenlength() == 10);
    ASSERT_TRUE(a.lengthSquared() == 50);
    ASSERT_TRUE(a.length() == sqrt(50));
    
}

TEST_F(PositionTest, VectorComparisons) {
    Vector a(6,9);
    Vector b(3,3);

    Vector a1 = a;
    Vector a2 = a;
    Vector a3 = a;
    Vector a4 = a;
    a1 += b;
    a2 -= b;
    a3 *= 3;
    a4 /= 3;

    ASSERT_TRUE(a + b == Vector(9,12));
    ASSERT_TRUE(a1 == Vector(9,12));
    ASSERT_TRUE(a - b == Vector(3,6));
    ASSERT_TRUE(a2 == Vector(3,6));
    ASSERT_TRUE(a * 3 == Vector(18,27));
    ASSERT_TRUE(a3 == Vector(18,27));
    ASSERT_TRUE(a / 3 == Vector(2,3));
    ASSERT_TRUE(a / 2 == Vector(3,4));
    ASSERT_TRUE(a4 == Vector(2,3));
}

TEST_F(PositionTest, FVectorBasicOperations) {
    Vector a(5.1,5.2);

    ASSERT_TRUE(a == Vector(5.1,5.2));
    ASSERT_FALSE(a != Vector(5.1,5.2));
}

TEST_F(PositionTest, FVectorFunctions) {
    FVector a(5.1,5.2);

    std::cout << a.manhattenlength() << std::endl;
    ASSERT_TRUE(a.toString() == "<5.100000, 5.200000>");
    ASSERT_TRUE(a.manhattenlength() == 10.3);
    ASSERT_TRUE(a.lengthSquared() == 53.05);
    ASSERT_TRUE(a.length() == sqrt(53.05));
}

TEST_F(PositionTest, FVectorComparisons) {
    FVector a(6.72,9.45);
    FVector b(3.1,3.2);

    FVector a1 = a;
    FVector a2 = a;
    FVector a3 = a;
    FVector a4 = a;
    a1 += b;
    a2 -= b;
    a3 *= 3.0;
    a4 /= 2.1;
    ASSERT_TRUE(a + b == FVector(9.82,12.65));
    ASSERT_TRUE(a1 == FVector(9.82,12.65));
    ASSERT_TRUE(a - b == FVector(3.62,6.25));
    ASSERT_TRUE(a2 == FVector(3.62,6.25));
    std::cout << (a*3).toString() << std::endl;
    ASSERT_TRUE(a * 3.0 == FVector(20.16,28.35));
    ASSERT_TRUE(a * (3.1,3.2) == FVector(20.832,30.24));
    ASSERT_TRUE(a3 == FVector(20.16,28.35));
    ASSERT_TRUE(a / 2.1 == FVector(3.2,4.5));
    ASSERT_TRUE(a4 == FVector(3.2,4.5));
}

TEST_F(PositionTest, PositionBasicOperations) {
    Position a(2,4);

    ASSERT_TRUE(a == Position(2,4));
    ASSERT_FALSE(a != Position(2,4));
    ASSERT_TRUE(a.withinArea(Position(1,1), Position(10,10)));
}

TEST_F(PositionTest, PositionFunctions) {
    Position a(5,5);
    Position b(1,2);

    ASSERT_TRUE(a.toString() == "(5, 5)");
    std::cout << a.manhattenDistanceTo(b) << std::endl;
    ASSERT_TRUE(a.manhattenDistanceTo(b) == 7);
    ASSERT_TRUE(a.manhattenDistanceToOrigin() == 10);
    ASSERT_TRUE(a.distanceToSquared(b) == 25);
    ASSERT_TRUE(a.distanceToOriginSquared() == 50);
    ASSERT_TRUE(a.distanceTo(b) == sqrt(25));
    ASSERT_TRUE(a.distanceToOrigin() == sqrt(50));
}

TEST_F(PositionTest, PositionComparisons) {
    Position a(6,9);
    Vector b(3,3);

    Position a1 = a;
    Position a2 = a;
    a1 += b;
    a2 -= b;

    ASSERT_TRUE(a + b == Position(9,12));
    ASSERT_TRUE(a1 == Position(9,12));
    ASSERT_TRUE(a - Position(2,2) == Vector(4,7));
    ASSERT_TRUE(a - b == Position(3,6));
    ASSERT_TRUE(a2 == Position(3,6));
}

TEST_F(PositionTest, FPositionBasicOperations) {
    FPosition a(2.1,4.2);

    ASSERT_TRUE(a == FPosition(2.1,4.2));
    ASSERT_FALSE(a != FPosition(2.1,4.2));
    ASSERT_TRUE(a.withinArea(FPosition(1.1,1.2), FPosition(9.1,9.2)));
}

TEST_F(PositionTest, FPositionFunctions) {
    FPosition a(5.5,5.4);
    FPosition b(1.1,2.2);

    std::cout << a.toString() << std::endl;
    ASSERT_TRUE(a.toString() == "(5.500000, 5.400000)");
    ASSERT_TRUE(a.manhattenDistanceTo(b) == 7.6);
    ASSERT_TRUE(a.manhattenDistanceToOrigin() == 10.9);
    ASSERT_TRUE(a.distanceToSquared(b) == 29.6);
    ASSERT_TRUE(a.distanceToOriginSquared() == 59.41);
    ASSERT_TRUE(a.distanceTo(b) == sqrt(29.6));
    ASSERT_TRUE(a.distanceToOrigin() == sqrt(59.41));
}

TEST_F(PositionTest, FPositionComparisons) {
    FPosition a(6.4,9.2);
    FVector b(3.2,3.1);

    FPosition a1 = a;
    FPosition a2 = a;
    a1 += b;
    a2 -= b;

    std::cout << (a+b).toString() << std::endl;
    ASSERT_TRUE(a + b == FPosition(9.6,12.3));
    ASSERT_TRUE(a1 == FPosition(9.6,12.3));
    ASSERT_TRUE(a - FPosition(2.1,2.1) == FVector(4.3,7.1));
    ASSERT_TRUE(a - b == FPosition(3.2,6.1));
    ASSERT_TRUE(a2 == FPosition(3.2,6.1));
    ASSERT_TRUE(a*3.1 == FPosition(19.84,28.52));
}