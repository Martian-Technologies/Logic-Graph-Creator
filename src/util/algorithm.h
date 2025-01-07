#ifndef algorithm_h
#define algorithm_h

#include <algorithm>

template <class Iterator, class T>
inline bool contains(Iterator firstIter, Iterator lastIter, const T& value) {
	while (firstIter != lastIter) {
		if (*firstIter == value) return true;
		++firstIter;
	}
	return false;
}

#endif /* algorithm_h */