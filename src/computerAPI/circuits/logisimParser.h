#ifndef logisimParser_h
#define logisimParser_h

/* parses LOGISIM's .circ files */
/* ONLY SUPPORTS IMPORT RIGHT NOW, NO EXPORTING AVAILABLE */

#include "backend/circuit/circuit.h"
#include "backend/circuit/parsedCircuit.h"
#include "backend/container/block/block.h"
#include "backend/container/block/blockDefs.h"
#include "backend/position/position.h"
#include "computerAPI/circuits/parsedCircuitLoader.h"
#include <unordered_map>

// this is for the visual appearance of the circuit
struct XMLAppearance {

};

// each gate
struct XMLComponent {
/*  '*' represents implemented, others have to get implemented at some point
Format: "<comp lib="0" loc="(60,300)" name="Pin">"
	* lib="0": Wiring Library
		- non-logic components
			- pins, wires, power and ground, clocks, tunnels, pull resistors
	* lib="1": Gate Library
		- basic logic gates
			-
			-
			- 
	lib="2": Plexer Library
		- selecitng and routing signals
			- multiplexers, demultiplexers, decoders, encoders, bit extenders
	lib="3": Arithmetic Library
		- components for performing arithmetic operations
			- adders, subtractors, multipliers, dividers, comparators, shifters

	lib="4": Memory Library
		- memory elements
			- RAM, ROM, Registers, Counters

	lib="5": Input/Output Library
		- graphical outputs for users
			- buttons, switches, leds, hex digit displays, tty(not going to suppor this most likely)

	lib="6": Base Library
		- Fundamental and Structural Elements
			- text labels, closed boxes, splitters

	lib="7": User Custom Libraries 
		- defined by user, incrementing form 7+i for each additional library added
*/

	uint8_t lib; // not required but could be helpful 
	// all values where | <a name="*" | appears
	std::string label; 
};

// wires a - b, should cut otu intermediates
// TODO: fully reduce excess wires
struct XMLWire {
	int x1, y1;
	int x2, y2;
};

class LogisimParser : public ParsedCircuitLoader {
public:
    LogisimParser(CircuitFileManager* circuitFileManager, CircuitManager* circuitManager) : ParsedCircuitLoader(circuitFileManager, circuitManager) {}

	std::vector<circuit_id_t> load(const std::string& path) override;
    bool save(const CircuitFileManager::FileData& fileData, bool compress);

private:
	void loadCircuit(const std::vector<std::string>& circuitChunk); // loads xml "<circuit name=...>"
	void loadAppearance(); // loads xml <appear> block
	void loadWire(); // loads xml "<wire from=... to=... />"
	bool loadComp(const std::vector<std::string>& componentChunk, BlockType& blockType, Rotation& rotation); // loads xml "<comp from=... to=...> ... </comp>"

	std::unordered_set<std::string> importedFiles;	
	std::unordered_map<std::string, BlockType> blockReference = {
		{ "Pin", BlockType::SWITCH },   // TODO: custom block should have this changed to chip in
		{ "PinOut", BlockType::LIGHT }, // TODO: this is chip out
		{ "AND Gate", BlockType::AND },
		{ "OR Gate", BlockType::OR },
		{ "NOT Gate", BlockType::NAND },
		{ "NAND Gate", BlockType::NAND }
	};
};
#endif /* logisimParser_h */
