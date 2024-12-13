#ifndef multiTypeMap_h
#define multiTypeMap_h

#include <any>
#include <memory>
#include <unordered_map>

/*
Stores all mappings
*/
template <class K>
class MultiTypeMap{
public:
    MultiTypeMap() : edits(false) {};

    // -- getters --
    template<class V>
    const V* get(const K& key) const;
    inline bool hasKey(const K& key) const { return mappings.find(key) != mappings.end(); }

    // -- setters --
    template<class V>
    void set(const K& key, const V& value);

private:
    std::unordered_map<K, std::any> mappings;
};

template<class K>
template<class V>
const V* MultiTypeMap<K>::get(const K& key) const {
    auto iter = mappings.find(key);
    if(iter == mappings.end()) return nullptr; // could remove this if haskey is always called prior
    return (iter->second);
}

template<class K>
template<class V>
void MultiTypeMap<K>::set(const K& key, const V& value){
    auto iter = mappings.find(key); // auto enjoyer
    if (iter != mappings.end()) {
        iter->second = std::make_unique<V>(value); // overriding the previous value
    } else {
        mappings[key] = std::make_unique<V>(value);
    }
}

#endif /* multiTypeMap_h */
