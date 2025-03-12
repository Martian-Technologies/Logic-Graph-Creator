#ifndef openCircuitsParser_h
#define openCircuitsParser_h

#include <QtCore/QJsonObject>
#include <list>

#include "backend/circuit/parsedCircuit.h"
#include "backend/position/position.h"

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
    std::vector<int> inputPorts;
    std::vector<int> outputPorts;
};

class OpenCircuitsParser {
public:
    OpenCircuitsParser() = default;
    bool parse(const std::string& path, SharedParsedCircuit outParsedCircuit);
    void parseOpenCircuitsJson();

    // Driver functions for processing the json associated to "id" within the "contents" of the save file
    void processBlockJson(int id, OpenCircuitsBlockInfo& info);
    void processICDataJson(int id, ICData& icData);

    // Stores the information such as angle, position, etc for blocks
    void parseTransform(const QJsonObject& transform, OpenCircuitsBlockInfo& info);

    // Finds the connections between blocks that are described via the "wires"
    void processOpenCircuitsPorts(const QJsonObject& ports, bool isOutput, OpenCircuitsBlockInfo& info, int thisId);

    // Filters and resolves across all blocks, even within the components of ICData's
    void filterAndResolveBlocks(std::unordered_map<int,OpenCircuitsBlockInfo*>& outFiltered);
    void resolveInputsAndOutputs(OpenCircuitsBlockInfo* b, std::unordered_map<int,OpenCircuitsBlockInfo*>& allBlocks);
    void resolveOpenCircuitsConnections(bool input, int startId, std::unordered_map<int, OpenCircuitsBlockInfo*>& allBlocks, std::unordered_set<int>& outResolvedConnectionBlocks);

    void fillParsedCircuit(const std::unordered_map<int,OpenCircuitsBlockInfo*>& filteredBlocks);
    void printParsedData();

private:
    QJsonObject contents;
    int newReferenceID = -1;
    std::unordered_map<int,OpenCircuitsBlockInfo> blocks;           // id to block data (WITHIN PRIMARY CIRCUIT ONLY)
    std::unordered_map<int,std::pair<FPosition,double>> transforms; // id to block position and angle in radians
    std::unordered_map<int,int> portParents;                        // ref id to ref id
    std::unordered_map<int,std::pair<int,int>> wires;               // id to ref id pair


    // Note that getting these lists of references is faster than adding them while iterating
    // through the entire json, because they are explicitly listed out in a specific json field.
    std::list<int> componentReferences; // used to gather the ids before inserting into "blocks"
    std::list<int> ICDataReferences; // used before actually processing ICData structures

    std::unordered_map<int, ICData> icDataMap; // the components of each icData aren't filtered to valid types
    SharedParsedCircuit outParsed;

    // every time an IC instance is added, the data it references will go here, so we can keep track of the important icDatas
    std::unordered_set<int> usedIcDatas;

    std::unordered_set<std::string> validOpenCircuitsTypes =
        {"ANDGate", "ORGate", "XORGate", "NANDGate", "NORGate", "XNORGate",
        "BUFGate", "Switch", "Button", "Clock", "LED", "NOTGate", "IC"};
    std::unordered_map<std::string, std::string> openCircuitsTypeToName = {
        {"ANDGate", "AND"}, {"ORGate", "OR"}, {"XORGate", "XOR"}, {"NANDGate", "NAND"}, {"NORGate", "NOR"},
        {"XNORGate", "XNOR"}, {"BUFGate", "BUFFER"}, {"Switch", "SWITCH"}, {"Button", "BUTTON"}, 
        {"Clock", "TICK_BUTTON"}, {"LED", "LIGHT"},
        {"NOTGate", "NOR"}, // NOR for not
        {"IC", "BUFFER"}, // IC will be buffer for now, until custom blocks
    };
    const double posScale = 0.02;
};

#endif
