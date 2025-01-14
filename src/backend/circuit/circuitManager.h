#ifndef circuitManager_h
#define circuitManager_h

#include "circuit.h"

class CircuitManager {
public:
	inline SharedCircuit getCircuit(circuit_id_t id) {
		auto iter = blockContainers.find(id);
		if (iter == circuits.end()) return nullptr;
		return iter->second;
	}
	inline const SharedCircuit getCircuit(circuit_id_t id) const {
		auto iter = circuits.find(id);
		if (iter == circuits.end()) return nullptr;
		return iter->second;
	}

	inline circuit_id_t createNewCircuit() {
		circuits.emplace(getNewCircuitId(), std::make_shared<Circuit>(getLastCreatedCircuitId()));
		return getLastCreatedCircuitId();
	}
	inline void destroyCircuit(circuit_id_t id) {
		auto iter = circuits.find(id);
		if (iter != circuits.end()) {
			circuits.erase(iter);
		}
	}


private:
	circuit_id_t getNewCircuitId() { return ++lastId; }
	circuit_id_t getLastCreatedCircuitId() { return lastId; }

	circuit_id_t lastId = 0;
	std::map<circuit_id_t, SharedCircuit> circuits;
};

#endif /* circuitManager_h */
