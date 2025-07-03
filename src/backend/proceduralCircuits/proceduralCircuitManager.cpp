#include "proceduralCircuitManager.h"

ProceduralCircuitManager::ProceduralCircuitManager(CircuitManager* circuitManager, DataUpdateEventManager* dataUpdateEventManager) :
	circuitManager(circuitManager), dataUpdateEventManager(dataUpdateEventManager) { }

const std::string& ProceduralCircuitManager::createNewProceduralCircuit() {
	std::string uuid(generate_uuid_v4());
	ProceduralCircuit proceduralCircuit(circuitManager, dataUpdateEventManager, "And Gate", uuid);
	std::pair<std::map<std::string, ProceduralCircuit>::iterator, bool> pair = proceduralCircuits.emplace(std::move(uuid), std::move(proceduralCircuit));
	return pair.first->first;
}

void ProceduralCircuitManager::createProceduralCircuit(const std::string& uuid) {
	ProceduralCircuit proceduralCircuit(circuitManager, dataUpdateEventManager, "And Gate", uuid);
	proceduralCircuits.emplace(uuid, std::move(proceduralCircuit));
}

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
