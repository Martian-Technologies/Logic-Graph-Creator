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
    // For normal gates this is the ID of the gate in the simulator (AND / NOR / TRISTATE_BUFFER, etc.)
    // For gates that refernece ICs, this is the ID for more context data for that position relative to the IC.
    // V_CUSTOM will the the top-left corner of the IC, and will be used as a reference point for the IC. This may act as an I/O port.
    // R_IO will be used in places where the IC has I/O ports. that are not at the top-left corner.
    NodeType type;
};

#endif /* circuitNode_h */