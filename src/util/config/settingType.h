#ifndef settingMap_h
#define settingMap_h

#include <any>
#include <unordered_map>
#include <string>
#include <optional>

class SettingMap {
public:
	enum SettingType {
		STRING,
		INT,
		KEYBIND
	}

    SettingMap() {}

	template<typename T>
	void registerName(std::string name, SettingType type) {
		mappings[name].first = type;
	}

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
	class SettingEntry {
	public:
		SettingEntry(SettingType type) {

		}

	private:
		SettingType type;
	}

    std::unordered_map<std::string, std::pair<SettingType, std::any>> mappings;
};

#endif /* settingMap_h */
