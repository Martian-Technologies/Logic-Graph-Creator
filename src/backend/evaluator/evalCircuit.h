#ifndef evalCircuit_h
#define evalCircuit_h

#include "backend/circuit/circuit.h"
#include "backend/position/sparse2d.h"
#include "circuitNode.h"
#include "evalTypedef.h"

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
	void setNode(Position pos, CircuitNode node) {
		circuitNodes.insert(pos, node);
	}
	void removeNode(Position pos) {
		circuitNodes.remove(pos);
	}
	void moveNode(Position oldPos, Position newPos) {
		std::optional<CircuitNode> node = getNode(oldPos);
		if (node) {
			circuitNodes.remove(oldPos);
			circuitNodes.insert(newPos, node.value());
		} else {
			logError("Node at position {} not found", "EvalCircuit::moveNode", oldPos.toString());
		}
	}
	template<typename F>
	void forEachNode(F&& func) const {
		circuitNodes.forEach([&func](Position pos, const CircuitNode& node) {
			func(pos, node);
		});
	}
private:
	circuit_id_t circuitId;
	Sparse2dArray<CircuitNode> circuitNodes;
};

#endif // evalCircuit_h