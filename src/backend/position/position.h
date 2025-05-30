#ifndef position_h
#define position_h

#include "util/fastMath.h"

typedef int cord_t;
typedef float f_cord_t;

struct Position;
struct FPosition;
struct Vector;
struct FVector;

struct Vector {
	class Iterator;

	inline Vector() noexcept : dx(0), dy(0) { }
	inline Vector(cord_t dx, cord_t dy) noexcept : dx(dx), dy(dy) { }
	// allows the easy creation of vectors that are all the same value
	inline Vector(cord_t d) noexcept : dx(d), dy(d) { }
	inline FVector free() const noexcept;

	inline void extentToFit(Vector vector) noexcept { dx = std::max(dx, vector.dx); dy = std::max(dy, vector.dy); }
	inline std::string toString() const noexcept { return "<" + std::to_string(dx) + ", " + std::to_string(dy) + ">"; }

	inline bool operator==(Vector other) const noexcept { return dx == other.dx && dy == other.dy; }
	inline bool operator!=(Vector other) const noexcept { return !operator==(other); }

	inline bool hasZeros() const noexcept { return !(dx && dy); }
	inline bool widthInSize(Vector size) const noexcept { return dx < size.dx && dy < size.dy; }

	inline cord_t manhattenlength() const noexcept { return Abs(dx) + Abs(dy); }
	inline f_cord_t lengthSquared() const noexcept { return FastPower<2>(dx) + FastPower<2>(dy); }
	inline f_cord_t length() const noexcept { return sqrt(lengthSquared()); }

	inline Vector operator+(Vector other) const noexcept { return Vector(dx + other.dx, dy + other.dy); }
	inline Vector& operator+=(Vector other) noexcept { dx += other.dx; dy += other.dy; return *this; }
	inline Vector operator-(Vector other) const noexcept { return Vector(dx - other.dx, dy - other.dy); }
	inline Vector& operator-=(Vector other) noexcept { dx -= other.dx; dy -= other.dy; return *this; }
	inline cord_t operator*(Vector vector) const noexcept { return dx * vector.dx + dy * vector.dy; }
	inline Vector operator*(cord_t scalar) const noexcept { return Vector(dx * scalar, dy * scalar); }
	inline Vector& operator*=(cord_t scalar) noexcept { dx *= scalar; dy *= scalar; return *this; }
	inline Vector operator/(cord_t scalar) const noexcept { return Vector(dx / scalar, dy / scalar); }
	inline Vector& operator/=(cord_t scalar) noexcept { dx /= scalar; dy /= scalar; return *this; }

	inline Iterator iter() const noexcept;

	cord_t dx, dy;
};

class Vector::Iterator {
public:
	inline Iterator(Vector vector) {
		if (vector == Vector(0)) {
			end = 0;
			width = 1;
			return;
		}
		xNeg = 1 - 2 * (vector.dx < 0);
		width = xNeg * vector.dx + 1;
		yNeg = 1 - 2 * (vector.dy < 0);
		end = (yNeg * vector.dy + 1) * width - 1;
	}
	inline Iterator& operator++() { next(); return *this; }
	inline Iterator& operator--() { prev(); return *this; }
	inline Iterator operator++(int) { Iterator tmp = *this; next(); return tmp; }
	inline Iterator operator--(int) { Iterator tmp = *this; prev(); return tmp; }
	inline explicit operator bool() const { return notDone; }
	inline Vector operator*() const { return  Vector(xNeg * cur % width, yNeg * cur / width); }
	// inline Vector operator->() const { return *(*this); }

private:
	inline void next() {
		notDone = cur != end;
		cur += notDone;
	}
	inline void prev() {
		notDone = true;
		cur -= cur != 0;
	}
	char xNeg;
	char yNeg;
	unsigned int end;
	unsigned int cur = 0;
	unsigned width;
	bool notDone = true;
};

Vector::Iterator Vector::iter() const noexcept { return Iterator(*this); }

template<>
struct std::hash<Vector> {
	inline std::size_t operator()(Vector vec) const noexcept {
		std::size_t x = std::hash<cord_t> {}(vec.dx);
		std::size_t y = std::hash<cord_t> {}(vec.dy);
		return (std::size_t)x ^ ((std::size_t)y << 32);
	}
};

template <>
struct std::formatter<Vector> : std::formatter<std::string> {
	auto format(Vector v, format_context& ctx) const {
		return formatter<string>::format(v.toString(), ctx);
	}
};

struct FVector {
	inline FVector() noexcept : dx(0.0f), dy(0.0f) { }
	inline FVector(f_cord_t dx, f_cord_t dy) noexcept : dx(dx), dy(dy) { }
	// allows the easy creation of fvectors that are all the same value
	inline FVector(f_cord_t d) noexcept : dx(d), dy(d) { }
	inline Vector snap() const noexcept;

