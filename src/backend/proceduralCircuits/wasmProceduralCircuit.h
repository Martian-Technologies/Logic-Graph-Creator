#ifndef wasmProceduralCircuit_h
#define wasmProceduralCircuit_h

#include "proceduralCircuit.h"

#include "backend/wasm/wasm.h"

class CircuitFileManager;

class WasmProceduralCircuit : public ProceduralCircuit {
public:
	class WasmInstance {
	public:
		WasmInstance(wasmtime::Module module, CircuitManager* circuitManager, CircuitFileManager* fileManager);
		WasmInstance(WasmInstance&& wasmInstance);

		WasmInstance& operator=(WasmInstance&& wasmInstance);

		void makeCircuit(const ProceduralCircuitParameters& parameters, GeneratedCircuit& generatedCircuit);

		inline bool isValid() const { return valid; }
		inline const std::string& getName() const { return name; }
		inline const std::string& getUUID() const { return UUID; }
		inline const ProceduralCircuitParameters& getDefaultParameters() const { return defaultParameters; }

	private:
		std::string wasmToString(int32_t wasmPtr);

		std::optional<wasmtime::Instance> instance;
		std::optional<wasmtime::Memory> memory;

		CircuitManager* circuitManager;
		CircuitFileManager* fileManager;

		// gotten on load
		bool valid = false;
		std::string name;
		std::string UUID;
		ProceduralCircuitParameters defaultParameters;

		// per wasm run need data
		mutable unsigned int blockId = 0;
		mutable unsigned int portId = 0;
		mutable const ProceduralCircuitParameters* parameters = nullptr;
		mutable GeneratedCircuit* generatedCircuit = nullptr;

		std::unique_ptr<WasmInstance*> thisPtr;
	};

	WasmProceduralCircuit(
		CircuitManager* circuitManager,
		DataUpdateEventManager* dataUpdateEventManager,
		WasmInstance&& wasmInstance
	);
	WasmProceduralCircuit(WasmProceduralCircuit&& other);

	void setWasm(WasmInstance&& wasmInstance);

private:
	void makeCircuit(const ProceduralCircuitParameters& parameters, GeneratedCircuit& generatedCircuit) override final;

	WasmInstance wasmInstance;

};

typedef std::shared_ptr<WasmProceduralCircuit> SharedWasmProceduralCircuit;


#endif /* wasmProceduralCircuit_h */
