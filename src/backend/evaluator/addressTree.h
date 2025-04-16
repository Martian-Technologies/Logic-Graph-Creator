#ifndef addressTree_h
#define addressTree_h

#include "backend/address.h"

template <class T>
class AddressTreeNode {
public:
	AddressTreeNode(circuit_id_t contId, Rotation rotation) : containerId(contId), rotation(rotation) {}

	void addValue(Position position, T value);
	void addValue(const Address& address, T value);
	std::vector<Address> addValue(Position position, circuit_id_t targetParentContainerId, T Value);
	void makeBranch(Position position, circuit_id_t newContainerId, Rotation rotation);
	void makeBranch(const Address& address, circuit_id_t newContainerId, Rotation rotation);
	std::vector<Address> makeBranch(Position position, circuit_id_t targetParentContainerId, circuit_id_t newContainerId, Rotation rotation);

	void removeValue(Position position) { values.erase(position); }
	void removeValue(const Address& address) {
		const_cast<AddressTreeNode<T>&>(getParentBranch(address)).values.erase(address.getPosition(address.size() - 1));
	}
	void removeValue(Position position, circuit_id_t targetParentContainerId) {
		if (containerId == targetParentContainerId) {
			removeValue(position);
		}
		else {
			for (auto& [pos, branch] : branches) {
				branch.removeValue(position, targetParentContainerId);
			}
		}
	}

	void nukeBranch(Position position) { branches.erase(position); }
	void nukeBranch(const Address& address) {
		const_cast<AddressTreeNode<T>&>(getParentBranch(address)).branches.erase(address.getPosition(address.size() - 1));
	}

	inline T getValue(Position position) const { return values.at(position); }
	inline T getValue(Position position, T defaultValue) const {
		auto it = values.find(position);
		if (it == values.end()) {
			return defaultValue;
		}
		return it->second;
	}
	inline T getValue(const Address& address) const { return getParentBranch(address).getValue(address.getPosition(address.size() - 1)); }
	inline T getValue(const Address& address, T defaultValue) const { return getParentBranch(address).getValue(address.getPosition(address.size() - 1), defaultValue); }
	inline std::vector<T> getAllValues() const;

	void setValue(Position position, T value);
	void setValue(const Address& address, T value);

	std::vector<Address> getPositions(circuit_id_t targetParentContainerId, Position position);

	AddressTreeNode<T>& getParentBranch(const Address& address);
	const AddressTreeNode<T>& getParentBranch(const Address& address) const;

	inline AddressTreeNode<T>& getBranch(Position position);
	AddressTreeNode<T>& getBranch(const Address& address);

	inline bool hasValue(const Position position) const { return values.find(position) != values.end(); }
	inline bool hasValue(const Address& address) const { return getParentBranch(address).hasValue(address.getPosition(address.size() - 1)); }
	inline bool hasBranch(const Position position) const { return branches.find(position) != branches.end(); }
	inline bool hasBranch(const Address& address) const { return getParentBranch(address).hasBranch(address.getPosition(address.size() - 1)); }

	void moveData(Position curPosition, Position newPosition);
	void moveData(circuit_id_t, Position curPosition, Position newPosition);
	void remap(const std::unordered_map<T, T>& mapping);

	Rotation getRotation() const { return rotation; }

	circuit_id_t getContainerId() const { return containerId; }

private:
	std::unordered_map<Position, T> values;
	std::unordered_map<Position, AddressTreeNode<T>> branches;
	circuit_id_t containerId;
	Rotation rotation;
};

template<class T>
void AddressTreeNode<T>::addValue(Position position, T value) {
	if (hasValue(position) || hasBranch(position)) {
		logError("AddressTree::addValue: position already exists");
		return;
	}
	values[position] = value;
}

template<class T>
void AddressTreeNode<T>::addValue(const Address& address, T value) {
	if (address.size() == 0) {
		logError("AddressTree::addValue: address size is 0");
		return;
	}
	AddressTreeNode<T>& parentBranch = getParentBranch(address);
	const Position finalPosition = address.getPosition(address.size() - 1);
	parentBranch.addValue(finalPosition, value);
}

