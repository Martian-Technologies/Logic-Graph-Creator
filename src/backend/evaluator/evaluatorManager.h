#ifndef evaluatorManager_h
#define evaluatorManager_h

#include "evaluator.h"

class EvaluatorManager {
public:
	EvaluatorManager() : lastId(0), evaluator() { }

	inline SharedCircuit getCircuit(circuit_id_t id) {
		auto iter = evaluator.find(id);
		if (iter == evaluator.end()) return nullptr;
		return iter->second;
	}
	inline const SharedCircuit getCircuit(circuit_id_t id) const {
		auto iter = evaluator.find(id);
		if (iter == evaluator.end()) return nullptr;
		return iter->second;
	}

	inline circuit_id_t createNewCircuit() {
		evaluator.emplace(getNewCircuitId(), std::make_shared<Circuit>(getLastCreatedCircuitId()));
		return getLastCreatedCircuitId();
	}
	inline void destroyCircuit(circuit_id_t id) {
		auto iter = evaluator.find(id);
		if (iter != evaluator.end()) {
			evaluator.erase(iter);
		}
	}


private:
	evaluator_id_t getNewCircuitId() { return ++lastId; }
	evaluator_id_t getLastCreatedCircuitId() { return lastId; }

	evaluator_id_t lastId;
	std::map<evaluator_id_t, SharedCircuit> blockContainers;
};

#endif /* evaluatorManager_h */
