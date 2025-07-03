#include "wasmProceduralCircuit.h"

#include "backend/circuit/circuitBlockData.h"

WasmProceduralCircuit::WasmProceduralCircuit(
	CircuitManager* circuitManager,
	DataUpdateEventManager* dataUpdateEventManager,
	const std::string& name,
	const std::string& uuid
) : ProceduralCircuit(circuitManager, dataUpdateEventManager, name, uuid) { }

WasmProceduralCircuit::WasmProceduralCircuit(WasmProceduralCircuit&& other) : ProceduralCircuit(static_cast<ProceduralCircuit&&>(other)), wasmCode(std::move(other.wasmCode)) { }

void WasmProceduralCircuit::makeCircuit(const ProceduralCircuitParameters& parameters, SharedCircuit circuit, BlockData* blockData, CircuitBlockData* circuitBlockData) {
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
}
