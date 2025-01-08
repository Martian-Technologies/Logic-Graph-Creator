#ifndef circuitManager_h
#define circuitManager_h

#include "circuit.h"

class CircuitManager {
public:
	CircuitManager() : lastId(0), blockContainers() { }

	inline std::shared_ptr<Circuit> getContainer(circuit_id_t id) {
		auto iter = blockContainers.find(id);
		if (iter == blockContainers.end()) return nullptr;
		return iter->second;
	}
	inline const std::shared_ptr<Circuit> getContainer(circuit_id_t id) const {
		auto iter = blockContainers.find(id);
		if (iter == blockContainers.end()) return nullptr;
		return iter->second;
	}

	inline circuit_id_t createNewContainer() {
		blockContainers.emplace(getNewContainerId(), std::make_shared<Circuit>(getLastCreatedContainerId()));
		return getLastCreatedContainerId();
	}
	inline void destroyContainer(circuit_id_t id) {
		auto iter = blockContainers.find(id);
		if (iter != blockContainers.end()) {
			blockContainers.erase(iter);
		}
	}


private:
	circuit_id_t getNewContainerId() { return ++lastId; }
	circuit_id_t getLastCreatedContainerId() { return lastId; }

	circuit_id_t lastId;
	std::map<circuit_id_t, std::shared_ptr<Circuit>> blockContainers;
};

#endif /* circuitManager_h */
