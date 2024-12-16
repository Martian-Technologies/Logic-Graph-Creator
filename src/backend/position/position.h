#ifndef position_h
#define position_h

#include <string>

#include "util/fastMath.h"

typedef int cord_t;

struct Position;
struct FreePosition;

struct Position {
    inline Position() : x(0), y(0) {}
    inline Position(cord_t x, cord_t y) : x(x), y(y) {}

    inline cord_t manhattenDistanceTo(const Position& other) const { return Abs(x - other.x) + Abs(y - other.y); }
    inline cord_t distanceToSquared(const Position& other) const { return IntPower<2>(x - other.x) + IntPower<2>(y - other.y); }
    bool withinArea(const Position& small, const Position& large) const {return small.x <= x && small.y <= y && large.x >= x && large.y >= y;}
	FreePosition free() const;
    inline bool operator==(const Position& other) const { return x == other.x && y == other.y; }
    inline bool operator!=(const Position& other) const { return !operator==(other); }
    inline Position operator+(const Position& position) const { return Position(position.x + x, position.y + y); }
    inline Position operator-(const Position& position) const { return Position(position.x - x, position.y - y); }
    inline std::string toString() const { return "(" + std::to_string(x) + ", " + std::to_string(y) + ")"; }

    cord_t x, y;
};

template<>
struct std::hash<Position> {
    inline std::size_t operator()(const Position& pos) const noexcept {
        std::size_t x = std::hash<cord_t>{}(pos.x);
        std::size_t y = std::hash<cord_t>{}(pos.y);
        return (std::size_t)x ^ ((std::size_t)y << 32);
    }
};

typedef float free_cord_t;

struct FreePosition {
    inline FreePosition() : x(0.0f), y(0.0f) {}
    inline FreePosition(free_cord_t x, free_cord_t y) : x(x), y(y) {}

    inline cord_t manhattenDistanceTo(const FreePosition& other) const { return Abs(x - other.x) + Abs(y - other.y); }
    inline cord_t distanceToSquared(const FreePosition& other) const { return IntPower<2>(x - other.x) + IntPower<2>(y - other.y); }
    bool withinArea(const FreePosition& small, const FreePosition& large) const {return small.x <= x && small.y <= y && large.x >= x && large.y >= y;}
	Position snap() const;
    inline bool operator==(const FreePosition& other) const { return x == other.x && y == other.y; }
    inline bool operator!=(const FreePosition& other) const { return !operator==(other); }
    inline FreePosition operator+(const FreePosition& position) const { return FreePosition(position.x + x, position.y + y); }
    inline FreePosition operator-(const FreePosition& position) const { return FreePosition(position.x - x, position.y - y); }
    inline std::string toString() const { return "(" + std::to_string(x) + ", " + std::to_string(y) + ")"; }

    free_cord_t x, y;
};

// conversion
inline FreePosition Position::free() const { return FreePosition(x, y); }
inline Position FreePosition::snap() const { return Position(std::round(x), std::round(y)); }

// ---- we also define block rotation here so ----
enum Rotation {
    ZERO = 0,
    NINETY = 1,
    ONE_EIGHTY = 2,
    TWO_SEVENTY = 3,
};

inline bool isRotated(Rotation rotation) noexcept { return rotation & 1; }
inline bool isFlipped(Rotation rotation) noexcept { return rotation > 1; }
inline Rotation rotate(Rotation rotation, bool clockWise) {
    if (clockWise) {
        if (rotation == ZERO) return TWO_SEVENTY;
        return (Rotation)((int)rotation - 1);
    }
    if (rotation == TWO_SEVENTY) return ZERO;
    return (Rotation)((int)rotation + 1);
}
#endif /* position_h */
