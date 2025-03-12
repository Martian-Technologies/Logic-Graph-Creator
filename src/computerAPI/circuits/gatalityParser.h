#ifndef gatalityParser_h
#define gatalityParser_h

#include "backend/circuit/circuit.h"
#include "backend/circuit/parsedCircuit.h"

class GatalityParser {
public:
    GatalityParser() = default;
    bool load(const std::string& path, SharedParsedCircuit outParsedCircuit);
    bool save(const std::string& path, Circuit* circuitPtr);
};

#endif
