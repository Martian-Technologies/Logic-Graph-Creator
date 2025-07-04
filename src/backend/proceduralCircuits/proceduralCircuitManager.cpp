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

const std::string& ProceduralCircuitManager::createWasmProceduralCircuit(wasmtime::Module wasmModule) {
	WasmProceduralCircuit::WasmInstance wasmInstance(wasmModule);
	if (!wasmInstance.isValid()) {
		logError("createWasmProceduralCircuit failed because wasmInstance was not valid.", "ProceduralCircuitManager");
	}
	SharedProceduralCircuit proceduralCircuit = getProceduralCircuit(wasmInstance.getUUID());
	if (proceduralCircuit) {
		SharedWasmProceduralCircuit wasmProceduralCircuit = std::static_pointer_cast<WasmProceduralCircuit>(proceduralCircuit);
		if (wasmProceduralCircuit) {
			logInfo("WasmProceduralCircuit with UUID {} already exists. Update wasm in WasmProceduralCircuit", "ProceduralCircuitManager", wasmInstance.getUUID());
			wasmProceduralCircuit->setWasm(std::move(wasmInstance));
		} else {
			logError("Non Wasm ProceduralCircuit with UUID {} already exists. Can't create WasmProceduralCircuit.", "ProceduralCircuitManager", wasmInstance.getUUID());
		}
	} else {
		SharedWasmProceduralCircuit proceduralCircuit = std::make_shared<WasmProceduralCircuit>(circuitManager, dataUpdateEventManager, std::move(wasmInstance));
		pathToUUID.emplace(proceduralCircuit->getPath(), proceduralCircuit->getUUID());
		proceduralCircuits.emplace(proceduralCircuit->getUUID(), proceduralCircuit);
		return proceduralCircuit->getUUID();
	}
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
