#ifndef proceduralCircuitManager_h
#define proceduralCircuitManager_h

#include "proceduralCircuit.h"

#include "util/uuid.h"

class ProceduralCircuitManager {
public:
	ProceduralCircuitManager(CircuitManager* circuitManager, DataUpdateEventManager* dataUpdateEventManager);

	const std::string& createNewProceduralCircuit();
	void createProceduralCircuit(const std::string& uuid);

	const std::string* getProceduralCircuitUUID(const std::string& path) const;

	ProceduralCircuit* getProceduralCircuit(const std::string& uuid);
	const ProceduralCircuit* getProceduralCircuit(const std::string& uuid) const;

	inline const std::map<std::string, ProceduralCircuit>& getProceduralCircuits() const { return proceduralCircuits; }

private:
	CircuitManager* circuitManager;
	DataUpdateEventManager* dataUpdateEventManager;
	DataUpdateEventManager::DataUpdateEventReceiver dataUpdateEventReceiver;
	std::map<std::string, ProceduralCircuit> proceduralCircuits;
	std::map<std::string, std::string> pathToUUID;
};

#endif /* proceduralCircuitManager_h */
