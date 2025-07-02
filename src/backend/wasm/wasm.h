#ifndef wasm_h
#define wasm_h

#include <wasmtime.hh>

class Wasm {
public:
    // Static interface
    static bool initialize();
    static wasmtime::Engine* getEngine();
    static wasmtime::Store* getStore();
    static wasmtime::Linker* getLinker();
    static std::optional<wasmtime::Module> loadModule(const std::string& path);
	static std::optional<wasmtime::Module> loadModuleFromString(const std::string& wasmOrWat);

private:
    // Static variables holding Wasmtime context
    static std::optional<wasmtime::Engine> engine;
    static std::optional<wasmtime::Store> store;
    static std::optional<wasmtime::Linker> linker;

    // Disable construction
    Wasm() = delete;
};

#endif /* wasm_h */
