#ifndef wasmProceduralCircuit_h
#define wasmProceduralCircuit_h

#include "proceduralCircuit.h"

#include "backend/wasm/wasm.h"

class WasmProceduralCircuit : public ProceduralCircuit {
public:
	WasmProceduralCircuit(
		CircuitManager* circuitManager,
		DataUpdateEventManager* dataUpdateEventManager,
		const std::string& name,
		const std::string& uuid
	);
	WasmProceduralCircuit(WasmProceduralCircuit&& other);

	void setWasm(wasmtime::Module wasmCode) { this->wasmCode.emplace(wasmCode); regenerateAll(); }

private:
	void makeCircuit(const ProceduralCircuitParameters& parameters, SharedCircuit circuit, BlockData* blockData, CircuitBlockData* circuitBlockData) override final;

	std::optional<wasmtime::Module> wasmCode;
};

#endif /* wasmProceduralCircuit_h */
