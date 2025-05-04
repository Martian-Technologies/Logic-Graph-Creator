#ifndef addressTree_h
#define addressTree_h

#include "backend/address.h"
#include "gate.h"

#include "backend/dataUpdateEventManager.h"

class AddressTreeNode {
public:
	AddressTreeNode(circuit_id_t contId, Rotation rotation, DataUpdateEventManager* dataUpdateEventManager, bool isRoot) : containerId(contId), rotation(rotation), dataUpdateEventManager(dataUpdateEventManager), isRoot(isRoot) { }

	inline void addValue(Position position, EvaluatorGate value);
	inline void addValue(const Address& address, EvaluatorGate value);
	inline std::vector<Address> addValue(Position position, circuit_id_t targetParentContainerId, EvaluatorGate Value);
	inline void makeBranch(Position position, circuit_id_t newContainerId, Rotation rotation);
	inline void makeBranch(const Address& address, circuit_id_t newContainerId, Rotation rotation);
	inline std::vector<Address> makeBranch(Position position, circuit_id_t targetParentContainerId, circuit_id_t newContainerId, Rotation rotation);

	inline void removeValue(Position position) { values.erase(position); }
	inline void removeValue(const Address& address) {
		getParentBranch(address)->values.erase(address.getPosition(address.size() - 1));
	}
	inline void removeValue(Position position, circuit_id_t targetParentContainerId) {
		if (containerId == targetParentContainerId) {
			removeValue(position);
		}
		else {
			for (auto& [pos, branch] : branches) {
				branch.removeValue(position, targetParentContainerId);
			}
		}
	}

	inline void nukeBranch(Position position) { branches.erase(position); }
	inline void nukeBranch(const Address& address) {
		getParentBranch(address)->branches.erase(address.getPosition(address.size() - 1));
		dataUpdateEventManager->sendEvent("addressTreeMakeBranch");
	}

	inline EvaluatorGate getValue(Position position) const { return values.at(position); }
	inline EvaluatorGate getValue(Position position, EvaluatorGate defaultValue) const {
		auto it = values.find(position);
		if (it == values.end()) {
			return defaultValue;
		}
		return it->second;
	}
	inline EvaluatorGate getValue(const Address& address) const { return getParentBranch(address)->getValue(address.getPosition(address.size() - 1)); }
	inline EvaluatorGate getValue(const Address& address, EvaluatorGate defaultValue) const { return getParentBranch(address)->getValue(address.getPosition(address.size() - 1), defaultValue); }
	inline std::vector<EvaluatorGate> getAllValues() const;
	inline std::vector<EvaluatorIOJunction> getAllIOs() const;

	inline void setValue(Position position, EvaluatorGate value);
	inline void setValue(const Address& address, EvaluatorGate value);

	inline std::vector<Address> getPositions(circuit_id_t targetParentContainerId, Position position) const;

	inline AddressTreeNode* getParentBranch(const Address& address);
	inline const AddressTreeNode* getParentBranch(const Address& address) const;

	inline AddressTreeNode* getBranch(Position position);
	AddressTreeNode* getBranch(const Address& address);
	inline const std::unordered_map<Position, AddressTreeNode>& getBranchs() const { return branches; }
	inline std::vector<AddressTreeNode*> getBranches(circuit_id_t targetParentContainerId) const;

	inline bool hasValue(const Position position) const { return values.find(position) != values.end(); }
	inline bool hasValue(const Address& address) const { return getParentBranch(address)->hasValue(address.getPosition(address.size() - 1)); }
	inline bool hasBranch(const Position position) const { return branches.find(position) != branches.end(); }
	inline bool hasBranch(const Address& address) const { return getParentBranch(address)->hasBranch(address.getPosition(address.size() - 1)); }

	inline void moveData(Position curPosition, Position newPosition);
	inline void moveData(circuit_id_t targetParentContainerId, Position curPosition, Position newPosition);
	inline void remap(const std::unordered_map<EvaluatorGate, EvaluatorGate>& mapping);

