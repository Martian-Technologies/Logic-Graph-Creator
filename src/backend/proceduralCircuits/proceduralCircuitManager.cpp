#include "proceduralCircuitManager.h"

ProceduralCircuitManager::ProceduralCircuitManager(CircuitManager* circuitManager, DataUpdateEventManager* dataUpdateEventManager) :
	circuitManager(circuitManager), dataUpdateEventManager(dataUpdateEventManager) { }


const ProceduralCircuit* ProceduralCircuitManager::getProceduralCircuit(const std::string& uuid) const {
	auto iter = proceduralCircuits.find(uuid);
	if (iter == proceduralCircuits.end()) return nullptr;
	return &(iter->second);
}

ProceduralCircuit* ProceduralCircuitManager::getProceduralCircuit(const std::string& uuid) {
	auto iter = proceduralCircuits.find(uuid);
	if (iter == proceduralCircuits.end()) return nullptr;
	return &(iter->second);
}