template<class T>
std::vector<Address> AddressTreeNode<T>::addValue(Position position, circuit_id_t targetParentContainerId, T value) {
	std::vector<Address> addresses;
	if (containerId == targetParentContainerId) {
		addValue(position, value);
		addresses.push_back(Address(position));
	}
	else {
		for (auto& [pos, branch] : branches) {
			std::vector<Address> newAddresses = branch.addValue(position, targetParentContainerId, value);
			for (Address& address : newAddresses) {
				address.nestPosition(pos);
			}
			addresses.insert(addresses.end(), newAddresses.begin(), newAddresses.end());
		}
	}
	return addresses;
}

template<class T>
void AddressTreeNode<T>::makeBranch(Position position, circuit_id_t newContainerId, Rotation rotation) {
	if (hasValue(position) || hasBranch(position)) {
		logError("AddressTree::makeBranch: position already exists");
		return;
	}
	branches.emplace(position, AddressTreeNode<T>(newContainerId, rotation));
}

template<class T>
void AddressTreeNode<T>::makeBranch(const Address& address, circuit_id_t newContainerId, Rotation rotation) {
	if (address.size() == 0) {
		logError("AddressTree::makeBranch: address size is 0");
		return;
	}
	getParentBranch(address).makeBranch(address.getPosition(address.size() - 1), newContainerId, rotation);
}

template<class T>
std::vector<Address> AddressTreeNode<T>::makeBranch(Position position, circuit_id_t targetParentContainerId, circuit_id_t newContainerId, Rotation rotation) {
	std::vector<Address> addresses;
	if (containerId == targetParentContainerId) {
		makeBranch(position, newContainerId, rotation);
		addresses.push_back(Address(position));
	}
	else {
		for (auto& [pos, branch] : branches) {
			std::vector<Address> newAddresses = branch.makeBranch(position, targetParentContainerId, newContainerId, rotation);
			for (Address& address : newAddresses) {
				address.nestPosition(pos);
			}
			addresses.insert(addresses.end(), newAddresses.begin(), newAddresses.end());
		}
	}
	return addresses;
}

template<class T>
AddressTreeNode<T>& AddressTreeNode<T>::getBranch(Position position) {
	auto it = branches.find(position);
	if (it == branches.end()) {
		logError("AddressTree::getBranch: address not found");
	}
	return it->second;
}

template<class T>
AddressTreeNode<T>& AddressTreeNode<T>::getBranch(const Address& address) {
	AddressTreeNode<T>* currentBranch = this;
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
void AddressTreeNode<T>::moveData(circuit_id_t targetParentContainerId, Position curPosition, Position newPosition) {
	if (containerId == targetParentContainerId) {
		moveData(curPosition, newPosition);
	} else {
		for (auto& [pos, branch] : branches) {
			branch.moveData(targetParentContainerId, curPosition, newPosition);
		}
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
inline std::vector<T> AddressTreeNode<T>::getAllValues() const {
	std::vector<T> allValues;
	for (auto& [position, value] : values) {
		allValues.push_back(value);
	}
	for (auto& [position, branch] : branches) {
		std::vector<T> newValues = branch.getAllValues();
		allValues.insert(allValues.end(), newValues.begin(), newValues.end());
	}
	return allValues;
}

template<class T>
void AddressTreeNode<T>::setValue(Position position, T value) {
	values[position] = value;
}

template<class T>
void AddressTreeNode<T>::setValue(const Address& address, T value) {
	getParentBranch(address).setValue(address.getPosition(address.size() - 1), value);
}

template<class T>
std::vector<Address> AddressTreeNode<T>::getPositions(circuit_id_t targetParentContainerId, Position position) {
	std::vector<Address> addresses;
	if (containerId == targetParentContainerId) {
		addresses.push_back(Address(position));
	}
	for (auto& [pos, branch] : branches) {
		std::vector<Address> newAddresses = branch.getPositions(targetParentContainerId, position);
		for (Address& address : newAddresses) {
			address.nestPosition(pos);
		}
		addresses.insert(addresses.end(), newAddresses.begin(), newAddresses.end());
	}
	return addresses;
}

template<class T>
AddressTreeNode<T>& AddressTreeNode<T>::getParentBranch(const Address& address) {
	AddressTreeNode<T>* currentBranch = this;
	for (size_t i = 0; i < address.size() - 1; i++) {
		auto it = currentBranch->branches.find(address.getPosition(i));
		if (it == currentBranch->branches.end()) {
			logError("AddressTree::getParentBranch: address not found");
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
			logError("AddressTree::getParentBranch: address not found");
		}
		currentBranch = &(it->second);
	}
	return *currentBranch;
}

#endif /* addressTree_h */