	inline std::string toString() const noexcept { return "<" + std::to_string(dx) + ", " + std::to_string(dy) + ">"; }

	inline bool operator==(FVector other) const noexcept { return approx_equals(dx, other.dx) && approx_equals(dy, other.dy); }
	inline bool operator!=(FVector other) const noexcept { return !operator==(other); }

	inline f_cord_t manhattenlength() const noexcept { return fabs(dx) + fabs(dy); }
	inline f_cord_t lengthSquared() const noexcept { return FastPower<2>(dx) + FastPower<2>(dy); }
	inline f_cord_t length() const noexcept { return sqrt(FastPower<2>(dx) + FastPower<2>(dy)); }

	inline FVector operator+(FVector other) const noexcept { return FVector(dx + other.dx, dy + other.dy); }
	inline FVector& operator+=(FVector other) noexcept { dx += other.dx; dy += other.dy; return *this; }
	inline FVector operator-(FVector other) const noexcept { return FVector(dx - other.dx, dy - other.dy); }
	inline FVector& operator-=(FVector other) noexcept { dx -= other.dx; dy -= other.dy; return *this; }
	inline FVector operator*(f_cord_t scalar) const noexcept { return FVector(dx * scalar, dy * scalar); }
	inline FVector& operator*=(f_cord_t scalar) noexcept { dx *= scalar, dy *= scalar; return *this; }
	inline f_cord_t operator*(FVector vector) const noexcept { return dx * vector.dx + dy * vector.dy; }
	inline FVector operator/(f_cord_t scalar) noexcept { return FVector(dx / scalar, dy / scalar); }
	inline FVector& operator/=(f_cord_t scalar) noexcept { dx /= scalar; dy /= scalar; return *this; }

	inline f_cord_t lengthAlongProjectToVec(FVector vector) const noexcept { return (*this * vector) / vector.length(); }
	inline FVector projectToVec(FVector vector) const noexcept { return vector * (*this * vector) / vector.lengthSquared(); }

	f_cord_t dx, dy;
};

template <>
struct std::formatter<FVector> : std::formatter<std::string> {
	auto format(FVector v, format_context& ctx) const {
		return formatter<string>::format(v.toString(), ctx);
	}
};


struct Position {
	class Iterator;

	inline Position() noexcept : x(0), y(0) { }
	inline Position(cord_t x, cord_t y) noexcept : x(x), y(y) { }
	inline FPosition free() const noexcept;

	inline std::string toString() const noexcept { return "(" + std::to_string(x) + ", " + std::to_string(y) + ")"; }

	inline bool operator==(Position position) const noexcept { return x == position.x && y == position.y; }
	inline bool operator!=(Position position) const noexcept { return !operator==(position); }
	inline bool withinArea(Position small, Position large) const noexcept { return small.x <= x && small.y <= y && large.x >= x && large.y >= y; }

	inline cord_t manhattenDistanceTo(Position position) const noexcept { return Abs(x - position.x) + Abs(y - position.y); }
	inline cord_t manhattenDistanceToOrigin() const noexcept { return Abs(x) + Abs(y); }
	inline cord_t distanceToSquared(Position position) const noexcept { return FastPower<2>(x - position.x) + FastPower<2>(y - position.y); }
	inline cord_t distanceToOriginSquared() const noexcept { return FastPower<2>(x) + FastPower<2>(y); }
	inline f_cord_t distanceTo(Position position) const noexcept { return sqrt(FastPower<2>(x - position.x) + FastPower<2>(y - position.y)); }
	inline f_cord_t distanceToOrigin() const noexcept { return sqrt(FastPower<2>(x) + FastPower<2>(y)); }

	inline Position operator+(Vector vector) const noexcept { return Position(x + vector.dx, y + vector.dy); }
	inline Position& operator+=(Vector vector) noexcept { x += vector.dx; y += vector.dy; return *this; }
	inline Vector operator-(Position position) const noexcept { return Vector(x - position.x, y - position.y); }
	inline Position operator-(Vector vector) const noexcept { return Position(x - vector.dx, y - vector.dy); }
	inline Position& operator-=(Vector vector) noexcept { x -= vector.dx; y -= vector.dy; return *this; }

	inline Iterator iterTo(Position other) const noexcept;

	cord_t x, y;
};

