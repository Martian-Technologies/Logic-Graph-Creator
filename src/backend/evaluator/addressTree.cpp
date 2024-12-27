#include "addressTree.h"

template<class T>
void AddressTreeNode<T>::addValue(block_id_t blockId, T value) {
    if (hasValue(blockId) || hasBranch(blockId)) {
        throw std::invalid_argument("AddressTree::addValue: blockId already exists");
    }
    values[blockId] = value;
}

template<class T>
void AddressTreeNode<T>::addValue(const Address& address, T value) {
    assert(false); // untested
    if (address.size() == 0) {
        throw std::invalid_argument("AddressTree::addValue: address size is 0");
    }
    if (address.size() == 1) {
        addValue(address.getBlockId(0), value);
        return;
    }
    AddressTreeNode<T> currentBranch = *this;
    for (int i = 0; i < address.size() - 1; i++) {
        if (!currentBranch.hasBranch(address.getBlockId(i))) {
            currentBranch.makeBranch(address.getBlockId(i));
        }
        currentBranch = currentBranch.getBranch(address.getBlockId(i));
    }
    currentBranch.addValue(address.getBlockId(address.size() - 1), value);
}

template<class T>
void AddressTreeNode<T>::makeBranch(block_id_t blockId) {
    assert(false); // untested
    if (hasValue(blockId) || hasBranch(blockId)) {
        throw std::invalid_argument("AddressTree::makeBranch: blockId already exists");
    }
    branches[blockId] = AddressTreeNode<T>();
}

template<class T>
void AddressTreeNode<T>::makeBranch(const Address& address) {
    assert(false); // untested
    if (address.size() == 0) {
        throw std::invalid_argument("AddressTree::makeBranch: address size is 0");
    }
    if (address.size() == 1) {
        makeBranch(address.getBlockId(0));
        return;
    }
    AddressTreeNode<T> currentBranch = *this;
    for (int i = 0; i < address.size() - 1; i++) {
        if (!currentBranch.hasBranch(address.getBlockId(i))) {
            currentBranch.makeBranch(address.getBlockId(i));
        }
        currentBranch = currentBranch.getBranch(address.getBlockId(i));
    }
    currentBranch.makeBranch(address.getBlockId(address.size() - 1));
}


template<class T>
AddressTreeNode<T> AddressTreeNode<T>::getBranch(const Address& address) const {
    AddressTreeNode<T> currentBranch = *this;
    for (int i = 0; i < address.size(); i++) {
        const auto pt = currentBranch.branches.find(address.getBlockId(i));
        if (pt == currentBranch.branches.end()) {
            throw std::out_of_range("AddressTree::getBranch: address not found");
        }
        currentBranch = pt->second;
    }
    return currentBranch;
}

template<class T>
T AddressTreeNode<T>::getValue(const Address& address) const {
    AddressTreeNode<T> currentBranch = *this;
    for (int i = 0; i < address.size() - 1; i++) {
        const auto pt = currentBranch.branches.find(address.getBlockId(i));
        if (pt == currentBranch.branches.end()) {
            throw std::out_of_range("AddressTree::getLeaf: address not found");
        }
        currentBranch = pt->second;
    }
    return currentBranch.getValue(address.getBlockId(address.size() - 1));
}

template<class T>
void AddressTreeNode<T>::remap(const std::unordered_map<T, T>& mapping) {
    for (auto& [key, value] : leaves) {
        if (mapping.find(value) != mapping.end()) {
            value = mapping.at(value);
        }
    }
    for (auto& [key, value] : branches) {
        value.remap(mapping);
    }
}