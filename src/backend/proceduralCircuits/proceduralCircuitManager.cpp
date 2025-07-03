#include "proceduralCircuitManager.h"

ProceduralCircuitManager::ProceduralCircuitManager(CircuitManager* circuitManager, DataUpdateEventManager* dataUpdateEventManager) :
	circuitManager(circuitManager), dataUpdateEventManager(dataUpdateEventManager), dataUpdateEventReceiver(dataUpdateEventManager) {
	dataUpdateEventReceiver.linkFunction("proceduralCircuitPathUpdate", [this](const DataUpdateEventManager::EventData* eventData) {
		auto data = eventData->cast<std::string>();
		if (data) {
			SharedProceduralCircuit proceduralCircuit = getProceduralCircuit(data->get());
			for (auto iter = pathToUUID.begin(); iter != pathToUUID.end(); ++iter) {
				if (iter->second == proceduralCircuit->getUUID()) {
					pathToUUID.erase(iter);
					pathToUUID.emplace(proceduralCircuit->getPath(), proceduralCircuit->getUUID());
					return;
				}
			}
		}
	});
}

const std::string* ProceduralCircuitManager::getProceduralCircuitUUID(const std::string& path) const {
	auto iter = pathToUUID.find(path);
	return (iter == pathToUUID.end()) ? nullptr : &(iter->second);
}

SharedProceduralCircuit ProceduralCircuitManager::getProceduralCircuit(const std::string& uuid) {
	auto iter = proceduralCircuits.find(uuid);
	if (iter == proceduralCircuits.end()) return nullptr;
	return iter->second;
}

const SharedProceduralCircuit ProceduralCircuitManager::getProceduralCircuit(const std::string& uuid) const {
	auto iter = proceduralCircuits.find(uuid);
	if (iter == proceduralCircuits.end()) return nullptr;
	return iter->second;
}
