#ifndef idProvider_h
#define idProvider_h

template<typename T>
class IdProvider {
public:
	IdProvider() : lastId(0) {}

	inline T getNewId() {
		if (unusedIds.empty()) {
			return lastId++;
		} else {
			T id = *unusedIds.begin();
			unusedIds.erase(unusedIds.begin());
			return id;
		}
	}
	inline T getNewId(T preferredId) {
		if (unusedIds.size() * 2 < lastId && unusedIds.contains(preferredId)) {
			unusedIds.erase(preferredId);
			return preferredId;
		}
		if (preferredId == lastId || unusedIds.empty()) {
			return lastId++;
		} else {
			T id = *unusedIds.begin();
			unusedIds.erase(unusedIds.begin());
			return id;
		}
	}
	inline void releaseId(T id) {
		if (id > lastId || unusedIds.contains(id)) {
			return;
		}
		unusedIds.insert(id);
	}
	inline bool isIdUsed(T id) const {
		return id <= lastId && !unusedIds.contains(id);
	}
	inline T getLastId() const {
		return lastId;
	}
	inline void reset() {
		lastId = 0;
		unusedIds.clear();
	}
	inline std::vector<T> getUsedIds() const {
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

#endif /* idProvider_h */
