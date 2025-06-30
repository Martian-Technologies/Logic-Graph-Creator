#ifndef circuitNode_h
#define circuitNode_h

#include "evalGate.h"

class CircuitNode {
public:
	static CircuitNode fromMiddle(middle_id_t id) {
		return CircuitNode(id << 1);
	}
	static CircuitNode fromIC(unsigned int id) {
		return CircuitNode((id << 1) | 1);
	}
	bool isIC() const { return (id_and_type & 1) == 1; }
	unsigned int getId() const { return id_and_type >> 1; }
private:
	explicit CircuitNode(unsigned int value) : id_and_type(value) {}
	unsigned int id_and_type;
};

#endif // circuitNode_h