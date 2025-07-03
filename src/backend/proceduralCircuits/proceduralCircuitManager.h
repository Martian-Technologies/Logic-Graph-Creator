#ifndef proceduralCircuitManager_h
#define proceduralCircuitManager_h

#include "proceduralCircuit.h"

class ProceduralCircuitManager {
public:
	ProceduralCircuitManager(CircuitManager* circuitManager, DataUpdateEventManager* dataUpdateEventManager) : circuitManager(circuitManager), dataUpdateEventManager(dataUpdateEventManager) {
		std::string uuid = generate_uuid_v4();
		proceduralCircuits.emplace(uuid, ProceduralCircuit(circuitManager, dataUpdateEventManager, "And Gate", uuid));
	}

	const ProceduralCircuit* getProceduralCircuit(const std::string& uuid) const;
	ProceduralCircuit* getProceduralCircuit(const std::string& uuid);

private:
	CircuitManager* circuitManager;
	DataUpdateEventManager* dataUpdateEventManager;
	std::map<std::string, ProceduralCircuit> proceduralCircuits;
};

#endif /* proceduralCircuitManager_h */
