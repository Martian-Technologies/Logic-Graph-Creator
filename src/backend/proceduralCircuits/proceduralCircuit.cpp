#include "proceduralCircuit.h"

#include "../circuit/circuitManager.h"

ProceduralCircuit::ProceduralCircuit(
	CircuitManager* circuitManager,
	DataUpdateEventManager* dataUpdateEventManager,
	const std::string& name,
	const std::string& uuid
) : circuitManager(circuitManager), dataUpdateEventManager(dataUpdateEventManager), dataUpdateEventReceiver(dataUpdateEventManager), proceduralCircuitName(name), proceduralCircuitUUID(uuid) { }

ProceduralCircuit::ProceduralCircuit(ProceduralCircuit&& other) :
	proceduralCircuitName(std::move(other.proceduralCircuitName)), proceduralCircuitUUID(std::move(other.proceduralCircuitUUID)),
	circuitManager(other.circuitManager), generatedCircuits(std::move(other.generatedCircuits)),
	dataUpdateEventManager(other.dataUpdateEventManager), dataUpdateEventReceiver(std::move(other.dataUpdateEventReceiver)) { }

ProceduralCircuit::~ProceduralCircuit() {
	// Destroy unused generated circuits. If some are still used ask the user what to do.
}

void ProceduralCircuit::setProceduralCircuitName(const std::string& name) {
	this->proceduralCircuitName = name;
	for (const auto& iter : generatedCircuits) {
		SharedCircuit circuit = circuitManager->getCircuit(iter.second);
		if (circuit) circuit->setCircuitName(name + iter.first.toString());
	}
}

circuit_id_t ProceduralCircuit::getCircuitId(const ProceduralCircuitParameters& parameters) {
	// Make sure to only use parameters that are reconized (anything in parameterDefaults)
	ProceduralCircuitParameters realParameters = parameterDefaults;
	for (auto& iter : realParameters.parameters) {
		auto iter2 = parameters.parameters.find(iter.first);
		if (iter2 != parameters.parameters.end()) {
			iter.second = iter2->second;
		}
	}

	// Check if its already been generated
	auto iter = generatedCircuits.find(realParameters);
	if (iter != generatedCircuits.end()) {
		return iter->second;
	}

	logInfo("Creating circuit with parameters: {}", "ProceduralCircuit", realParameters.toString());

	// Create the circuit if it has not been generated
	circuit_id_t id = circuitManager->createNewCircuit();

	// Add the circuit id to the generated circuits
	generatedCircuits[realParameters] = id;

	// Setup the block to be a IC
	BlockType type = circuitManager->setupBlockData(id);
	if (type == BlockType::NONE) return 0;

	// Get useful objects
	SharedCircuit circuit = circuitManager->getCircuit(id);
	BlockData* blockData = circuitManager->getBlockDataManager()->getBlockData(type);
	CircuitBlockData* circuitBlockData = circuitManager->getCircuitBlockDataManager()->getCircuitBlockData(id);

	// Make the block

	blockData->setSize(Vector(2, 2));

	circuit->tryInsertBlock(Position(-1, 1), Rotation::ZERO, BlockType::SWITCH);
	circuitBlockData->setConnectionIdPosition(0, Position(-1, 1));
	blockData->setConnectionInput(Vector(0, 0), 0);

	circuit->tryInsertBlock(Position(-1, -1), Rotation::ZERO, BlockType::SWITCH);
	circuitBlockData->setConnectionIdPosition(1, Position(-1, -1));
	blockData->setConnectionInput(Vector(0, 1), 1);

	circuit->tryInsertBlock(Position(1, 0), Rotation::ZERO, BlockType::LIGHT);
	circuitBlockData->setConnectionIdPosition(2, Position(1, 0));
	blockData->setConnectionOutput(Vector(1, 0), 2);

	circuit->tryInsertBlock(Position(0, 0), Rotation::ZERO, BlockType::AND);

	circuit->tryCreateConnection(Position(-1, 1), Position(0, 0));
	circuit->tryCreateConnection(Position(-1, -1), Position(0, 0));
	circuit->tryCreateConnection(Position(0, 0), Position(1, 0));

	return id;
}
