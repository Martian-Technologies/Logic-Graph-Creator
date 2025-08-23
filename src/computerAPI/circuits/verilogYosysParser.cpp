#include "verilogYosysParser.h"

#include <kernel/yosys.h>
// #include <slang/driver/Driver.h>

std::vector<circuit_id_t> VerilogYosysParser::load(const std::string& path) {
	logInfo("Import Verilog file from: {}", "VerilogYosysParser", path);

	// slang_frontend
	// slang::driver::Driver driver;
	// driver.addStandardArgs();

	// // create args for slang
	// std::vector<std::unique_ptr<char[]>> c_args_guard;
	// std::vector<char*> c_args;

	// char* c = new char[6];
	// strcpy(c, "slang");
	// c_args_guard.emplace_back(c);
	// c_args.push_back(c);

	// c = new char[path.size() + 1];
	// strcpy(c, path.c_str());
	// c_args_guard.emplace_back(c);
	// c_args.push_back(c);

	// if (!driver.parseCommandLine(c_args.size(), &c_args[0])) logError("Bad slang command", "VerilogYosysParser");
	// if (!driver.parseAllSources()) logError("Slang parsing failed\n", "VerilogYosysParser");
	// for (const std::filesystem::path& depfile : driver.getDepfiles()) {
	// 	std::filesystem::path fullPath = std::filesystem::weakly_canonical(std::filesystem::absolute(depfile));
	// 	if (std::filesystem::weakly_canonical(std::filesystem::absolute(std::filesystem::path(path))).compare(fullPath) != 0) {
	// 		logInfo("using: {}", "VerilogYosysParser", fullPath.string());
	// 		// circuitFileManager->loadFromFile(fullPath);
	// 	}
	// }
	// The include does not include needed verilog and so does not help us :(

	Yosys::yosys_setup(); // ok to run again

	// Create an empty design
	Yosys::RTLIL::Design* design = new Yosys::RTLIL::Design();

	// Import Verilog file
	try {
		Yosys::Frontend::frontend_call(design, nullptr, path, "slang");

		Yosys::Pass::call(design, "hierarchy -check -top top");

		// // Iterate over all modules
		for (Yosys::RTLIL::Module* module : design->modules()) {
			std::cout << "Module: " << module->name.str() << "\n";

			// Iterate over all cells (gates)
			for (Yosys::RTLIL::Cell* cell : module->cells()) {
				std::cout << "  Cell: " << cell->name.str() << ", type: " << cell->type.str() << "\n";
			}
		}
	} catch (...) {
		// Yosys error handler
		logError("Yosys command error.");
	}

	return {};
}
