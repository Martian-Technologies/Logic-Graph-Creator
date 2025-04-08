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

	inline Vector() : dx(0), dy(0) { }
	inline Vector(cord_t dx, cord_t dy) : dx(dx), dy(dy) { }
	// allows the easy creation of vectors that are all the same value
	inline Vector(cord_t d) : dx(d), dy(d) { }
	inline FVector free() const;

	inline std::string toString() const { return "<" + std::to_string(dx) + ", " + std::to_string(dy) + ">"; }

	inline bool operator==(const Vector& other) const { return dx == other.dx && dy == other.dy; }
	inline bool operator!=(const Vector& other) const { return !operator==(other); }

	inline cord_t manhattenlength() const { return Abs(dx) + Abs(dy); }
	inline f_cord_t lengthSquared() const { return FastPower<2>(dx) + FastPower<2>(dy); }
	inline f_cord_t length() const { return sqrt(lengthSquared()); }

	inline Vector operator+(const Vector& other) const { return Vector(dx + other.dx, dy + other.dy); }
	inline Vector& operator+=(const Vector& other) { dx += other.dx; dy += other.dy; return *this; }
	inline Vector operator-(const Vector& other) const { return Vector(dx - other.dx, dy - other.dy); }
	inline Vector& operator-=(const Vector& other) { dx -= other.dx; dy -= other.dy; return *this; }
	inline cord_t operator*(const Vector& vector) const { return dx * vector.dx + dy * vector.dy; }
	inline Vector operator*(cord_t scalar) const { return Vector(dx * scalar, dy * scalar); }
	inline Vector& operator*=(cord_t scalar) { dx *= scalar; dy *= scalar; return *this; }
	inline Vector operator/(cord_t scalar) const { return Vector(dx / scalar, dy / scalar); }
	inline Vector& operator/=(cord_t scalar) { dx /= scalar; dy /= scalar; return *this; }

	inline Iterator iter() const;

	cord_t dx, dy;
};

