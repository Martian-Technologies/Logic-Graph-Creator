#ifndef evalCircuit_h
#define evalCircuit_h

#include "backend/circuit/circuit.h"
#include "backend/position/sparse2d.h"
#include "circuitNode.h"
#include "evalGate.h"

class EvalCircuit {
public:
	EvalCircuit(circuit_id_t circuitId)
		: circuitId(circuitId) {};
	EvalCircuit(const EvalCircuit&) = delete;
	EvalCircuit& operator=(const EvalCircuit&) = delete;
	EvalCircuit(EvalCircuit&&) = default;
	EvalCircuit& operator=(EvalCircuit&&) = default;
	std::optional<CircuitNode> getNode(Position pos) const noexcept;
	circuit_id_t getCircuitId() const noexcept {
		return circuitId;
	}
private:
	circuit_id_t circuitId;
	Sparse2dArray<CircuitNode> circuitNodes;
};

#endif // evalCircuit_h