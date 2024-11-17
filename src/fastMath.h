#ifndef fastMath_h
#define fastMath_h

#include <cstdint>
#include <iostream>

#include <bitset>


template <unsigned int p>
inline int constexpr IntPower(int x) {
    if constexpr (p == 0) return 1;
    if constexpr (p == 1) return x;

    int tmp = IntPower<p / 2>(x);
    if constexpr ((p % 2) == 0) { return tmp * tmp; } else { return x * tmp * tmp; }
}

inline int Abs(int x) {return x < 0 ? -x : x;}

#endif /* fastMath_h */
