#ifndef addressTree_h
#define addressTree_h

#include <unordered_map>
#include <stdexcept>

#include "../address.h"

template <class T>
class AddressTreeNode {
public:
    void addValue(Position position, T value);
    void addValue(const Address& address, T value);
    void makeBranch(Position position);
    void makeBranch(const Address& address);

    void removeValue(Position position) { values.erase(position); }
    void removeValue(const Address& address) {
        const_cast<AddressTreeNode<T>&>(getParentBranch(address)).values.erase(address.getPosition(address.size() - 1));
    }

    inline T getValue(Position position) const { return values.at(position); }
    inline T getValue(const Address& address) const { return getParentBranch(address).getValue(address.getPosition(address.size() - 1)); }

    // Added const overload for getParentBranch
    AddressTreeNode<T>& getParentBranch(const Address& address);
    const AddressTreeNode<T>& getParentBranch(const Address& address) const;

    inline const AddressTreeNode<T>& getBranch(Position position) const;
    const AddressTreeNode<T>& getBranch(const Address& address) const;

    inline bool hasValue(Position position) const { return values.find(position) != values.end(); }
    inline bool hasBranch(Position position) const { return branches.find(position) != branches.end(); }

    void moveData(Position curPosition, Position newPosition);
    void remap(const std::unordered_map<T, T>& mapping);

private:
    std::unordered_map<Position, T> values;
    std::unordered_map<Position, AddressTreeNode<T>> branches;
};

template<class T>
void AddressTreeNode<T>::addValue(Position position, T value) {
    if (hasValue(position) || hasBranch(position)) {
        throw std::invalid_argument("AddressTree::addValue: position already exists");
    }
    values[position] = value;
}

template<class T>
void AddressTreeNode<T>::addValue(const Address& address, T value) {
    if (address.size() == 0) {
        throw std::invalid_argument("AddressTree::addValue: address size is 0");
    }
    AddressTreeNode<T>& parentBranch = getParentBranch(address);
    const Position finalPosition = address.getPosition(address.size() - 1);
    parentBranch.addValue(finalPosition, value);
}

template<class T>
void AddressTreeNode<T>::makeBranch(Position position) {
    if (hasValue(position) || hasBranch(position)) {
        throw std::invalid_argument("AddressTree::makeBranch: position already exists");
    }
    branches[position] = AddressTreeNode<T>();
}

template<class T>
void AddressTreeNode<T>::makeBranch(const Address& address) {
    if (address.size() == 0) {
        throw std::invalid_argument("AddressTree::makeBranch: address size is 0");
    }
    getParentBranch(address).makeBranch(address.getPosition(address.size() - 1));
}

template<class T>
const AddressTreeNode<T>& AddressTreeNode<T>::getBranch(Position position) const {
    auto it = branches.find(position);
    if (it == branches.end()) {
        throw std::out_of_range("AddressTree::getBranch: address not found");
    }
    return it->second;
}

template<class T>
const AddressTreeNode<T>& AddressTreeNode<T>::getBranch(const Address& address) const {
    const AddressTreeNode<T>* currentBranch = this;
    for (size_t i = 0; i < address.size(); i++) {
        currentBranch = &getBranch(address.getPosition(i));
    }
    return *currentBranch;
}

template<class T>
void AddressTreeNode<T>::moveData(Position curPosition, Position newPosition) {
    if (hasValue(curPosition)) {
        auto pair = values.extract(curPosition);
        pair.key() = newPosition;
        values.insert(std::move(pair));
    } else {
        auto pair = branches.extract(curPosition);
        pair.key() = newPosition;
        branches.insert(std::move(pair));
    }
}

template<class T>
void AddressTreeNode<T>::remap(const std::unordered_map<T, T>& mapping) {
    for (auto& [position, value] : values) {
        auto it = mapping.find(value);
        if (it != mapping.end()) {
            value = it->second;
        }
    }
    for (auto& [position, branch] : branches) {
        branch.remap(mapping);
    }
}

template<class T>
AddressTreeNode<T>& AddressTreeNode<T>::getParentBranch(const Address& address) {
    AddressTreeNode<T>* currentBranch = this;
    for (size_t i = 0; i < address.size() - 1; i++) {
        auto it = currentBranch->branches.find(address.getPosition(i));
        if (it == currentBranch->branches.end()) {
            throw std::out_of_range("AddressTree::getParentBranch: address not found");
        }
        currentBranch = &(it->second);
    }
    return *currentBranch;
}

template<class T>
const AddressTreeNode<T>& AddressTreeNode<T>::getParentBranch(const Address& address) const {
    const AddressTreeNode<T>* currentBranch = this;
    for (size_t i = 0; i < address.size() - 1; i++) {
        auto it = currentBranch->branches.find(address.getPosition(i));
        if (it == currentBranch->branches.end()) {
            throw std::out_of_range("AddressTree::getParentBranch: address not found");
        }
        currentBranch = &(it->second);
    }
    return *currentBranch;
}

#endif /* addressTree_h */