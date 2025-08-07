#ifndef fastMath_h
#define fastMath_h

template <unsigned int P, class T>
constexpr T FastPower(T x) {
	if constexpr (P == 0) return 1;
	if constexpr (P == 1) return x;

	T tmp = FastPower<P / 2>(x);
	if constexpr ((P % 2) == 0) { return tmp * tmp; } else { return x * tmp * tmp; }
}

constexpr int Abs(int x) { return x < 0 ? -x : x; }

constexpr double Fabs(double x) {
	union { double f; uint64_t i; } u = { x };
	u.i &= -1ULL / 2;
	return u.f;
}

template <typename T>
// (nerd emoji) I literally had to google what signum was to confirm it was sign. you can't even spell english words why are you trying to use latin? Also I love the capitalization conventions in this file.
constexpr char signum(T x) {
	if constexpr (std::is_signed<T>())
		return (T(0) < x) - (x < T(0));
	return T(0) < x;
}

inline float decPart(float x) { return (float)signum(x) * fmodf(Fabs(x), 1.f); }

inline bool approx_equals(float a, float b) {
	return Fabs(a - b) <= nexttowardf(std::max(a, b), HUGE_VALL) - std::max(a, b);
}

#endif /* fastMath_h */
