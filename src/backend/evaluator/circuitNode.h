#ifndef circuitNode_h
#define circuitNode_h

enum class NodeType : char {
    AND,
    OR,
    XOR,
    NAND,
    NOR,
    XNOR,
    RETAIN,
    ONETICK,
    DUMMY,
    CONST_ON,
    JUNCTION,
    P_TRISTATE_BUFFER,
    P_TRISTATE_BUFFER_INVERTED,
    V_CUSTOM,
    R_IO,
};

struct CircuitNode {
    int id; // semi-unique ID for the node. IDs are not unique across real/parent and virtual/refernece gates.
    NodeType type;
};

#endif /* circuitNode_h */