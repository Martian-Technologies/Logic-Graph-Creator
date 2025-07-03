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

		// std::optional<wasmtime::Module> helloMod = Wasm::loadModule((DirectoryManager().getResourceDirectory() / "hello.wat").string());
		// if (helloMod) {
		// 	wasmtime::Module helloModule = helloMod.value();
		// 	logInfo("Number of exports: {}", "", helloModule.exports().size());
		// 	wasmtime::Func host_func = wasmtime::Func::wrap(*Wasm::getStore(), []() { std::cout << "Calling back...\n"; });
		// 	auto instanceResult = wasmtime::Instance::create(*Wasm::getStore(), helloModule, {host_func});
		// 	if (!instanceResult) {
		// 		logError("Instance creation failed: {}", "Wasm", instanceResult.err().message());
		// 	} else {
		// 		auto instance = instanceResult.unwrap();

		// 		// Run the exported function
		// 		auto func = std::get<wasmtime::Func>(*instance.get(*Wasm::getStore(), "run"));
		// 		auto results = func.call(*Wasm::getStore(), {}).unwrap();
		// 		// logInfo(results[0].kind());
		// 	}
		// } else {
		// 	logError("Failed to load hello.wat module", "Wasm");
		// }
