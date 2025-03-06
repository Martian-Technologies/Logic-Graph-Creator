#ifndef multiTypeMap_h
#define multiTypeMap_h

#include <unordered_map>
#include <variant>
#include <string>
#include <stdexcept>

#include "gui/circuitView/renderer/color.h"

typedef std::variant<std::string, bool, int, float, Color> VariantType; 

class MultiTypeMap {
public:

    MultiTypeMap() : edited(false) {}

    // -- Getters --
    VariantType get(const std::string& key) const {
		auto itr = mappings.find(key);
		if (itr == mappings.end()) return VariantType{}; 

		return itr->second;
	}

    bool hasKey(const std::string& key) const { return mappings.find(key) != mappings.end(); }
    bool isEdited() const { return edited; }

    // -- Setters --
    void set(const std::string& key, const VariantType& value) {
        edited = true;
        mappings[key] = value;
    }

private:
    bool edited;
    std::unordered_map<std::string, VariantType> mappings;

	std::string firstValue;


};


#endif /* multiTypeMap_h */
