#include "parsedCircuitLoader.h"

BlockType ParsedCircuitLoader::loadParsedCircuit(SharedParsedCircuit parsedCircuit) {
	CircuitValidator validator(*parsedCircuit, circuitManager->getBlockDataManager());
	circuit_id_t id = circuitManager->createNewCircuit(parsedCircuit.get());
	if (id == 0) return BlockType::NONE;
	return circuitManager->getCircuitBlockDataManager()->getCircuitBlockData(id)->getBlockType();
}
