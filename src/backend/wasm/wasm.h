#ifndef wasm_h
#define wasm_h

#include <wasmtime.hh>

class Wasm {
public:
	// Can happen more than once. NEEDS to happen at lease once
    static bool initialize();
    static wasmtime::Engine* getEngine();
    static wasmtime::Store* getStore();
    static std::optional<wasmtime::Module> loadModule(const std::string& path);
	static std::optional<wasmtime::Module> loadModuleFromString(const std::string& wasmOrWat);

private:
    static std::optional<wasmtime::Engine> engine;
    static std::optional<wasmtime::Store> store;

    Wasm() = delete;
};

#endif /* wasm_h */
