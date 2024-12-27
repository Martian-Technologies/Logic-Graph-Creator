#ifndef address_h
#define address_h

#include <vector>

#include "backend/defs.h"

class Address {
public:
    inline int size() const { return addresses.size(); }
    inline block_id_t getBlockId(int index) const { return addresses[index]; }

    inline void addBlockId(block_id_t blockId) { addresses.push_back(blockId); }

private:
    std::vector<block_id_t> addresses;

};

#endif /* address_h */