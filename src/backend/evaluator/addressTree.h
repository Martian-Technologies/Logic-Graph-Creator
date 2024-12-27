#ifndef addressTree_h
#define addressTree_h

#include <unordered_map>
#include "address.h"

template <class T> class AddressTree {
private:
    std::unordered_map<block_id_t, T> leaves;
    std::unordered_map<block_id_t, AddressTree<T>> branches;
public:
    AddressTree() = default;
    void makeLeaf(block_id_t blockId, T leaf);
    void makeLeaf(const Address& address, T leaf);
    void makeBranch(block_id_t blockId, AddressTree<T> branch);
    void makeBranch(const Address& address, AddressTree<T> branch);
    T getLeaf(block_id_t blockId) const { return leaves.at(blockId); }
    AddressTree<T> getBranch(block_id_t blockId) const { return branches.at(blockId); }

    bool hasLeaf(block_id_t blockId) const { return leaves.find(blockId) != leaves.end(); }
    bool hasBranch(block_id_t blockId) const { return branches.find(blockId) != branches.end(); }

    AddressTree<T> getBranch(const Address& address) const;
    T getLeaf(const Address& address) const;

    void remap(const std::unordered_map<T, T>& mapping);
};

#endif /* addressTree_h */