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
	if (!wasmCode.has_value()) return;

	unsigned int portId = 0;
	unsigned int* portIdPtr = &portId;

	wasmtime::Func tryInsertBlockFunc = wasmtime::Func::wrap(*Wasm::getStore(),
		[circuit](int32_t x, int32_t y, int32_t rotation, int32_t blockType) -> int32_t {
			return circuit->tryInsertBlock(Position(x, y), (Rotation)rotation, (BlockType)blockType);
		});

	wasmtime::Func tryCreateConnectionFunc = wasmtime::Func::wrap(*Wasm::getStore(),
		[circuit](int32_t outputX, int32_t outputY, int32_t inputX, int32_t inputY) -> int32_t {
			return circuit->tryCreateConnection(Position(outputX, outputY), Position(inputX, inputY));
		});

	wasmtime::Func addConnectionInputFunc = wasmtime::Func::wrap(*Wasm::getStore(),
		[circuitBlockData, blockData, portIdPtr](int32_t blockX, int32_t blockY, int32_t portX, int32_t portY) {
			circuitBlockData->setConnectionIdPosition(*portIdPtr, Position(blockX, blockY));
			blockData->setConnectionInput(Vector(portX, portY), *portIdPtr);
			++(*portIdPtr);
		});

	wasmtime::Func addConnectionOutputFunc = wasmtime::Func::wrap(*Wasm::getStore(),
		[circuitBlockData, blockData, portIdPtr](int32_t blockX, int32_t blockY, int32_t portX, int32_t portY) {
			circuitBlockData->setConnectionIdPosition(*portIdPtr, Position(blockX, blockY));
			blockData->setConnectionOutput(Vector(portX, portY), *portIdPtr);
			++(*portIdPtr);
		});

	wasmtime::Func setSizeFunc = wasmtime::Func::wrap(*Wasm::getStore(),
		[blockData](int32_t width, int32_t height) {
			blockData->setSize(Vector(width, height));
		});

	std::optional<wasmtime::Memory> memory;
	std::optional<wasmtime::Memory>* memoryPtr = &memory;

	wasmtime::Func logInfoFunc = wasmtime::Func::wrap(*Wasm::getStore(),
		[memoryPtr](int32_t strOffset) {
			auto memSpan = memoryPtr->value().data(*Wasm::getStore());
			const char* str = (const char*)(memSpan.data() + strOffset);
			logInfo(std::string(str, strnlen(str, memSpan.size() - strOffset)), "WasmProceduralCircuit > WasmCode");
		});

	wasmtime::Func logErrorFunc = wasmtime::Func::wrap(*Wasm::getStore(),
		[memoryPtr](int32_t strOffset) {
			auto memSpan = memoryPtr->value().data(*Wasm::getStore());
			const char* str = (const char*)(memSpan.data() + strOffset);
			logError(std::string(str, strnlen(str, memSpan.size() - strOffset)), "WasmProceduralCircuit > WasmCode");
		});

	// Linker to associate "env" functions
	wasmtime::Linker linker(*Wasm::getEngine());
	wasmtime::Result<std::__1::monostate> linkerResult = wasmtime::Result<std::__1::monostate>(nullptr);
	linkerResult = linker.define(*Wasm::getStore(), "env", "tryInsertBlock", tryInsertBlockFunc);
	if (!linkerResult) return;
	linkerResult = linker.define(*Wasm::getStore(), "env", "tryCreateConnection", tryCreateConnectionFunc);
	if (!linkerResult) return;
	linkerResult = linker.define(*Wasm::getStore(), "env", "addConnectionInput", addConnectionInputFunc);
	if (!linkerResult) return;
	linkerResult = linker.define(*Wasm::getStore(), "env", "addConnectionOutput", addConnectionOutputFunc);
	if (!linkerResult) return;
	linkerResult = linker.define(*Wasm::getStore(), "env", "setSize", setSizeFunc);
	if (!linkerResult) return;
	linkerResult = linker.define(*Wasm::getStore(), "env", "logInfo", logInfoFunc);
	if (!linkerResult) return;
	linkerResult = linker.define(*Wasm::getStore(), "env", "logError", logErrorFunc);
	if (!linkerResult) return;

	// Instantiate the module
	auto instanceResult = linker.instantiate(*Wasm::getStore(), wasmCode.value());
	if (!instanceResult) {
		logError("Failed to instantiate WASM module.", "WasmProceduralCircuit");
		return;
	}
	auto instance = instanceResult.unwrap();

	std::optional<wasmtime::Extern> memoryExport = instance.get(*Wasm::getStore(), "memory");
	if (!memoryExport) return;
	memory.emplace(std::get<wasmtime::Memory>(memoryExport.value()));

	auto func = std::get<wasmtime::Func>(*instance.get(*Wasm::getStore(), "generateCircuit"));
	auto results = func.call(*Wasm::getStore(), {}).unwrap();
	bool success = results.front().i32();
}