	inline Rotation getRotation() const { return rotation; }

	inline circuit_id_t getContainerId() const { return containerId; }

	inline void removeConnectionIO(connection_end_id_t connectionId) {
		connectionIOs.erase(connectionId);
	}
	inline void addConnectionIO(connection_end_id_t connectionId, EvaluatorIOJunction value) {
		connectionIOs[connectionId] = value;
	}
	inline EvaluatorIOJunction getConnectionIO(connection_end_id_t connectionId) const {
		auto it = connectionIOs.find(connectionId);
		if (it == connectionIOs.end()) {
			logError("AddressTree::getConnectionIO: connectionId not found");
			return EvaluatorIOJunction();
		}
		return it->second;
	}
	inline bool hasConnectionIO(connection_end_id_t connectionId) const {
		return connectionIOs.find(connectionId) != connectionIOs.end();
	}
	inline std::unordered_map<connection_end_id_t, EvaluatorIOJunction> getAllConnectionIOs() const {
		return connectionIOs;
	}
	inline std::vector<connection_end_id_t> getAllConnectionIOIds() const {
		std::vector<connection_end_id_t> connectionIds;
		for (auto& pair : connectionIOs) {
			connectionIds.push_back(pair.first);
		}
		return connectionIds;
	}
	inline bool isNodeRoot() const { return isRoot; }

private:
	std::unordered_map<Position, EvaluatorGate> values;
	std::unordered_map<Position, AddressTreeNode> branches;
	std::unordered_map<connection_end_id_t, EvaluatorIOJunction> connectionIOs;
	DataUpdateEventManager* dataUpdateEventManager;
	circuit_id_t containerId;
	Rotation rotation;
	bool isRoot;
};

inline void AddressTreeNode::addValue(Position position, EvaluatorGate value) {
	if (hasValue(position) || hasBranch(position)) {
		logError("AddressTree::addValue: position already exists");
		return;
	}
	values[position] = value;
}

inline void AddressTreeNode::addValue(const Address& address, EvaluatorGate value) {
	if (address.size() == 0) {
		logError("AddressTree::addValue: address size is 0");
		return;
	}
	AddressTreeNode* parentBranch = getParentBranch(address);
	const Position finalPosition = address.getPosition(address.size() - 1);
	parentBranch->addValue(finalPosition, value);
}

