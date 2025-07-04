#ifndef wasmProceduralCircuit_h
#define wasmProceduralCircuit_h

#include "proceduralCircuit.h"

#include "backend/wasm/wasm.h"

class WasmProceduralCircuit : public ProceduralCircuit {
public:
	class WasmInstance {
	public:
		WasmInstance(wasmtime::Module module);
		WasmInstance(WasmInstance&& wasmInstance);

		WasmInstance& operator=(WasmInstance&& wasmInstance);

		void makeCircuit(const ProceduralCircuitParameters& parameters, SharedCircuit circuit, BlockData* blockData, CircuitBlockData* circuitBlockData);

		inline bool isValid() const { return valid; }
		inline const std::string& getName() const { return name; }
		inline const std::string& getUUID() const { return UUID; }

	private:
		std::string wasmToString(int32_t wasmPtr);

		std::optional<wasmtime::Instance> instance;
		std::optional<wasmtime::Memory> memory;

		// gotten on load
		bool valid = false;
		std::string name;
		std::string UUID;

		// per wasm run need data
		mutable unsigned int portId = 0;
		mutable SharedCircuit circuit = nullptr;
		mutable BlockData* blockData = nullptr;
		mutable CircuitBlockData* circuitBlockData = nullptr;
	};

	WasmProceduralCircuit(
		CircuitManager* circuitManager,
		DataUpdateEventManager* dataUpdateEventManager,
		WasmInstance&& wasmInstance
	);
	WasmProceduralCircuit(WasmProceduralCircuit&& other);

	void setWasm(WasmInstance&& wasmInstance);

private:
	void makeCircuit(const ProceduralCircuitParameters& parameters, SharedCircuit circuit, BlockData* blockData, CircuitBlockData* circuitBlockData) override final;

	WasmInstance wasmInstance;
};

typedef std::shared_ptr<WasmProceduralCircuit> SharedWasmProceduralCircuit;


#endif /* wasmProceduralCircuit_h */
