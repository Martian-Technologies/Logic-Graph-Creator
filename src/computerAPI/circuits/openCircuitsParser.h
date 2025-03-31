#ifndef openCircuitsParser_h
#define openCircuitsParser_h

#include <nlohmann/json.hpp>
#include <list>

#include "backend/circuit/parsedCircuit.h"
#include "backend/position/position.h"
#include "computerAPI/circuits/parsedCircuitLoader.h"

using json = nlohmann::json;

struct OpenCircuitsBlockInfo {
	std::string type;
	FPosition position;
	double angle; // in radians
	std::vector<int> inputBlocks; // reference ids to other blocks/circuit nodes
	std::vector<int> outputBlocks;
	int icReference; // if it is an IC block
};

struct ICData {
	std::unordered_map<int, OpenCircuitsBlockInfo> components;
	std::vector<block_id_t> inputPorts;
	std::vector<block_id_t> outputPorts;
};

class OpenCircuitsParser : public ParsedCircuitLoader {
public:
	OpenCircuitsParser(CircuitManager* cm) : ParsedCircuitLoader(cm) { }
	bool parse(const std::string& path, SharedParsedCircuit outParsedCircuit);
	void parseOpenCircuitsJson();

	// Driver functions for processing the json associated to "id" within the "contents" of the save file
	void processBlockJson(int id, OpenCircuitsBlockInfo& info);
	void processICDataJson(int id, ICData& icData);

	// Stores the information such as angle, position, etc for blocks
	void parseTransform(const json& transform, OpenCircuitsBlockInfo& info);

	// Finds the connections between blocks that are described via the "wires"
	void processOpenCircuitsPorts(const json& ports, bool isOutput, OpenCircuitsBlockInfo& info, int thisId);

	// Filters and resolves across all blocks, even within the components of ICData's
	void filterAndResolveBlocks(std::unordered_map<int, OpenCircuitsBlockInfo*>& outFiltered);
	void resolveInputsAndOutputs(OpenCircuitsBlockInfo* b, std::unordered_map<int, OpenCircuitsBlockInfo*>& allBlocks);
	void resolveOpenCircuitsConnections(bool input, int startId, std::unordered_map<int, OpenCircuitsBlockInfo*>& allBlocks, std::vector<int>& orderedConnectionBlocks);

	void fillParsedCircuit(const std::unordered_map<int, OpenCircuitsBlockInfo*>& filteredBlocks);
	void printParsedData();

private:
	json contents;
	int newReferenceID = -1;
	std::unordered_map<int, OpenCircuitsBlockInfo> blocks;           // id to block data (WITHIN PRIMARY CIRCUIT ONLY)
	std::unordered_map<int, std::pair<FPosition, double>> transforms; // id to block position and angle in radians
	std::unordered_map<int, int> portParents;                        // ref id to ref id
	std::unordered_map<int, std::pair<int, int>> wires;               // id to ref id pair


	// Note that getting these lists of references is faster than adding them while iterating
	// through the entire json, because they are explicitly listed out in a specific json field.
	std::list<int> componentReferences; // used to gather the ids before inserting into "blocks"
	std::list<int> ICDataReferences; // used before actually processing ICData structures

	std::unordered_map<int, ICData> icDataMap; // the components of each icData aren't filtered to valid types
	SharedParsedCircuit outParsed;

	// every time an IC instance is added, the data it references will go here, so we can keep track of the important icDatas
	std::unordered_set<int> usedIcDatas;

	std::unordered_map<std::string, BlockType> openCircuitsTypeToName = {
		{"ANDGate", BlockType::AND}, {"ORGate", BlockType::OR}, {"XORGate", BlockType::XOR}, {"NANDGate", BlockType::NAND}, {"NORGate", BlockType::NOR},
		{"XNORGate", BlockType::XNOR}, {"BUFGate", BlockType::JUNCTION}, {"Switch", BlockType::SWITCH}, {"Button", BlockType::BUTTON},
		{"Clock", BlockType::TICK_BUTTON}, {"LED", BlockType::LIGHT},
		{"NOTGate", BlockType::NOR}, {"IC", BlockType::CUSTOM},
	};
	const double posScale = 0.02;
};

#endif