class Vector::Iterator {
public:
	inline Iterator(const Vector& vector) {
		if (vector == Vector(0)) {
			end = 0;
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

Vector::Iterator Vector::iter() const { return Iterator(*this); }

template<>
struct std::hash<Vector> {
	inline std::size_t operator()(const Vector& vec) const noexcept {
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
	inline FVector() : dx(0.0f), dy(0.0f) { }
	inline FVector(f_cord_t dx, f_cord_t dy) : dx(dx), dy(dy) { }
	inline Vector snap() const;

	inline std::string toString() const { return "<" + std::to_string(dx) + ", " + std::to_string(dy) + ">"; }

	inline bool operator==(const FVector& other) const { return dx == other.dx && dy == other.dy; }
	inline bool operator!=(const FVector& other) const { return !operator==(other); }

	inline f_cord_t manhattenlength() const { return fabs(dx) + fabs(dy); }
	inline f_cord_t lengthSquared() const { return FastPower<2>(dx) + FastPower<2>(dy); }
	inline f_cord_t length() const { return sqrt(FastPower<2>(dx) + FastPower<2>(dy)); }

	inline FVector operator+(const FVector& other) const { return FVector(dx + other.dx, dy + other.dy); }
	inline FVector& operator+=(const FVector& other) { dx += other.dx; dy += other.dy; return *this; }
	inline FVector operator-(const FVector& other) const { return FVector(dx - other.dx, dy - other.dy); }
	inline FVector& operator-=(const FVector& other) { dx -= other.dx; dy -= other.dy; return *this; }
	inline FVector operator*(f_cord_t scalar) const { return FVector(dx * scalar, dy * scalar); }
	inline FVector& operator*=(f_cord_t scalar) { ; return *this; }
	inline f_cord_t operator*(const FVector& vector) const { return dx * vector.dx + dy * vector.dy; }
	inline FVector operator/(f_cord_t scalar) { return FVector(dx / scalar, dy / scalar); }
	inline FVector& operator/=(f_cord_t scalar) { dx /= scalar; dy /= scalar; return *this; }

	inline f_cord_t lengthAlongProjectToVec(const FVector& vector) const { return (*this * vector) / vector.length(); }
	inline FVector projectToVec(const FVector& vector) const { return vector * (*this * vector) / vector.lengthSquared(); }

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

	inline Position() : x(0), y(0) { }
	inline Position(cord_t x, cord_t y) : x(x), y(y) { }
	inline FPosition free() const;

	inline std::string toString() const { return "(" + std::to_string(x) + ", " + std::to_string(y) + ")"; }

	inline bool operator==(const Position& position) const { return x == position.x && y == position.y; }
	inline bool operator!=(const Position& position) const { return !operator==(position); }
	inline bool withinArea(const Position& small, const Position& large) const { return small.x <= x && small.y <= y && large.x >= x && large.y >= y; }

	inline cord_t manhattenDistanceTo(const Position& position) const { return Abs(x - position.x) + Abs(y - position.y); }
	inline cord_t manhattenDistanceToOrigin() const { return Abs(x) + Abs(y); }
	inline cord_t distanceToSquared(const Position& position) const { return FastPower<2>(x - position.x) + FastPower<2>(y - position.y); }
	inline cord_t distanceToOriginSquared() const { return FastPower<2>(x) + FastPower<2>(y); }
	inline cord_t distanceTo(const Position& position) const { return sqrt(FastPower<2>(x - position.x) + FastPower<2>(y - position.y)); }
	inline cord_t distanceToOrigin() const { return sqrt(FastPower<2>(x) + FastPower<2>(y)); }

	inline Position operator+(const Vector& vector) const { return Position(x + vector.dx, y + vector.dy); }
	inline Position& operator+=(const Vector& vector) { x += vector.dx; y += vector.dy; return *this; }
	inline Vector operator-(const Position& position) const { return Vector(x - position.x, y - position.y); }
	inline Position operator-(const Vector& vector) const { return Position(x - vector.dx, y - vector.dy); }
	inline Position& operator-=(const Vector& vector) { x -= vector.dx; y -= vector.dy; return *this; }

	inline Iterator iterTo(const Position& other) const;

	cord_t x, y;
};

class Position::Iterator {
public:
	inline Iterator(const Position& start, const Position& end) {
		if (start == end) {
			this->end = 0;
			this->start = start;
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
	inline Iterator& operator++() { next(); return *this; }
	inline Iterator& operator--() { prev(); return *this; }
	inline Iterator operator++(int) { Iterator tmp = *this; next(); return tmp; }
	inline Iterator operator--(int) { Iterator tmp = *this; prev(); return tmp; }
	inline explicit operator bool() const { return notDone; }
	inline const Position operator*() const { return start + Vector(cur % width, cur / width); }
	inline const Position operator->() const { return *(*this); }

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

Position::Iterator Position::iterTo(const Position& other) const { return Iterator(*this, other); }


template<>
struct std::hash<Position> {
	inline std::size_t operator()(const Position& pos) const noexcept {
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
	inline FPosition() : x(0.0f), y(0.0f) { }
	inline FPosition(f_cord_t x, f_cord_t y) : x(x), y(y) { }
	inline Position snap() const;

	inline std::string toString() const { return "(" + std::to_string(x) + ", " + std::to_string(y) + ")"; }

	inline bool operator==(const FPosition& position) const { return x == position.x && y == position.y; }
	inline bool operator!=(const FPosition& position) const { return !operator==(position); }
	inline bool withinArea(const FPosition& small, const FPosition& large) const { return small.x <= x && small.y <= y && large.x >= x && large.y >= y; }

	inline f_cord_t manhattenDistanceTo(const FPosition& other) const { return fabs(x - other.x) + fabs(y - other.y); }
	inline f_cord_t manhattenDistanceToOrigin() const { return fabs(x) + fabs(y); }
	inline f_cord_t distanceToSquared(const FPosition& other) const { return FastPower<2>(x - other.x) + FastPower<2>(y - other.y); }
	inline f_cord_t distanceToOriginSquared() const { return FastPower<2>(x) + FastPower<2>(y); }
	inline f_cord_t distanceTo(const FPosition& other) const { return sqrt(FastPower<2>(x - other.x) + FastPower<2>(y - other.y)); }
	inline f_cord_t distanceToOrigin() const { return sqrt(FastPower<2>(x) + FastPower<2>(y)); }

	inline FPosition operator+(const FVector& vector) const { return FPosition(x + vector.dx, y + vector.dy); }
	inline FPosition& operator+=(const FVector& vector) { x += vector.dx; y += vector.dy; return *this; }
	inline FVector operator-(const FPosition& position) const { return FVector(x - position.x, y - position.y); }
	inline FPosition operator-(const FVector& vector) const { return FPosition(x - vector.dx, y - vector.dy); }
	inline FPosition& operator-=(const FVector& vector) { x -= vector.dx; y -= vector.dy; return *this; }
	inline FPosition operator*(f_cord_t scalar) const { return FPosition(x * scalar, y * scalar); }
	inline f_cord_t lengthAlongProjectToVec(const FPosition& orginOfVec, const FVector& vector) const { return (*this - orginOfVec).lengthAlongProjectToVec(vector); }
	inline FPosition projectToVec(const FPosition& orginOfVec, const FVector& vector) const { return orginOfVec + (*this - orginOfVec).projectToVec(vector); }

	f_cord_t x, y;
};

template <>
struct std::formatter<FPosition> : std::formatter<std::string> {
	auto format(FPosition v, format_context& ctx) const {
		return formatter<string>::format(v.toString(), ctx);
	}
};

// conversion
inline FPosition Position::free() const { return FPosition(x, y); }
inline Position FPosition::snap() const { return Position(downwardFloor(x), downwardFloor(y)); }
inline FVector Vector::free() const { return FVector(dx, dy); }
inline Vector FVector::snap() const { return Vector(downwardFloor(dx), downwardFloor(dy)); }


// ---- we also define block rotation here so ----
enum Rotation : char {
	ZERO = 0,
	NINETY = 1,
	ONE_EIGHTY = 2,
	TWO_SEVENTY = 3,
};

inline Vector rotateSize(Rotation rotation, const Vector& size) noexcept {
	if (rotation & 1) return Vector(size.dy, size.dx);
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
inline constexpr Rotation rotationNeg(Rotation rotation) { return (Rotation)((4 - (char)rotation) * (char)rotation); }
inline constexpr int getDegrees(Rotation rotation) { return rotation * 90; }
inline Vector rotateVectorWithArea(const Vector& vector, const Vector& size, Rotation rotationAmount) {
	switch (rotationAmount) {
	case Rotation::NINETY: return Vector(size.dy - vector.dy - 1, vector.dx);
	case Rotation::ONE_EIGHTY: return Vector(size.dx - vector.dx - 1, size.dy - vector.dy - 1);
	case Rotation::TWO_SEVENTY: return Vector(vector.dy, size.dx - vector.dx - 1);
	default: return vector;
	}
}
inline Vector reverseRotateVectorWithArea(const Vector& vector, const Vector& size, Rotation rotationAmount) {
	switch (rotationAmount) {
	case Rotation::NINETY: return Vector(vector.dy, size.dy - vector.dx - 1);
	case Rotation::ONE_EIGHTY: return Vector(size.dx - vector.dx - 1, size.dy - vector.dy - 1);
	case Rotation::TWO_SEVENTY: return Vector(size.dx - vector.dy - 1, vector.dx);
	default: return vector;
	}
}
#endif /* position_h */
