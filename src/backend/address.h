#ifndef address_h
#define address_h

#include <vector>
#include "backend/defs.h"

class Address {
private:
    std::vector<block_id_t> address;

public:
    Address() = default;
    void addBlockId(block_id_t blockId) { address.push_back(blockId); }
    block_id_t getBlockId(int index) const { return address[index]; }
    int size() const { return address.size(); }
};

#endif /* address_h */