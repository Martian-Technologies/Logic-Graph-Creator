#include "positionTests.h"
#include "util/uuid.h"

void PositionTest::SetUp() { }

void PositionTest::TearDown() {

}

TEST_F(PositionTest, VectorBasicOperations) {
	Vector a(5, 5);

	ASSERT_TRUE(a == Vector(5, 5));
	ASSERT_FALSE(a != Vector(5, 5));
}

TEST_F(PositionTest, VectorFunctions) {
	Vector a(5, 5);
	ASSERT_TRUE(a.toString() == "<5, 5>");
	ASSERT_TRUE(a.manhattenlength() == 10);
	ASSERT_TRUE(approx_equals(a.lengthSquared(), 50.f));
	ASSERT_TRUE(approx_equals(a.length(), sqrt(50)));

}

TEST_F(PositionTest, VectorComparisons) {
	Vector a(6, 9);
	Vector b(3, 3);

	Vector a1 = a;
	Vector a2 = a;
	Vector a3 = a;
	Vector a4 = a;
	a1 += b;
	a2 -= b;
	a3 *= 3;
	a4 /= 3;

	ASSERT_TRUE(a + b == Vector(9, 12));
	ASSERT_TRUE(a1 == Vector(9, 12));
	ASSERT_TRUE(a - b == Vector(3, 6));
	ASSERT_TRUE(a2 == Vector(3, 6));
	ASSERT_TRUE(a * 3 == Vector(18, 27));
	ASSERT_TRUE(a3 == Vector(18, 27));
	ASSERT_TRUE(a / 3 == Vector(2, 3));
	ASSERT_TRUE(a / 2 == Vector(3, 4));
	ASSERT_TRUE(a4 == Vector(2, 3));
}

TEST_F(PositionTest, FVectorBasicOperations) {
	FVector a(5.1f, 5.2f);

	ASSERT_TRUE(a == FVector(5.1f, 5.2f));
	ASSERT_FALSE(a != FVector(5.1f, 5.2f));
}

TEST_F(PositionTest, FVectorFunctions) {
	FVector a(5.1f, 5.2f);

	// Idk if we should test this. Maybe a better way is the read the string in as a new vector and then comapare.
	ASSERT_TRUE(a.toString() == "<5.100000, 5.200000>");

	ASSERT_TRUE(approx_equals(a.manhattenlength(), 10.3f));
	ASSERT_TRUE(approx_equals(a.lengthSquared(), 53.05f));
	ASSERT_TRUE(approx_equals(a.length(), sqrt(53.05f)));
}

TEST_F(PositionTest, FVectorComparisons) {
	FVector a(6.72f, 9.45f);
	FVector b(3.1f, 3.2f);

	FVector a1 = a;
	FVector a2 = a;
	FVector a3 = a;
	FVector a4 = a;
	a1 += b;
	a2 -= b;
	a3 *= 3.0f;
	a4 /= 2.1f;
	ASSERT_TRUE(a + b == FVector(9.82f, 12.65f));
	ASSERT_TRUE(a1 == FVector(9.82f, 12.65f));
	ASSERT_TRUE(a - b == FVector(3.62f, 6.25f));
	ASSERT_TRUE(a2 == FVector(3.62f, 6.25f));
	ASSERT_TRUE(a * 3.0f == FVector(20.16f, 28.35f));
	ASSERT_TRUE(a * 3.1f == FVector(20.832f, 29.295f));
	ASSERT_TRUE(a3 == FVector(20.16f, 28.35f));
	ASSERT_TRUE(a / 2.1f == FVector(3.2f, 4.5f));
	ASSERT_TRUE(a4 == FVector(3.2f, 4.5f));
}

TEST_F(PositionTest, PositionBasicOperations) {
	Position a(2, 4);

	ASSERT_TRUE(a == Position(2, 4));
	ASSERT_FALSE(a != Position(2, 4));
	ASSERT_TRUE(a.withinArea(Position(1, 1), Position(10, 10)));
}

TEST_F(PositionTest, PositionFunctions) {
	Position a(5, 5);
	Position b(1, 2);

	ASSERT_TRUE(a.toString() == "(5, 5)");
	ASSERT_TRUE(a.manhattenDistanceTo(b) == 7);
	ASSERT_TRUE(a.manhattenDistanceToOrigin() == 10);
	ASSERT_TRUE(a.distanceToSquared(b) == 25);
	ASSERT_TRUE(a.distanceToOriginSquared() == 50);
	ASSERT_TRUE(approx_equals(a.distanceTo(b), sqrt(25)));
	ASSERT_TRUE(approx_equals(a.distanceToOrigin(), sqrt(50)));
}

TEST_F(PositionTest, PositionComparisons) {
	Position a(6, 9);
	Vector b(3, 3);

	Position a1 = a;
	Position a2 = a;
	a1 += b;
	a2 -= b;

	ASSERT_TRUE(a + b == Position(9, 12));
	ASSERT_TRUE(a1 == Position(9, 12));
	ASSERT_TRUE(a - Position(2, 2) == Vector(4, 7));
	ASSERT_TRUE(a - b == Position(3, 6));
	ASSERT_TRUE(a2 == Position(3, 6));
}

TEST_F(PositionTest, FPositionBasicOperations) {
	FPosition a(2.1, 4.2);

	ASSERT_TRUE(a == FPosition(2.1, 4.2));
	ASSERT_FALSE(a != FPosition(2.1, 4.2));
	ASSERT_TRUE(a.withinArea(FPosition(1.1, 1.2), FPosition(9.1, 9.2)));
}

TEST_F(PositionTest, FPositionFunctions) {
	FPosition a(5.5, 5.4);
	FPosition b(1.1, 2.2);

	ASSERT_TRUE(a.toString() == "(5.500000, 5.400000)");
	ASSERT_TRUE(approx_equals(a.manhattenDistanceTo(b), 7.6f));
	ASSERT_TRUE(approx_equals(a.manhattenDistanceToOrigin(), 10.9f));
	ASSERT_TRUE(approx_equals(a.distanceToSquared(b), 29.6f));
	ASSERT_TRUE(approx_equals(a.distanceToOriginSquared(), 59.41f));
	ASSERT_TRUE(approx_equals(a.distanceTo(b), sqrt(29.6)));
	ASSERT_TRUE(approx_equals(a.distanceToOrigin(), sqrt(59.41)));
}

TEST_F(PositionTest, FPositionComparisons) {
	FPosition a(6.4, 9.2);
	FVector b(3.2, 3.1);

	FPosition a1 = a;
	FPosition a2 = a;
	a1 += b;
	a2 -= b;

	ASSERT_TRUE(a + b == FPosition(9.6, 12.3));
	ASSERT_TRUE(a1 == FPosition(9.6, 12.3));
	ASSERT_TRUE(a - FPosition(2.1, 2.1) == FVector(4.3, 7.1));
	ASSERT_TRUE(a - b == FPosition(3.2, 6.1));
	ASSERT_TRUE(a2 == FPosition(3.2, 6.1));
	ASSERT_TRUE(a * 3.1 == FPosition(19.84, 28.52));
}