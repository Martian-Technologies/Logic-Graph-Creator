#include "circuitFileManager.h"
#include "gatalityParser.h"
#include "openCircuitsParser.h"

CircuitFileManager::CircuitFileManager(const CircuitManager* circuitManager) : circuitManager(circuitManager) {}

BlockType stringToBlockType(const std::string& str) {
    if (str == "NONE") return BlockType::NONE;
    if (str == "AND") return BlockType::AND;
    if (str == "OR") return BlockType::OR;
    if (str == "XOR") return BlockType::XOR;
    if (str == "NAND") return BlockType::NAND;
    if (str == "NOR") return BlockType::NOR;
    if (str == "XNOR") return BlockType::XNOR;
    if (str == "BUFFER") return BlockType::BUFFER;
    if (str == "TRISTATE_BUFFER") return BlockType::TRISTATE_BUFFER;
    if (str == "BUTTON") return BlockType::BUTTON;
    if (str == "TICK_BUTTON") return BlockType::TICK_BUTTON;
    if (str == "SWITCH") return BlockType::SWITCH;
    if (str == "CONSTANT") return BlockType::CONSTANT;
    if (str == "LIGHT") return BlockType::LIGHT;
    if (str == "CUSTOM") return BlockType::CUSTOM;
    return BlockType::NONE;
}

Rotation stringToRotation(const std::string& str) {
    if (str == "ZERO") return Rotation::ZERO;
    if (str == "NINETY") return Rotation::NINETY;
    if (str == "ONE_EIGHTY") return Rotation::ONE_EIGHTY;
    if (str == "TWO_SEVENTY") return Rotation::TWO_SEVENTY;
    return Rotation::ZERO;
}

std::string blockTypeToString(BlockType type) {
    switch (type) {
        case BlockType::NONE: return "NONE";
        case BlockType::AND: return "AND";
        case BlockType::OR: return "OR";
        case BlockType::XOR: return "XOR";
        case BlockType::NAND: return "NAND";
        case BlockType::NOR: return "NOR";
        case BlockType::XNOR: return "XNOR";
        case BlockType::BUFFER: return "BUFFER";
        case BlockType::TRISTATE_BUFFER: return "TRISTATE_BUFFER";
        case BlockType::BUTTON: return "BUTTON";
        case BlockType::TICK_BUTTON: return "TICK_BUTTON";
        case BlockType::SWITCH: return "SWITCH";
        case BlockType::CONSTANT: return "CONSTANT";
        case BlockType::LIGHT: return "LIGHT";
        case BlockType::CUSTOM: return "CUSTOM";
        default: return "NONE";
    }
}

std::string rotationToString(Rotation rotation) {
    switch (rotation) {
        case Rotation::ZERO: return "ZERO";
        case Rotation::NINETY: return "NINETY";
        case Rotation::ONE_EIGHTY: return "ONE_EIGHTY";
        case Rotation::TWO_SEVENTY: return "TWO_SEVENTY";
        default: return "ZERO";
    }
}

bool CircuitFileManager::loadFromFile(const std::string& path, SharedParsedCircuit outParsed) {
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".cir") {
        // our gatality file parser function
        GatalityParser parser;
        return parser.load(path, outParsed);
    } else if (path.size() >= 8 && path.substr(path.size() - 8) == ".circuit") {
        // open circuit file parser function
        OpenCircuitsParser parser;
        return parser.parse(path, outParsed);
    }else {
        logError("Unsupported file extension. Expected .circuit or .cir", "FileManager");
    }
    return false;
}

bool CircuitFileManager::saveToFile(const std::string& path, Circuit* circuitPtr) {
    GatalityParser saver;
    return saver.save(path, circuitPtr);
}
