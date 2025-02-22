#ifndef multiTypeMap_h
#define multiTypeMap_h

#include <any>
#include <optional>
#include <unordered_map>

class MultiTypeMap { // stores all preferences
public:
    MultiTypeMap(std::unordered_map<std::string, std::any> mapped) : mappings(std::move(mapped)), edited(false) { }

    // -- getters --
	template<typename T>
	std::optional<T> get(const std::string& key) const { 
		auto itr = mappings.find(key);
		if (itr == mappings.end()){
			return std::nullopt;
		} else if (itr->second.type() == typeid(T)) { // additional safety to ensure grabbing value of correct type
			return std::any_cast<T>(itr->second);
		}
		return std::nullopt; 
	}
    inline bool hasKey(const std::string& key) const { return mappings.find(key) != mappings.end(); }

    // -- setters --
    bool set(const std::string& key, const std::any& value) {
		if (mappings.find(key) != mappings.end()) {
			mappings[key] = value;
			edited = true;
			return 1;
		}
		return 0;
	}

private:
    bool edited; // for whether or not to update config file
    std::unordered_map<std::string, std::any> mappings;
};

#endif /* multiTypeMap_h */
