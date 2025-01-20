#ifndef emptyVector_h
#define emptyVector_h

// do not use this will be removed
template <class T>
inline const std::vector<T>& getEmptyVector() {
	static std::vector<T> emptyVector = std::vector<T>();
	return emptyVector;
}

#endif /* emptyVector_h */