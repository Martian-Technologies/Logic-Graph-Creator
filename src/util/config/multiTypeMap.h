#ifndef multiTypeMap_h
#define multiTypeMap_h

#include <any>
#include <unordered_map>

/*
Stores all mappings
*/
class MultiTypeMap{
public:
    MultiTypeMap();

    // -- getters --
    template<class V>
    const V* get(const std::string& key) const;
    inline bool hasKey(const std::string& key) const { return mappings.find(key) != mappings.end(); }

    // -- setters --
    template<class V>
    void set(const std::string& key, const V& value);

private:
    bool edited;
    std::unordered_map<std::string, std::any> mappings;
};

#endif /* multiTypeMap_h */
