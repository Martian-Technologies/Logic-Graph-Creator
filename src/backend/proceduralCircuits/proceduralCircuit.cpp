#include "proceduralCircuit.h"

ProceduralCircuit::ProceduralCircuit(
	CircuitManager* circuitManager,
	DataUpdateEventManager* dataUpdateEventManager,
	const std::string& name,
	const std::string& uuid
) : circuitManager(circuitManager), dataUpdateEventManager(dataUpdateEventManager), proceduralCircuitName(name), proceduralCircuitUUID(uuid) {

}