class Position::Iterator {
public:
	inline Iterator(Position start, Position end) noexcept {
		if (start == end) {
			this->end = 0;
			this->start = start;
			width = 1;
			return;
		}
		if (start.x > end.x) {
			this->start.x = end.x;
			width = start.x - end.x + 1;
		} else {
			this->start.x = start.x;
			width = end.x - start.x + 1;
		}
		if (start.y > end.y) {
			this->end = (start.y - end.y + 1) * width - 1;
			this->start.y = end.y;
		} else {
			this->end = (end.y - start.y + 1) * width - 1;
			this->start.y = start.y;
		}
	}
	inline Iterator& operator++() noexcept { next(); return *this; }
	inline Iterator& operator--() noexcept { prev(); return *this; }
	inline Iterator operator++(int) noexcept { Iterator tmp = *this; next(); return tmp; }
	inline Iterator operator--(int) noexcept { Iterator tmp = *this; prev(); return tmp; }
	inline explicit operator bool() const noexcept { return notDone; }
	inline const Position operator*() const noexcept { return start + Vector(cur % width, cur / width); }
	inline const Position operator->() const noexcept { return *(*this); }

private:
	inline void next() {
		notDone = cur != end;
		cur += notDone;
	}
	inline void prev() {
		cur -= notDone && cur != 0;
		notDone = true;
	}
	Position start;
	unsigned int end;
	unsigned int cur = 0;
	unsigned width;
	bool notDone = true;
};

Position::Iterator Position::iterTo(Position other) const noexcept { return Iterator(*this, other); }

inline bool areaWithinArea(Position area1Small, Position area1Large, Position area2Small, Position area2Large) {
	return (
		area2Small.withinArea(area1Small, area1Large) ||
		area2Large.withinArea(area1Small, area1Large) ||
		area1Small.withinArea(area2Small, area2Large) ||
		area2Large.withinArea(area2Small, area2Large)
	);
}

template<>
struct std::hash<Position> {
	inline std::size_t operator()(Position pos) const noexcept {
		std::size_t x = std::hash<cord_t> {}(pos.x);
		std::size_t y = std::hash<cord_t> {}(pos.y);
		return (std::size_t)x ^ ((std::size_t)y << 32);
	}
};

template <>
struct std::formatter<Position> : std::formatter<std::string> {
	auto format(Position v, format_context& ctx) const {
		return formatter<string>::format(v.toString(), ctx);
	}
};

struct FPosition {
	inline FPosition() noexcept : x(0.0f), y(0.0f) { }
	inline FPosition(f_cord_t x, f_cord_t y) noexcept : x(x), y(y) { }
	inline Position snap() const noexcept;

	inline std::string toString() const noexcept { return "(" + std::to_string(x) + ", " + std::to_string(y) + ")"; }

	inline bool operator==(FPosition position) const noexcept { return approx_equals(x, position.x) && approx_equals(y, position.y); }
	inline bool operator!=(FPosition position) const noexcept { return !operator==(position); }
	inline bool withinArea(FPosition small, FPosition large) const noexcept { return small.x <= x && small.y <= y && large.x >= x && large.y >= y; }

	inline f_cord_t manhattenDistanceTo(FPosition other) const noexcept { return fabs(x - other.x) + fabs(y - other.y); }
	inline f_cord_t manhattenDistanceToOrigin() const noexcept { return fabs(x) + fabs(y); }
	inline f_cord_t distanceToSquared(FPosition other) const noexcept { return FastPower<2>(x - other.x) + FastPower<2>(y - other.y); }
	inline f_cord_t distanceToOriginSquared() const noexcept { return FastPower<2>(x) + FastPower<2>(y); }
	inline f_cord_t distanceTo(FPosition other) const noexcept { return sqrt(FastPower<2>(x - other.x) + FastPower<2>(y - other.y)); }
	inline f_cord_t distanceToOrigin() const noexcept { return sqrt(FastPower<2>(x) + FastPower<2>(y)); }

	inline FPosition operator+(FVector vector) const noexcept { return FPosition(x + vector.dx, y + vector.dy); }
	inline FPosition& operator+=(FVector vector) noexcept { x += vector.dx; y += vector.dy; return *this; }
	inline FVector operator-(FPosition position) const noexcept { return FVector(x - position.x, y - position.y); }
	inline FPosition operator-(FVector vector) const noexcept { return FPosition(x - vector.dx, y - vector.dy); }
	inline FPosition& operator-=(FVector vector) noexcept { x -= vector.dx; y -= vector.dy; return *this; }
	inline FPosition operator*(f_cord_t scalar) const noexcept { return FPosition(x * scalar, y * scalar); }
	inline f_cord_t lengthAlongProjectToVec(FPosition orginOfVec, FVector vector) const noexcept { return (*this - orginOfVec).lengthAlongProjectToVec(vector); }
	inline FPosition projectToVec(FPosition orginOfVec, FVector vector) const noexcept { return orginOfVec + (*this - orginOfVec).projectToVec(vector); }

	f_cord_t x, y;
};

