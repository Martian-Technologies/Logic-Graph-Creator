#include "wasmProceduralCircuit.h"

#include "backend/circuit/circuitBlockData.h"

WasmProceduralCircuit::WasmInstance::WasmInstance(wasmtime::Module module) : thisPtr(std::make_unique<WasmInstance*>(this)) {
	WasmInstance** thisPtrPtr = thisPtr.get();
	wasmtime::Func getParameterFunc = wasmtime::Func::wrap(*Wasm::getStore(),
		[thisPtrPtr](int32_t strOffset) -> int32_t {
			const std::map<std::string, int>& parameters = (*thisPtrPtr)->parameters->parameters;
			auto iter = parameters.find((*thisPtrPtr)->wasmToString(strOffset));
			return (iter == parameters.end()) ? 0 : iter->second;
		});

	wasmtime::Func tryInsertBlockFunc = wasmtime::Func::wrap(*Wasm::getStore(),
		[thisPtrPtr](int32_t x, int32_t y, int32_t rotation, int32_t blockType) -> int32_t {
			return (*thisPtrPtr)->circuit->tryInsertBlock(Position(x, y), (Rotation)rotation, (BlockType)blockType);
		});

	wasmtime::Func tryCreateConnectionFunc = wasmtime::Func::wrap(*Wasm::getStore(),
		[thisPtrPtr](int32_t outputX, int32_t outputY, int32_t inputX, int32_t inputY) -> int32_t {
			return (*thisPtrPtr)->circuit->tryCreateConnection(Position(outputX, outputY), Position(inputX, inputY));
		});

	wasmtime::Func addConnectionInputFunc = wasmtime::Func::wrap(*Wasm::getStore(),
		[thisPtrPtr](int32_t blockX, int32_t blockY, int32_t portX, int32_t portY) {
			(*thisPtrPtr)->circuitBlockData->setConnectionIdPosition((*thisPtrPtr)->portId, Position(blockX, blockY));
			(*thisPtrPtr)->blockData->setConnectionInput(Vector(portX, portY), (*thisPtrPtr)->portId);
			++((*thisPtrPtr)->portId);
		});

	wasmtime::Func addConnectionOutputFunc = wasmtime::Func::wrap(*Wasm::getStore(),
		[thisPtrPtr](int32_t blockX, int32_t blockY, int32_t portX, int32_t portY) {
			(*thisPtrPtr)->circuitBlockData->setConnectionIdPosition((*thisPtrPtr)->portId, Position(blockX, blockY));
			(*thisPtrPtr)->blockData->setConnectionOutput(Vector(portX, portY), (*thisPtrPtr)->portId);
			++((*thisPtrPtr)->portId);
		});

	wasmtime::Func setSizeFunc = wasmtime::Func::wrap(*Wasm::getStore(),
		[thisPtrPtr](int32_t width, int32_t height) {
			(*thisPtrPtr)->blockData->setSize(Vector(width, height));
		});

	wasmtime::Func logInfoFunc = wasmtime::Func::wrap(*Wasm::getStore(),
		[thisPtrPtr](int32_t strOffset) {
			logInfo((*thisPtrPtr)->wasmToString(strOffset), "WasmProceduralCircuit > WasmCode");
		});

	wasmtime::Func logErrorFunc = wasmtime::Func::wrap(*Wasm::getStore(),
		[thisPtrPtr](int32_t strOffset) {
			logError((*thisPtrPtr)->wasmToString(strOffset), "WasmProceduralCircuit > WasmCode");
		});

	// Linker to associate "env" functions
	wasmtime::Linker linker(*Wasm::getEngine());
	wasmtime::Result<std::__1::monostate> linkerResult = wasmtime::Result<std::__1::monostate>(nullptr);
	linkerResult = linker.define(*Wasm::getStore(), "env", "getParameter", getParameterFunc);
	if (!linkerResult) {
		logError("could not create link to env.getParameter()", "WasmProceduralCircuit::WasmInstance");
		return;
	}
	linkerResult = linker.define(*Wasm::getStore(), "env", "tryInsertBlock", tryInsertBlockFunc);
	if (!linkerResult) {
		logError("could not create link to env.tryInsertBlock()", "WasmProceduralCircuit::WasmInstance");
		return;
	}
	linkerResult = linker.define(*Wasm::getStore(), "env", "tryCreateConnection", tryCreateConnectionFunc);
	if (!linkerResult) {
		logError("could not create link to env.tryCreateConnection()", "WasmProceduralCircuit::WasmInstance");
		return;
	}
	linkerResult = linker.define(*Wasm::getStore(), "env", "addConnectionInput", addConnectionInputFunc);
	if (!linkerResult) {
		logError("could not create link to env.addConnectionInput()", "WasmProceduralCircuit::WasmInstance");
		return;
	}
	linkerResult = linker.define(*Wasm::getStore(), "env", "addConnectionOutput", addConnectionOutputFunc);
	if (!linkerResult) {
		logError("could not create link to env.addConnectionOutput()", "WasmProceduralCircuit::WasmInstance");
		return;
	}
	linkerResult = linker.define(*Wasm::getStore(), "env", "setSize", setSizeFunc);
	if (!linkerResult) {
		logError("could not create link to env.setSize()", "WasmProceduralCircuit::WasmInstance");
		return;
	}
	linkerResult = linker.define(*Wasm::getStore(), "env", "logInfo", logInfoFunc);
	if (!linkerResult) {
		logError("could not create link to env.logInfo()", "WasmProceduralCircuit::WasmInstance");
		return;
	}
	linkerResult = linker.define(*Wasm::getStore(), "env", "logError", logErrorFunc);
	if (!linkerResult) {
		logError("could not create link to env.logError()", "WasmProceduralCircuit::WasmInstance");
		return;
	}

	// Instantiate the module
	auto instanceResult = linker.instantiate(*Wasm::getStore(), module);
	if (!instanceResult) {
		logError("Failed to instantiate WASM module: {}", "WasmProceduralCircuit::WasmInstance", instanceResult.err().message());
		return;
	}
	instance.emplace(std::move(instanceResult.unwrap()));
	std::optional<wasmtime::Extern> memoryExport = instance.value().get(*Wasm::getStore(), "memory");
	if (!memoryExport) {
		logError("Failed to get WASM memory.", "WasmProceduralCircuit::WasmInstance");
		return;
	}
	memory.emplace(std::get<wasmtime::Memory>(memoryExport.value()));

	std::optional<wasmtime::Extern> uuidExtern(instance.value().get(*Wasm::getStore(), "getUUID"));
	if (!uuidExtern) {
		logError("Failed to get getUUID function.", "WasmProceduralCircuit::WasmInstance");
		return;
	}
	auto uuidFunc = std::get<wasmtime::Func>(uuidExtern.value());
	auto uuidResults = uuidFunc.call(*Wasm::getStore(), {}).unwrap();
	UUID = wasmToString(uuidResults.front().i32());

	std::optional<wasmtime::Extern> nameExtern(instance.value().get(*Wasm::getStore(), "getName"));
	if (!nameExtern) {
		logError("Failed to get getName function.", "WasmProceduralCircuit::WasmInstance");
		return;
	}
	auto nameFunc = std::get<wasmtime::Func>(nameExtern.value());
	auto nameResults = nameFunc.call(*Wasm::getStore(), {}).unwrap();
	name = wasmToString(nameResults.front().i32());

	valid = true;

	std::optional<wasmtime::Extern> defaultParametersExtern(instance.value().get(*Wasm::getStore(), "getDefaultParameters"));
	if (!defaultParametersExtern) {
		return; // No default parameters is valid!
	}
	auto defaultParametersFunc = std::get<wasmtime::Func>(defaultParametersExtern.value());
	auto defaultParametersResults = defaultParametersFunc.call(*Wasm::getStore(), {}).unwrap();
	std::stringstream ss(wasmToString(defaultParametersResults.front().i32()));
	defaultParameters = ProceduralCircuitParameters(ss);
}

