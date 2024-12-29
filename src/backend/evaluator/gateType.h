#ifndef gateType_h
#define gateType_h

enum class GateType {
    NONE = 0,
    AND = 1,
    OR = 2,
    XOR = 3,
    NAND = 4,
    NOR = 5,
    XNOR = 6,
    CONTINUOUS_INPUT = 7,
    TICK_INPUT = 8,
    CONSTANT_ON = 9,
};

#endif // gateType_h