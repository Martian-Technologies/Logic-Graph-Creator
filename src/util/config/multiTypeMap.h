#ifndef multiTypeMap_h
#define multiTypeMap_h

#include <any>
#include <unordered_map>
#include <string>
#include <optional>

class MultiTypeMap {
public:

    MultiTypeMap() {}

    // -- Getters --
	template<typename T>
    std::optional<T> get(const std::string& key) const {
		std::unordered_map<std::string, std::any>::const_iterator itr = mappings.find(key);
		if (itr == mappings.cend()) return std::nullopt;

		try {
			return std::any_cast<T>(itr->second);
		} catch (const std::bad_any_cast&) {
			return std::nullopt;
		}
	}
    bool hasKey(const std::string& key) const { return mappings.find(key) != mappings.end(); }

    // -- Setters --
	template<typename T>
    bool set(const std::string& key, const T& value) {
		std::unordered_map<std::string, std::any>::iterator itr = mappings.find(key);
		if (itr != mappings.end() && itr->second.type() != typeid(T)) return false;
        mappings[key] = value;
		return true;
    }

private:
    std::unordered_map<std::string, std::any> mappings;
};


#endif /* multiTypeMap_h */
