#include "verilogYosysParser.h"

#include <kernel/yosys.h>

std::vector<circuit_id_t> VerilogYosysParser::load(const std::string& path) {
	Yosys::yosys_setup(); // ok to run again
	// Yosys::load_plugin("/Users/ben/Documents/GitHub/Logic-Graph-Creator/build/_deps/yosys-slang-build/slang.so", {});
	// Yosys::loaded_plugins.begin()->second->

	logInfo("Yosys plugins:", "VerilogYosysParser");
	for (auto& pair : Yosys::loaded_plugins) {
		logInfo("{}", "VerilogYosysParser", pair.first);
	}

	logInfo("Yosys frontend:", "VerilogYosysParser");
	for (auto& pair : Yosys::frontend_register) {
		logInfo("{}", "VerilogYosysParser", pair.first);
	}

    // Create an empty design
    std::unique_ptr<Yosys::RTLIL::Design> design = std::make_unique<Yosys::RTLIL::Design>();
	
    // Import Verilog file
    Yosys::Frontend::frontend_call(design.get(), nullptr, path, "slang");

    // // Iterate over all modules
    for (Yosys::RTLIL::Module* module : design->modules()) {
        std::cout << "Module: " << module->name.str() << "\n";

        // Iterate over all cells (gates)
        for (Yosys::RTLIL::Cell* cell : module->cells()) {
            std::cout << "  Cell: " << cell->name.str() << ", type: " << cell->type.str() << "\n";
        }
    }

	return {};
}
