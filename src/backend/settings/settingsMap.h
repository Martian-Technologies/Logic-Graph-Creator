#ifndef settingsMap_h
#define settingsMap_h

class SettingsMap {
public:
	enum SettingType {
		VOID,
		STRING,
		INT,
		KEYBIND
	};
	template<SettingType settingType> struct SettingTypeToType;
	template<> struct SettingTypeToType<SettingType::STRING> { using type = std::string; };
	template<> struct SettingTypeToType<SettingType::INT> { using type = int; };
	template<> struct SettingTypeToType<SettingType::KEYBIND> { using type = std::string; };

	SettingsMap() { }

	template<SettingType settingType>
	void registerName(std::string name) {
		mappings[name] = std::make_unique<settingType>();
	}
	template<SettingType settingType>
	void registerName(std::string name, const SettingTypeToType<settingType>::type& value) {
		mappings[name] = std::make_unique<settingType>(value);
	}

	// -- Getters --
	template<SettingType settingType>
	const SettingTypeToType<settingType>::type* get(const std::string& key) const {
		std::unordered_map<std::string, std::unique_ptr<SettingEntryBase>>::const_iterator iter = mappings.find(key);
		if (iter == mappings.end() || settingType != iter->second->getType()) return nullptr;
		const SettingEntry<settingType>* settingEntry = dynamic_cast<std::unique_ptr<SettingEntry<settingType>>>(iter->second).get();
		if (!settingEntry) {
			logError("Failed to get value. Type and SettingType mismatched internal state bad. Please report error and relaunch the app.", "SettingsMap");
			return nullptr;
		}
		return &(settingEntry->getValue());
	}
	const SettingType getType(const std::string& key) const {
		std::unordered_map<std::string, std::unique_ptr<SettingEntryBase>>::const_iterator iter = mappings.find(key);
		if (iter == mappings.end()) return SettingType::VOID;
		return iter->second->getType();
	}
	bool hasKey(const std::string& key) const { return mappings.find(key) != mappings.end(); }

	// -- Setters --
	template<SettingType settingType>
	bool set(const std::string& key, const SettingTypeToType<settingType>::type& value) {
		std::unordered_map<std::string, std::unique_ptr<SettingEntryBase>>::const_iterator iter = mappings.find(key);
		if (iter == mappings.end()) {
			logError("Failed to set value. Could not find key \"" + key + "\"", "SettingsMap");
			return false;
		}
		if (settingType != iter->second->getType()) {
			logError("Failed to set value. Could not find key \"" + key + "\"", "SettingsMap");
			return false;
		}
		SettingEntry<settingType>* settingEntry = dynamic_cast<std::unique_ptr<SettingEntry<settingType>>>(iter->second).get();
		if (!settingEntry) {
			logError("Failed to set value. Type and SettingType mismatched internal state bad. Please report error and relaunch the app.", "SettingsMap");
			return false;
		}
		settingEntry->setValue(value);
		return true;
	}

private:
	class SettingEntryBase {
	public:
	SettingEntryBase(SettingType type) : type(type) { }
		SettingType getType() const { return type; }

	private:
		SettingType type;
	};

	template <SettingType T>
	class SettingEntry : public SettingEntryBase {
	public:
		SettingEntry() : SettingEntryBase(T) { }
		SettingEntry(const SettingTypeToType<T>::type& value) : SettingEntryBase(T), value(value) { }
		
		const SettingTypeToType<T>::type& getValue() const { return value; }
		void setValue(const SettingTypeToType<T>::type& value) { this->value = value; }

	private:
		SettingTypeToType<T>::type value;
	};

	std::unordered_map<std::string, std::unique_ptr<SettingEntryBase>> mappings;
};

#endif /* settingsMap_h */
