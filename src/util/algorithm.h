#ifndef algorithm_h
#define algorithm_h

#include <vector>
#include <iterator>
#include <algorithm>

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
	size_t end = s.find_first_of(delimiter);

	std::vector<std::string> output;

	while (end <= std::string::npos) {
		output.emplace_back(s.substr(start, end - start));

		if (end == std::string::npos)
			break;

		start = end + 1;
		end = s.find_first_of(delimiter, start);
	}

	return output;
}

inline void stringSplitInto(const std::string& s, const char delimiter, std::vector<std::string>& vectorToFill) {
	size_t start = 0;
	size_t end = s.find_first_of(delimiter);

	while (end <= std::string::npos) {
		vectorToFill.emplace_back(s.substr(start, end - start));

		if (end == std::string::npos)
			break;

		start = end + 1;
		end = s.find_first_of(delimiter, start);
	}
}

#endif /* algorithm_h */
