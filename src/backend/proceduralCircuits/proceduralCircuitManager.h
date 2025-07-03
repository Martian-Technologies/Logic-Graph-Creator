#ifndef proceduralCircuitManager_h
#define proceduralCircuitManager_h

#include "proceduralCircuit.h"

#include "util/uuid.h"

class ProceduralCircuitManager {
public:
	ProceduralCircuitManager(CircuitManager* circuitManager, DataUpdateEventManager* dataUpdateEventManager);

	template<class ProceduralCircuitType>
	const std::string& createProceduralCircuit(const std::string& name, const std::string& uuid = generate_uuid_v4()) {
		std::shared_ptr<ProceduralCircuitType> proceduralCircuit = std::make_shared<ProceduralCircuitType>(circuitManager, dataUpdateEventManager, name, uuid);
		pathToUUID.emplace(proceduralCircuit->getPath(), uuid);
		auto pair = proceduralCircuits.emplace(uuid, proceduralCircuit);
		return pair.first->first;
	}

	const std::string* getProceduralCircuitUUID(const std::string& path) const;

	SharedProceduralCircuit getProceduralCircuit(const std::string& uuid);
	const SharedProceduralCircuit getProceduralCircuit(const std::string& uuid) const;
	template<class ProceduralCircuitType>
	inline std::shared_ptr<ProceduralCircuitType> getProceduralCircuit(const std::string& uuid) { return dynamic_cast<std::shared_ptr<ProceduralCircuitType>>(getProceduralCircuit(uuid)); }
	template<class ProceduralCircuitType>
	inline const std::shared_ptr<ProceduralCircuitType> getProceduralCircuit(const std::string& uuid) const { return dynamic_cast<std::shared_ptr<ProceduralCircuitType>>(getProceduralCircuit(uuid)); }

	inline const std::map<std::string, SharedProceduralCircuit>& getProceduralCircuits() const { return proceduralCircuits; }

private:
	CircuitManager* circuitManager;
	DataUpdateEventManager* dataUpdateEventManager;
	DataUpdateEventManager::DataUpdateEventReceiver dataUpdateEventReceiver;
	std::map<std::string, SharedProceduralCircuit> proceduralCircuits;
	std::map<std::string, std::string> pathToUUID;
};

#endif /* proceduralCircuitManager_h */
