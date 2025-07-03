#include "proceduralCircuitManager.h"

ProceduralCircuitManager::ProceduralCircuitManager(CircuitManager* circuitManager, DataUpdateEventManager* dataUpdateEventManager) :
	circuitManager(circuitManager), dataUpdateEventManager(dataUpdateEventManager), dataUpdateEventReceiver(dataUpdateEventManager) {
		dataUpdateEventReceiver.linkFunction("proceduralCircuitPathUpdate", [this](const DataUpdateEventManager::EventData* eventData) {
			auto data = eventData->cast<std::string>();
			if (data) {
				ProceduralCircuit* proceduralCircuit = getProceduralCircuit(data->get());
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

const std::string& ProceduralCircuitManager::createNewProceduralCircuit() {
	std::string uuid(generate_uuid_v4());
	ProceduralCircuit proceduralCircuit(circuitManager, dataUpdateEventManager, "And Gate", uuid);
	pathToUUID.emplace(proceduralCircuit.getPath(), uuid);
	std::pair<std::map<std::string, ProceduralCircuit>::iterator, bool> pair = proceduralCircuits.emplace(std::move(uuid), std::move(proceduralCircuit));
	return pair.first->first;
}

void ProceduralCircuitManager::createProceduralCircuit(const std::string& uuid) {
	ProceduralCircuit proceduralCircuit(circuitManager, dataUpdateEventManager, "And Gate", uuid);
	pathToUUID.emplace(proceduralCircuit.getPath(), uuid);
	proceduralCircuits.emplace(uuid, std::move(proceduralCircuit));
}

const std::string* ProceduralCircuitManager::getProceduralCircuitUUID(const std::string& path) const {
	auto iter = pathToUUID.find(path);
	return (iter == pathToUUID.end()) ? nullptr : &(iter->second);
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
