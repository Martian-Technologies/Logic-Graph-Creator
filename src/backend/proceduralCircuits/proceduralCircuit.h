#ifndef proceduralCircuit_h
#define proceduralCircuit_h

#include "backend/dataUpdateEventManager.h"
#include "backend/circuit/circuit.h"

class CircuitManager;

typedef unsigned int procedural_circuit_id_t;

struct ProceduralCircuitParameters {
	std::string toString() const {
		std::string str = "(";
		for (const auto& iter : parameters) {
			if (str.size() != 1) str += ", ";
			str += iter.first + ": " + std::to_string(iter.second);
		}
		return str + ")";
	}

	bool operator==(const ProceduralCircuitParameters& other) const {
		return parameters == other.parameters;
	}

	std::map<std::string, int> parameters;
};

template<>
struct std::hash<ProceduralCircuitParameters> {
	inline std::size_t operator()(const ProceduralCircuitParameters& parameters) const noexcept {
		std::size_t seed = 0;
		for (const auto& iter : parameters.parameters) {
			std::size_t a = std::hash<std::string> {}(iter.first);
			std::size_t b = std::hash<int> {}(iter.second);
			seed = a + 0x9e3779b9 + (seed << 6) + (seed >> 2); // this is what boost::hash_combine does
			seed = b + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}
		return seed;
	}
};

class ProceduralCircuit {
public:
	ProceduralCircuit(
		CircuitManager* circuitManager,
		DataUpdateEventManager* dataUpdateEventManager,
		const std::string& name,
		const std::string& uuid
	);
	ProceduralCircuit(ProceduralCircuit&& other);
	~ProceduralCircuit();

	inline std::string getPath() const { return "Procedural Circuits/" + getProceduralCircuitName(); }

	inline const std::string& getUUID() const { return proceduralCircuitUUID; }
	inline std::string getProceduralCircuitNameUUID() const { return proceduralCircuitName + " : " + proceduralCircuitUUID; }
	inline const std::string& getProceduralCircuitName() const { return proceduralCircuitName; }
	void setProceduralCircuitName(const std::string& name);

	inline void setParameterDefaults(const ProceduralCircuitParameters& parameterDefaults) { this->parameterDefaults = parameterDefaults; }
	circuit_id_t getCircuitId(const ProceduralCircuitParameters& parameters);
	BlockType getBlockType(const ProceduralCircuitParameters& parameters);

private:
	std::string proceduralCircuitName;
	std::string proceduralCircuitUUID;

	ProceduralCircuitParameters	parameterDefaults;

	CircuitManager* circuitManager;
	std::unordered_map<ProceduralCircuitParameters, circuit_id_t> generatedCircuits;

	DataUpdateEventManager* dataUpdateEventManager;
	DataUpdateEventManager::DataUpdateEventReceiver dataUpdateEventReceiver;
};

#endif /* proceduralCircuit_h */
