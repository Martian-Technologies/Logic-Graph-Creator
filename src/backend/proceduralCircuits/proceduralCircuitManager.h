#ifndef proceduralCircuitManager_h
#define proceduralCircuitManager_h

#include "proceduralCircuit.h"

#include "util/uuid.h"

class ProceduralCircuitManager {
public:
	ProceduralCircuitManager(CircuitManager* circuitManager, DataUpdateEventManager* dataUpdateEventManager);

	void test() {
		std::string uuid = generate_uuid_v4();
		proceduralCircuits.emplace(uuid, ProceduralCircuit(circuitManager, dataUpdateEventManager, "And Gate", uuid));
		logInfo(getProceduralCircuit(uuid)->getCircuitId(ProceduralCircuitParameters()));
	}

	ProceduralCircuit* getProceduralCircuit(const std::string& uuid);
	const ProceduralCircuit* getProceduralCircuit(const std::string& uuid) const;

private:
	CircuitManager* circuitManager;
	DataUpdateEventManager* dataUpdateEventManager;
	std::map<std::string, ProceduralCircuit> proceduralCircuits;
};

#endif /* proceduralCircuitManager_h */
