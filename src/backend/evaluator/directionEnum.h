#ifndef directionEnum_h
#define directionEnum_h

enum class Direction {
	IN,
	OUT,
};
inline Direction operator!(Direction dir) {
	return (dir == Direction::IN) ? Direction::OUT : Direction::IN;
}

#endif /* directionEnum_h */
