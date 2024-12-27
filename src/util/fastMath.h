#ifndef fastMath_h
#define fastMath_h

#include <cstdint>
#include <cmath>

template <unsigned int p, class T>
constexpr int FastPower(T x) {
    if constexpr (p == 0) return 1;
    if constexpr (p == 1) return x;

    int tmp = FastPower<p / 2>(x);
    if constexpr ((p % 2) == 0) { return tmp * tmp; }
    else { return x * tmp * tmp; }
}

constexpr int Abs(int x) { return x < 0 ? -x : x; }

template <typename T>
constexpr char signum(T x) {
    if constexpr (std::is_signed<T>())
        return (T(0) < x) - (x < T(0));
    return T(0) < x;
}

#if __APPLE__
constexpr float decPart(float x) { return (float)signum(x) * fmodf(fabs(x), 1.f); }
#else
inline float decPart(float x) { return (float)signum(x) * fmodf(fabs(x), 1.f); }
#endif

template <typename T>
constexpr int downwardFloor(T x) { return (x < 0) ? (((float)(int)x == x) ? x : (x - 1)) : x; }

constexpr float downwardDecPart(float x) { return x - downwardFloor(x); }


#endif /* fastMath_h */