WasmProceduralCircuit::WasmInstance::WasmInstance(WasmInstance&& wasmInstance) :
	instance(std::move(wasmInstance.instance.value())),
	memory(std::move(wasmInstance.memory.value())),
	name(std::move(wasmInstance.name)),
	UUID(std::move(wasmInstance.UUID)),
	defaultParameters(std::move(wasmInstance.defaultParameters)),
	thisPtr(std::move(wasmInstance.thisPtr)) {
	*thisPtr = this;
}

WasmProceduralCircuit::WasmInstance& WasmProceduralCircuit::WasmInstance::operator=(WasmInstance&& wasmInstance) {
	if (this != &wasmInstance) {
		instance = std::move(wasmInstance.instance.value());
		memory = std::move(wasmInstance.memory.value());
		name = std::move(wasmInstance.name);
		UUID = std::move(wasmInstance.UUID);
		defaultParameters = std::move(wasmInstance.defaultParameters);
		thisPtr = std::move(wasmInstance.thisPtr);
		*thisPtr = this;
	}
	return *this;
}

void WasmProceduralCircuit::WasmInstance::makeCircuit(const ProceduralCircuitParameters& parameters, SharedCircuit circuit, BlockData* blockData, CircuitBlockData* circuitBlockData) {
	if (!instance.has_value()) return;

	this->parameters = &parameters;
	this->circuit = circuit;
	this->blockData = blockData;
	this->circuitBlockData = circuitBlockData;
	portId = 0;

	auto func = std::get<wasmtime::Func>(instance.value().get(*Wasm::getStore(), "generateCircuit").value());
	auto results = func.call(*Wasm::getStore(), {}).unwrap();
	bool success = results.front().i32();

	this->parameters = nullptr;
	this->circuit = nullptr;
	this->blockData = nullptr;
	this->circuitBlockData = nullptr;
}

