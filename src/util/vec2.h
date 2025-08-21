#ifndef vec2_h
#define vec2_h

#include "fastMath.h"

typedef float vec_coordinate_t;

struct Vec2 {
	inline Vec2() : x(0.0f), y(0.0f) { }
	inline Vec2(vec_coordinate_t x, vec_coordinate_t y) : x(x), y(y) { }

	inline vec_coordinate_t manhattenDistanceTo(const Vec2& other) const { return std::abs(x - other.x) + std::abs(y - other.y); }
	inline vec_coordinate_t distanceToSquared(const Vec2& other) const { return FastPower<2>(x - other.x) + FastPower<2>(y - other.y); }
	bool withinArea(const Vec2& small, const Vec2& large) const { return small.x <= x && small.y <= y && large.x >= x && large.y >= y; }
	inline bool operator==(const Vec2& other) const { return x == other.x && y == other.y; }
	inline bool operator!=(const Vec2& other) const { return !operator==(other); }
	inline Vec2 operator+(const Vec2& position) const { return Vec2(x + position.x, y + position.y); }
	inline Vec2 operator-(const Vec2& position) const { return Vec2(x - position.x, y - position.y); }
	inline Vec2 operator*(vec_coordinate_t scalar) const { return Vec2(x * scalar, y * scalar); }
	inline Vec2& operator+=(const Vec2& position) { x += position.x; y += position.y; return *this; }
	inline Vec2& operator-=(const Vec2& position) { x -= position.x; y -= position.y; return *this; }
	inline Vec2& operator/=(vec_coordinate_t scalar) { x /= scalar; y /= scalar; return *this; }
	inline std::string toString() const { return "(" + std::to_string(x) + ", " + std::to_string(y) + ")"; }

	vec_coordinate_t x, y;
};

typedef int vec_int_coordinate_t;

struct Vec2Int {
	inline Vec2Int() : x(0), y(0) { }
	inline Vec2Int(vec_int_coordinate_t x, vec_int_coordinate_t y) : x(x), y(y) { }

	inline vec_int_coordinate_t manhattenDistanceTo(const Vec2Int& other) const { return abs(x - other.x) + abs(y - other.y); }
	inline vec_int_coordinate_t distanceToSquared(const Vec2Int& other) const { return FastPower<2>(x - other.x) + FastPower<2>(y - other.y); }
	bool withinArea(const Vec2Int& small, const Vec2Int& large) const { return small.x <= x && small.y <= y && large.x >= x && large.y >= y; }
	inline bool operator==(const Vec2Int& other) const { return x == other.x && y == other.y; }
	inline bool operator!=(const Vec2Int& other) const { return !operator==(other); }
	inline Vec2Int operator+(const Vec2Int& position) const { return Vec2Int(x + position.x, y + position.y); }
	inline Vec2Int operator-(const Vec2Int& position) const { return Vec2Int(x - position.x, y - position.y); }
	inline Vec2Int operator*(vec_int_coordinate_t scalar) const { return Vec2Int(x * scalar, y * scalar); }
	inline Vec2Int& operator+=(const Vec2Int& position) { x += position.x; y += position.y; return *this; }
	inline Vec2Int& operator-=(const Vec2Int& position) { x -= position.x; y -= position.y; return *this; }
	inline Vec2Int& operator/=(vec_int_coordinate_t scalar) { x /= scalar; y /= scalar; return *this; }
	inline std::string toString() const { return "(" + std::to_string(x) + ", " + std::to_string(y) + ")"; }

	vec_int_coordinate_t x, y;
};

#endif /* vec2_h */
