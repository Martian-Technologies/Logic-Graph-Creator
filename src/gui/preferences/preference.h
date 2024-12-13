#include <string>

#include "../../util/multiTypeMap.h"

class Preferences{
public:
    template<class V>
    inline void set(const std::string& key, const V& value) { map.set<V>(key, value); }

    template<class V>
    inline const V* get(const std::string& key) const { return map.get<V>(key); }
    
    inline bool hasKey(const std::string& key) const { map.hasKey(key); }

private:
    MultiTypeMap<std::string> map;
};
