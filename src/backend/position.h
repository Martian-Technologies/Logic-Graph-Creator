#ifndef position_h
#define position_h

#include <string>

#include "../util/fastMath.h"
#include "types.h"

struct Position {
    inline Position() : x(0), y(0) {}
    inline Position(cord_t x, cord_t y) : x(x), y(y) {}

    inline cord_t manhattenDistanceTo(const Position& other) const { return Abs(x - other.x) + Abs(y - other.y); }
    inline cord_t distanceToSquared(const Position& other) const { return IntPower<2>(x - other.x) + IntPower<2>(y - other.y); }
    inline bool operator==(const Position& other) const { return x == other.x && y == other.y; }
    inline bool operator!=(const Position& other) const { return !operator==(other); }
    inline Position operator+(const Position& position) const {return Position(position.x + x, position.y + y);}
    inline Position operator-(const Position& position) const {return Position(position.x - x, position.y - y);}
    inline std::string toString() const {return "(" + std::to_string(x) + ", " + std::to_string(y) + ")";}
    
    cord_t x, y;
};

template<>
struct std::hash<Position> {
    inline std::size_t operator()(const Position& pos) const noexcept {
        std::size_t x = std::hash<cord_t>{}(pos.x);
        std::size_t y = std::hash<cord_t>{}(pos.y);
        return x ^ (y << 1);
    }
};


// ---- we also define block rotation here so ----
enum BlockRotation {
    ZERO = 0,
    NINETY = 1,
    ONE_EIGHTY = 2,
    TWO_SEVENTY = 3,
};

inline bool isRotated(BlockRotation rotation) noexcept {return rotation & 1;}

#endif /* position_h */