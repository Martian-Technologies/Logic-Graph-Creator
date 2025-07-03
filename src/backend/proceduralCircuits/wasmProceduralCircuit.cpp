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
	
	// blockData->setSize(Vector(2, 2));

	// circuit->tryInsertBlock(Position(-1, 1), Rotation::ZERO, BlockType::SWITCH);
	// circuitBlockData->setConnectionIdPosition(0, Position(-1, 1));
	// blockData->setConnectionInput(Vector(0, 0), 0);

	// circuit->tryInsertBlock(Position(-1, -1), Rotation::ZERO, BlockType::SWITCH);
	// circuitBlockData->setConnectionIdPosition(1, Position(-1, -1));
	// blockData->setConnectionInput(Vector(0, 1), 1);

	// circuit->tryInsertBlock(Position(1, 0), Rotation::ZERO, BlockType::LIGHT);
	// circuitBlockData->setConnectionIdPosition(2, Position(1, 0));
	// blockData->setConnectionOutput(Vector(1, 0), 2);

	// circuit->tryInsertBlock(Position(0, 0), Rotation::ZERO, BlockType::AND);

	// circuit->tryCreateConnection(Position(-1, 1), Position(0, 0));
	// circuit->tryCreateConnection(Position(-1, -1), Position(0, 0));
	// circuit->tryCreateConnection(Position(0, 0), Position(1, 0));

	// bool tryInsertBlock(cord_t x, cord_t y, Rotation rotation, BlockType blockType);
	// bool tryCreateConnection(cord_t outputX, cord_t outputY, cord_t inputX, cord_t inputY);
	// void addConnectionInput(cord_t blockX, cord_t blockY, cord_t portX, cord_t portY);
	// void addConnectionOutput(cord_t blockX, cord_t blockY, cord_t portX, cord_t portY);
	// void setSize(cord_t width, cord_t height);
	// void logInfo(const char* msg);
	// void logError(const char* msg);

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

    // wasmtime::Func logInfoFunc = wasmtime::Func::wrap(*Wasm::getStore(),
    //     [circuit](int32_t outputX, int32_t outputY, int32_t inputX, int32_t inputY) {
    //         return circuit->tryCreateConnection(Position(outputX, outputY), Position(inputX, inputY));
    //     });
    // wasmtime::Func logErrorFunc = wasmtime::Func::wrap(*Wasm::getStore(),
    //     [circuit](int32_t outputX, int32_t outputY, int32_t inputX, int32_t inputY) {
    //         return circuit->tryCreateConnection(Position(outputX, outputY), Position(inputX, inputY));
    //     });

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

    // Instantiate the module
    auto instanceResult = linker.instantiate(*Wasm::getStore(), wasmCode.value());
    if (!instanceResult) {
        logError("Failed to instantiate WASM module.", "WasmProceduralCircuit");
        return;
    }
	auto instance = instanceResult.unwrap();

	auto func = std::get<wasmtime::Func>(*instance.get(*Wasm::getStore(), "generateCircuit"));
	auto results = func.call(*Wasm::getStore(), {}).unwrap();
	logInfo("results sise: {}", "WasmProceduralCircuit", results.size());
}
