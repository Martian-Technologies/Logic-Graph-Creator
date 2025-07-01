#ifndef proceduralCircuit_h
#define proceduralCircuit_h

#include "../circuit/circuitManager.h"

typedef unsigned int procedural_circuit_id_t;

class ProceduralCircuit {
public:
	struct ProceduralCircuitParameters {
		std::map<std::string, int> parameters;
	};

	ProceduralCircuit(
		CircuitManager* circuitManager,
		DataUpdateEventManager* dataUpdateEventManager,
		const std::string& name,
		const std::string& uuid
	);
	~ProceduralCircuit();

	inline const std::string& getUUID() const { return proceduralCircuitUUID; }
	inline std::string getProceduralCircuitNameUUID() const { return proceduralCircuitName + " : " + proceduralCircuitUUID; }
	inline const std::string& getProceduralCircuitName() const { return proceduralCircuitName; }
	void setProceduralCircuitName(const std::string& name);

	void setParameterKeys(std::vector<std::string> parameterKeys);
	circuit_id_t getCircuitId(const ProceduralCircuitParameters& parameters);

private:
	std::string proceduralCircuitName;
	std::string proceduralCircuitUUID;
	std::map<ProceduralCircuitParameters, circuit_id_t> generatedCircuits;
	CircuitManager* circuitManager;
	DataUpdateEventManager* dataUpdateEventManager;
	DataUpdateEventManager::DataUpdateEventReceiver dataUpdateEventReceiver;
};

#endif /* proceduralCircuit_h */