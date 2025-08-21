#ifndef algorithm_h
#define algorithm_h

template <class Iterator, class T>
inline bool contains(Iterator firstIter, Iterator lastIter, const T& value) {
	while (firstIter != lastIter) {
		if (*firstIter == value) return true;
		++firstIter;
	}
	return false;
}

inline std::vector<std::string> stringSplit(const std::string& s, const char delimiter) {
	size_t start = 0;
	size_t end = 0;

	std::vector<std::string> output;

	while (end != std::string::npos) {
		end = s.find_first_of(delimiter, start);
		output.emplace_back(s.substr(start, end - start));
		start = end + 1;
	}

	return output;
}

inline void stringSplitInto(const std::string& s, const char delimiter, std::vector<std::string>& vectorToFill) {
	size_t start = 0;
	size_t end = 0;
	while (end != std::string::npos) {
		end = s.find_first_of(delimiter, start);
		vectorToFill.emplace_back(s.substr(start, end - start));
		start = end + 1;
	}
}

#endif /* algorithm_h */
