#ifndef idProvider_h
#define idProvider_h

template<typename T>
class IdProvider {
public:
	IdProvider() : lastId(0) {}

	T getNewId() {
		if (unusedIds.empty()) {
			return lastId++;
		} else {
			T id = *unusedIds.begin();
			unusedIds.erase(unusedIds.begin());
			return id;
		}
	}
	void releaseId(T id) {
		if (id > lastId || unusedIds.contains(id)) {
			throw std::invalid_argument("Invalid ID release attempt");
		}
		unusedIds.insert(id);
	}
	bool isIdUsed(T id) const {
		return id <= lastId && !unusedIds.contains(id);
	}
	T getLastId() const {
		return lastId;
	}
	void reset() {
		lastId = 0;
		unusedIds.clear();
	}
	std::vector<T> getUsedIds() const {
		std::vector<T> usedIds;
		for (T id = 0; id < lastId; ++id) {
			if (!unusedIds.contains(id)) {
				usedIds.push_back(id);
			}
		}
		return usedIds;
	}
private:
	T lastId;
	std::set<T> unusedIds;
};

#endif // idProvider_h