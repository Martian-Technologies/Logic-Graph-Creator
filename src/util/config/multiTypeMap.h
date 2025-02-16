#ifndef multiTypeMap_h
#define multiTypeMap_h

#include <any>
#include <tuple>
#include <unordered_map>

class AnyType;

/*
Stores all mappings
*/
class MultiTypeMap {
public:
    MultiTypeMap(){}

    // -- getters --
    const AnyType get(const std::string& key) const { return mappings.find(key)->second; }
    inline bool hasKey(const std::string& key) const { return mappings.find(key) != mappings.end(); }

    // -- setters --
    template<class V>
    void set(const std::string& key, const V& value);

private:
    bool edited;
    std::unordered_map<std::string, AnyType> mappings;
};

uint8_t htoi(const char hexChar){
	if (hexChar >= '0' && hexChar <= '9') {
        return hexChar - '0';  // Convert '0'-'9' to 0-9
    } else if (hexChar >= 'a' && hexChar <= 'f') {
        return hexChar - 'a' + 10;  // Convert 'a'-'f' to 10-15
    } else if (hexChar >= 'A' && hexChar <= 'F') {
        return hexChar - 'A' + 10;  // Convert 'A'-'F' to 10-15
    }
	return 0;
}

class AnyType {
public:
	virtual ~AnyType() = default;
	
	virtual void setValue(const std::string& value);
	virtual int getNumericValue() const{ throw std::runtime_error("Not a numerical type"); }
	virtual const std::string& getStringValue() const{ throw std::runtime_error("Not a numerical type"); }
	virtual bool getBoolValue() const{ throw std::runtime_error("Not a numerical type"); }
	virtual std::tuple<uint8_t, uint8_t, uint8_t> getRGBValue() const{ throw std::runtime_error("Not a numerical type"); }
};

class NumericType : AnyType {
public:
	NumericType(const std::string& value) { this->value = static_cast<int>(std::stoi(value)); }
	void setValue(const std::string& value) override {
		this->value = static_cast<int>(std::stoi(value));
	}
	int getNumericValue() const { return value; }
private:
	int value;
};

class StringType : AnyType {
public:
	StringType(const std::string& value) : value(value) {}
	void setValue(const std::string& value) override {
		this->value = value;
	}
	const std::string& getStringValue() const { return value; }
private:
	std::string value;
};

class BoolType : AnyType {
public:
	BoolType(const std::string& value) { this->value = (value[0] == '1') ? true : false; }
	void setValue(const std::string& value) override {
		this->value = (value[0] == '1') ? true : false;
	}
	bool getBoolValue() const { return value; }
private:
	bool value;
};

class RGBType : AnyType {
public:
	RGBType(const std::string& value) {
		r = htoi(value[5])*16 + htoi(value[4]); 
		g = htoi(value[3])*16 + htoi(value[2]); 
		b = htoi(value[1])*16 + htoi(value[0]); 
	}
	void setValue(const std::string& value) override {	
		r = htoi(value[5])*16 + htoi(value[4]); 
		g = htoi(value[3])*16 + htoi(value[2]); 
		b = htoi(value[1])*16 + htoi(value[0]);
	}
	std::tuple<uint8_t, uint8_t, uint8_t> getRGBValue() const { return std::make_tuple(r,g,b); }
private:
	uint8_t r, g, b;
};

#endif /* multiTypeMap_h */
