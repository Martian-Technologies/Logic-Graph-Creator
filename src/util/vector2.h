#ifndef vector2_h
#define vector2_h

#include "fastMath.h"
#include <string>

typedef float vec_cord_t;

struct Vec2 {
    inline Vec2() : x(0.0f), y(0.0f) {}
    inline Vec2(vec_cord_t x, vec_cord_t y) : x(x), y(y) {}

    inline vec_cord_t manhattenDistanceTo(const Vec2& other) const { return std::abs(x - other.x) + std::abs(y - other.y); }
    inline vec_cord_t distanceToSquared(const Vec2& other) const { return std::pow(x - other.x, 2.0f) + std::pow(y - other.y, 2.0f); }
    bool withinArea(const Vec2& small, const Vec2& large) const {return small.x <= x && small.y <= y && large.x >= x && large.y >= y;}
    inline bool operator==(const Vec2& other) const { return x == other.x && y == other.y; }
    inline bool operator!=(const Vec2& other) const { return !operator==(other); }
    inline Vec2 operator+(const Vec2& position) const { return Vec2(x + position.x, y + position.y); }
    inline Vec2 operator-(const Vec2& position) const { return Vec2(x - position.x, y - position.y); }
    inline Vec2 operator*(vec_cord_t scalar) const { return Vec2(x * scalar, y * scalar); }
    inline Vec2& operator+=(const Vec2& position) { x += position.x; y += position.y; return *this; }
    inline Vec2& operator-=(const Vec2& position) { x -= position.x; y -= position.y; return *this; }
    inline Vec2& operator/=(vec_cord_t scalar) { x /= scalar; y /= scalar; return *this; }
    inline std::string toString() const { return "(" + std::to_string(x) + ", " + std::to_string(y) + ")"; }

    vec_cord_t x, y;
};

typedef int vec_int_cord_t;

struct Vec2Int {
    inline Vec2Int() : x(0), y(0) {}
    inline Vec2Int(vec_int_cord_t x, vec_int_cord_t y) : x(x), y(y) {}

    inline vec_int_cord_t manhattenDistanceTo(const Vec2Int& other) const { return Abs(x - other.x) + Abs(y - other.y); }
    inline vec_int_cord_t distanceToSquared(const Vec2Int& other) const { return IntPower<2>(x - other.x) + IntPower<2>(y - other.y); }
    bool withinArea(const Vec2Int& small, const Vec2Int& large) const {return small.x <= x && small.y <= y && large.x >= x && large.y >= y;}
    inline bool operator==(const Vec2Int& other) const { return x == other.x && y == other.y; }
    inline bool operator!=(const Vec2Int& other) const { return !operator==(other); }
    inline Vec2Int operator+(const Vec2Int& position) const { return Vec2Int(x + position.x, y + position.y); }
    inline Vec2Int operator-(const Vec2Int& position) const { return Vec2Int(x - position.x, y - position.y); }
    inline Vec2Int operator*(vec_int_cord_t scalar) const { return Vec2Int(x * scalar, y * scalar); }
    inline Vec2Int& operator+=(const Vec2Int& position) { x += position.x; y += position.y; return *this; }
    inline Vec2Int& operator-=(const Vec2Int& position) { x -= position.x; y -= position.y; return *this; }
    inline Vec2Int& operator/=(vec_int_cord_t scalar) { x /= scalar; y /= scalar; return *this; }
    inline std::string toString() const { return "(" + std::to_string(x) + ", " + std::to_string(y) + ")"; }

    vec_int_cord_t x, y;
};

#endif