inline bool areaWithinArea(FPosition area1Small, FPosition area1Large, FPosition area2Small, FPosition area2Large) noexcept {
	return (
		area2Small.withinArea(area1Small, area1Large) ||
		area2Large.withinArea(area1Small, area1Large) ||
		area1Small.withinArea(area2Small, area2Large) ||
		area2Large.withinArea(area2Small, area2Large)
	);
}

template <>
struct std::formatter<FPosition> : std::formatter<std::string> {
	auto format(FPosition v, format_context& ctx) const {
		return formatter<string>::format(v.toString(), ctx);
	}
};

// conversion
inline FPosition Position::free() const noexcept { return FPosition(x, y); }
inline Position FPosition::snap() const noexcept { return Position(downwardFloor(x), downwardFloor(y)); }
inline FVector Vector::free() const noexcept { return FVector(dx, dy); }
inline Vector FVector::snap() const noexcept { return Vector(downwardFloor(dx), downwardFloor(dy)); }


// ---- we also define block rotation here so ----
enum Rotation : char {
	ZERO = 0,
	NINETY = 1,
	ONE_EIGHTY = 2,
	TWO_SEVENTY = 3,
};

inline Vector rotateVector(Vector vector, Rotation rotationAmount) noexcept {
	switch (rotationAmount) {
	case Rotation::TWO_SEVENTY: return Vector(vector.dy, -vector.dx);
	case Rotation::ONE_EIGHTY: return Vector(-vector.dx, -vector.dy);
	case Rotation::NINETY: return Vector(-vector.dy, vector.dx);
	default: return vector;
	}
}
inline Vector rotateSize(Rotation rotationAmount, Vector size) noexcept {
	if (rotationAmount & 1) return Vector(size.dy, size.dx);
	return size;
}
inline constexpr Rotation rotate(Rotation rotation, bool clockWise) {
	if (clockWise) {
		if (rotation == Rotation::TWO_SEVENTY) return Rotation::ZERO;
		return (Rotation)((int)rotation + 1);
	}
	if (rotation == Rotation::ZERO) return Rotation::TWO_SEVENTY;
	return (Rotation)((int)rotation - 1);
}
inline constexpr Rotation addRotations(Rotation rotationA, Rotation rotationB) {
	char output = rotationA + rotationB;
	if ((char)output > (char)Rotation::TWO_SEVENTY)
		output -= 4;
	return (Rotation)output;
}
inline constexpr Rotation rotationNeg(Rotation rotation) { return (Rotation)((4 - (char)rotation) * (char)rotation); }
inline constexpr int getDegrees(Rotation rotation) { return rotation * 90; }
inline Vector rotateVectorWithArea(Vector vector, Vector size, Rotation rotationAmount) {
	switch (rotationAmount) {
	case Rotation::NINETY: return Vector(size.dy - vector.dy - 1, vector.dx);
	case Rotation::ONE_EIGHTY: return Vector(size.dx - vector.dx - 1, size.dy - vector.dy - 1);
	case Rotation::TWO_SEVENTY: return Vector(vector.dy, size.dx - vector.dx - 1);
	default: return vector;
	}
}
inline Vector reverseRotateVectorWithArea(Vector vector, Vector size, Rotation rotationAmount) {
	switch (rotationAmount) {
	case Rotation::NINETY: return Vector(vector.dy, size.dy - vector.dx - 1);
	case Rotation::ONE_EIGHTY: return Vector(size.dx - vector.dx - 1, size.dy - vector.dy - 1);
	case Rotation::TWO_SEVENTY: return Vector(size.dx - vector.dy - 1, vector.dx);
	default: return vector;
	}
}
inline FVector rotateVectorWithArea(FVector vector, FVector size, Rotation rotationAmount) {
	switch (rotationAmount) {
	case Rotation::NINETY: return FVector(size.dy - vector.dy - 1.f, vector.dx);
	case Rotation::ONE_EIGHTY: return FVector(size.dx - vector.dx - 1.f, size.dy - vector.dy - 1.f);
	case Rotation::TWO_SEVENTY: return FVector(vector.dy, size.dx - vector.dx - 1.f);
	default: return vector;
	}
}
inline FVector reverseRotateVectorWithArea(FVector vector, FVector size, Rotation rotationAmount) {
	switch (rotationAmount) {
	case Rotation::NINETY: return FVector(vector.dy, size.dy - vector.dx - 1.f);
	case Rotation::ONE_EIGHTY: return FVector(size.dx - vector.dx - 1.f, size.dy - vector.dy - 1.f);
	case Rotation::TWO_SEVENTY: return FVector(size.dx - vector.dy - 1.f, vector.dx);
	default: return vector;
	}
}

#endif /* position_h */
