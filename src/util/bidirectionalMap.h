#ifndef bidirectionalMap_h
#define bidirectionalMap_h

template <class T1, class T2>
class BidirectionalMap {
public:
	void set(const T1& key, const T2& value) {
		T2Vals[key] = value;
		T1Vals[key] = value;
	}

	const T2* const get(const T1& key) {
		auto iter = T2Vals.find(key);
		if (iter == T2Vals.end()) return nullptr;
		return &(iter->second);
	}
	const T1* const get(const T2& key) {
		auto iter = T1Vals.find(key);
		if (iter == T1Vals.end()) return nullptr;
		return &(iter->second);
	}
	
private:
	std::unordered_map<T1, T2> T2Vals;
	std::unordered_map<T2, T1> T1Vals;

};

template<class T2>
class BidirectionalMap<unsigned int, T2> {
public:
	void set(const unsigned int& a, const T2& b) {
		if (T2Vals.size() <= a)  T2Vals.resize(a);
		T2Vals[a] = b;
		T1Vals[b] = a;
	}

	const T2* get(const unsigned int& key) const {
		if (key >= T2Vals.size()) return nullptr;
		if (T2Vals[key]) return &(T2Vals[key].value());
		return nullptr;
	}
	const unsigned int* get(const T2& key) const {
		auto iter = T1Vals.find(key);
		if (iter == T1Vals.end()) return nullptr;
		return &(iter->second);
	}

private:
	std::vector<std::optional<T2>> T2Vals;
	std::unordered_map<T2, unsigned int> T1Vals;
};

#endif /* bidirectionalMap_h */
