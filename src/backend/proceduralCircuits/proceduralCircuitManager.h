#ifndef proceduralCircuitManager_h
#define proceduralCircuitManager_h

#include "proceduralCircuit.h"

#include "util/uuid.h"

class ProceduralCircuitManager {
public:
	ProceduralCircuitManager(CircuitManager* circuitManager, DataUpdateEventManager* dataUpdateEventManager);

	const std::string& createNewProceduralCircuit();
	void createProceduralCircuit(const std::string& uuid);


	ProceduralCircuit* getProceduralCircuit(const std::string& uuid);
	const ProceduralCircuit* getProceduralCircuit(const std::string& uuid) const;

	inline const std::map<std::string, ProceduralCircuit>& getProceduralCircuits() const { return proceduralCircuits; }

private:
	CircuitManager* circuitManager;
	DataUpdateEventManager* dataUpdateEventManager;
	std::map<std::string, ProceduralCircuit> proceduralCircuits;
};

#endif /* proceduralCircuitManager_h */