std::string WasmProceduralCircuit::WasmInstance::wasmToString(int32_t wasmPtr) {
	auto memSpan = memory.value().data(*Wasm::getStore());
	const char* str = (const char*)(memSpan.data() + wasmPtr);
	return std::string(str, strnlen(str, memSpan.size() - wasmPtr));
}

WasmProceduralCircuit::WasmProceduralCircuit(
	CircuitManager* circuitManager,
	DataUpdateEventManager* dataUpdateEventManager,
	WasmInstance&& wasmInstance
) : ProceduralCircuit(circuitManager, dataUpdateEventManager, wasmInstance.getName(), wasmInstance.getUUID()), wasmInstance(std::move(wasmInstance)) {
	setParameterDefaults(this->wasmInstance.getDefaultParameters());
}

WasmProceduralCircuit::WasmProceduralCircuit(WasmProceduralCircuit&& other) : ProceduralCircuit(static_cast<ProceduralCircuit&&>(other)), wasmInstance(std::move(other.wasmInstance)) { }

void WasmProceduralCircuit::setWasm(WasmInstance&& wasmInstance) {
	if (!wasmInstance.isValid()) {
		logError("setWasm() Failed! WasmInstance was not valid.", "WasmProceduralCircuit");
		return;
	}
	if (wasmInstance.getUUID() != this->wasmInstance.getUUID()) {
		logError(
			"setWasm() Failed! Tried to update the uuid of a WasmProceduralCircuit. DO NOT DO THIS. Old UUID: {}. New UUID: {}",
			 "WasmProceduralCircuit", this->wasmInstance.getUUID(), wasmInstance.getUUID()
		);
		return;
	}
	setProceduralCircuitName(wasmInstance.getName());
	setParameterDefaults(wasmInstance.getDefaultParameters());

	this->wasmInstance = std::move(wasmInstance);

	regenerateAll();
}

void WasmProceduralCircuit::makeCircuit(const ProceduralCircuitParameters& parameters, SharedCircuit circuit, BlockData* blockData, CircuitBlockData* circuitBlockData) {
	wasmInstance.makeCircuit(parameters, circuit, blockData, circuitBlockData);
}