inline std::vector<Address> AddressTreeNode::addValue(Position position, circuit_id_t targetParentContainerId, EvaluatorGate value) {
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

inline void AddressTreeNode::makeBranch(Position position, circuit_id_t newContainerId, Rotation rotation) {
	if (hasValue(position) || hasBranch(position)) {
		logError("AddressTree::makeBranch: position already exists");
		return;
	}
	branches.emplace(position, AddressTreeNode(newContainerId, rotation, dataUpdateEventManager, false));
	dataUpdateEventManager->sendEvent("addressTreeMakeBranch");
}

inline void AddressTreeNode::makeBranch(const Address& address, circuit_id_t newContainerId, Rotation rotation) {
	if (address.size() == 0) {
		logError("AddressTree::makeBranch: address size is 0");
		return;
	}
	getParentBranch(address)->makeBranch(address.getPosition(address.size() - 1), newContainerId, rotation);
}

inline std::vector<Address> AddressTreeNode::makeBranch(Position position, circuit_id_t targetParentContainerId, circuit_id_t newContainerId, Rotation rotation) {
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

inline AddressTreeNode* AddressTreeNode::getBranch(Position position) {
	auto it = branches.find(position);
	if (it == branches.end()) {
		logError("AddressTree::getBranch: position not found");
		return nullptr;
	}
	return &(it->second);
}

inline AddressTreeNode* AddressTreeNode::getBranch(const Address& address) {
	AddressTreeNode* currentBranch = this;
	for (size_t i = 0; i < address.size(); i++) {
		currentBranch = currentBranch->getBranch(address.getPosition(i));
		if (!currentBranch) {
			logError("AddressTree::getBranch: address not found");
			return nullptr;
		}
	}
	return currentBranch;
}

inline std::vector<AddressTreeNode*> AddressTreeNode::getBranches(circuit_id_t targetParentContainerId) const {
	// BFS
	std::vector<AddressTreeNode*> branchesToReturn;
	std::queue<AddressTreeNode*> queue;
	queue.push(const_cast<AddressTreeNode*>(this));
	while (!queue.empty()) {
		AddressTreeNode* currentBranch = queue.front();
		queue.pop();
		if (currentBranch->containerId == targetParentContainerId) {
			branchesToReturn.push_back(currentBranch);
		}
		for (auto& [pos, branch] : currentBranch->branches) {
			queue.push(&branch);
		}
	}
	return branchesToReturn;
}

inline void AddressTreeNode::moveData(Position curPosition, Position newPosition) {
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

inline void AddressTreeNode::moveData(circuit_id_t targetParentContainerId, Position curPosition, Position newPosition) {
	if (containerId == targetParentContainerId) {
		moveData(curPosition, newPosition);
	} else {
		for (auto& [pos, branch] : branches) {
			branch.moveData(targetParentContainerId, curPosition, newPosition);
		}
	}
}

inline void AddressTreeNode::remap(const std::unordered_map<EvaluatorGate, EvaluatorGate>& mapping) {
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

inline std::vector<EvaluatorGate> AddressTreeNode::getAllValues() const {
	std::vector<EvaluatorGate> allValues;
	for (auto& [position, value] : values) {
		allValues.push_back(value);
	}
	for (auto& [position, branch] : branches) {
		std::vector<EvaluatorGate> newValues = branch.getAllValues();
		allValues.insert(allValues.end(), newValues.begin(), newValues.end());
	}
	return allValues;
}

inline std::vector<EvaluatorIOJunction> AddressTreeNode::getAllIOs() const {
	std::vector<EvaluatorIOJunction> allIOs;
	for (auto& [connectionId, value] : connectionIOs) {
		allIOs.push_back(value);
	}
	for (auto& [position, branch] : branches) {
		std::vector<EvaluatorIOJunction> newValues = branch.getAllIOs();
		allIOs.insert(allIOs.end(), newValues.begin(), newValues.end());
	}
	return allIOs;
}

inline void AddressTreeNode::setValue(Position position, EvaluatorGate value) {
	values[position] = value;
}


inline void AddressTreeNode::setValue(const Address& address, EvaluatorGate value) {
	getParentBranch(address)->setValue(address.getPosition(address.size() - 1), value);
}

inline std::vector<Address> AddressTreeNode::getPositions(circuit_id_t targetParentContainerId, Position position) const {
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

inline AddressTreeNode* AddressTreeNode::getParentBranch(const Address& address) {
	AddressTreeNode* currentBranch = this;
	for (size_t i = 0; i < address.size() - 1; i++) {
		auto it = currentBranch->branches.find(address.getPosition(i));
		if (it == currentBranch->branches.end()) {
			logError("AddressTree::getParentBranch: address not found");
			return nullptr;
		}
		currentBranch = &(it->second);
	}
	return currentBranch;
}

inline const AddressTreeNode* AddressTreeNode::getParentBranch(const Address& address) const {
	const AddressTreeNode* currentBranch = this;
	for (size_t i = 0; i < address.size() - 1; i++) {
		auto it = currentBranch->branches.find(address.getPosition(i));
		if (it == currentBranch->branches.end()) {
			logError("AddressTree::getParentBranch: address not found");
			return nullptr;
		}
		currentBranch = &(it->second);
	}
	return currentBranch;
}

#endif /* addressTree_h */