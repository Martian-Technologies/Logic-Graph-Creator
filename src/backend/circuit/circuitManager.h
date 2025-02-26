#ifndef circuitManager_h
#define circuitManager_h

#include "circuit.h"

class CircuitManager {
public:
	inline SharedCircuit getCircuit(circuit_id_t id) {
		auto iter = circuits.find(id);
		if (iter == circuits.end()) return nullptr;
		return iter->second;
	}
	inline const SharedCircuit getCircuit(circuit_id_t id) const {
		auto iter = circuits.find(id);
		if (iter == circuits.end()) return nullptr;
		return iter->second;
	}

	inline circuit_id_t createNewCircuit() {
		circuit_id_t id = getNewCircuitId();
		circuits.emplace(id, std::make_shared<Circuit>(id));
		return id;
	}

	inline void destroyCircuit(circuit_id_t id) {
		auto iter = circuits.find(id);
		if (iter != circuits.end()) {
			circuits.erase(iter);
		}
	}

	typedef std::map<circuit_id_t, SharedCircuit>::iterator iterator;
	typedef std::map<circuit_id_t, SharedCircuit>::const_iterator const_iterator;

	inline iterator begin() { return circuits.begin(); }
	inline iterator end() { return circuits.end(); }
	inline const_iterator begin() const { return circuits.begin(); }
	inline const_iterator end() const { return circuits.end(); }


private:
	circuit_id_t getNewCircuitId() { return ++lastId; }

	circuit_id_t lastId = 0;
	std::map<circuit_id_t, SharedCircuit> circuits;
};

#endif /* circuitManager_h */
