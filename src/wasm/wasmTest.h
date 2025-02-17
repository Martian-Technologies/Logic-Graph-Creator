#ifndef wasmTest_h
#define wasmTest_h

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <cstdio>  // For popen, pclose, etc.
#include <memory> // Unique pointer
#include <stdexcept> // Exceptions.

//Requires pre-installing wasmtime!

#include <wasmtime.hh>


std::string executeCommandWithInput(const std::string& command, const std::string& input) {
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "w+"), pclose); // Use "w+" to allow both writing and reading.
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }

    // Write input to the pipe.
    if (fwrite(input.c_str(), 1, input.size(), pipe.get()) != input.size()) {
        throw std::runtime_error("fwrite() failed!");
    }
    fflush(pipe.get());  // Ensure all input is sent.

    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr) {
        result += buffer;
    }

    return result;
}

int run() {
    // 1. Define the WASM source code
    const std::string wasmSourceCode =
        "#include <stdio.h>\n"
        "extern \"C\" {\n"  // Use extern "C" for C++ to avoid name mangling
        "  int add(int a, int b) {\n"
        "    printf(\"Adding %d and %d from WebAssembly!\\n\", a, b);\n"
        "    return a + b;\n"
        "  }\n"
        "}";

    // 2. Emscripten Compilation (Piping the source code to emcc)
    std::string emccCommand = "emcc -x c++ - -s EXPORTED_FUNCTIONS=['_add'] -s SIDE_MODULE=1 -o a.wasm"; // Compile directly to .wasm
    // -x c++ tells emcc to treat the input stream as c++ code
    // -s SIDE_MODULE=1 is criical, telling emscripten to output just the wasm and not javascript

    std::string emccOutput;


    try {
        emccOutput = executeCommandWithInput(emccCommand, wasmSourceCode);
    } catch (const std::runtime_error& e) {
        std::cerr << "Emscripten compilation failed: " << e.what() << std::endl;
        return 1;

    }
    std::cout << "Emcc Output: " << emccOutput << std::endl; // Print emcc output for debugging.


    // 3. Load and Run with Wasmtime
    try {


        wasmtime::Engine engine;
        wasmtime::Module module = wasmtime::Module::from_file(engine, "a.wasm");
        wasmtime::Linker linker(engine);

        wasmtime::Store store(engine);
        wasmtime::Instance instance(store, module, linker);

        // Find the function.  Note:  The mangled name from emcc!
        auto add_func = instance.get_function(store, "_add");
        if (!add_func) {
            std::cerr << "Error:  Could not find function 'add' (mangled as '_add' by emcc)." << std::endl;
            return 1;
        }


        // Call the function with arguments
        wasmtime::Val resultVal;
        std::vector<wasmtime::Val> args = {wasmtime::Val(int32_t(10)), wasmtime::Val(int32_t(20))};
        add_func->call(store, args, {&resultVal});

        // Get the result
        int result = resultVal.i32();

        std::cout << "Result from WASM: " << result << std::endl;

         // Clean up the temporary WASM file
        std::remove("a.wasm");

    } catch (const wasmtime::Error& e) {
        std::cerr << "Wasmtime error: " << e.message() << std::endl;
        return 1;
    }


    return 0;
}

#endif /* wasmTest_h */