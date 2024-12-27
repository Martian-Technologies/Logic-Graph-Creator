#ifndef addressTree_h
#define addressTree_h

#include <unordered_map>

#include "../address.h"

template <class T> class AddressTreeNode {
public:
    void addValue(block_id_t blockId, T value);
    void addValue(const Address& address, T value);
    void makeBranch(block_id_t blockId);
    void makeBranch(const Address& address);
    T getValue(block_id_t blockId) const { return values.at(blockId); }
    AddressTreeNode<T> getBranch(block_id_t blockId) const { return branches.at(blockId); }

    bool hasValue(block_id_t blockId) const { return values.find(blockId) != values.end(); }
    bool hasBranch(block_id_t blockId) const { return branches.find(blockId) != branches.end(); }

    AddressTreeNode<T> getBranch(const Address& address) const;
    T getValue(const Address& address) const;

    void remap(const std::unordered_map<T, T>& mapping);
private:
    std::unordered_map<block_id_t, T> values;
    std::unordered_map<block_id_t, AddressTreeNode<T>> branches;
};

#endif /* addressTree_h */