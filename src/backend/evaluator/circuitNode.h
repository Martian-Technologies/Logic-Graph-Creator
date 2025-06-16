#ifndef circuitNode_h
#define circuitNode_h

struct CircuitNode {
    bool simulatorReference; // true if this node references a real gate in the simulator, false if it is a virtual node that references an IC or acts as an input/output node for a multi-cell gate
    int id; // semi-unique ID for the node. IDs are not unique across simulatorReference true/false, but are unique within each type
    NodeType type;
}

#endif /* circuitNode_h */