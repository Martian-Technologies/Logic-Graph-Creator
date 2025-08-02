#ifndef evalCircuitContainer_h
#define evalCircuitContainer_h

#include "backend/position/position.h"
#include "circuitNode.h"
#include "evalCircuit.h"
#include "idProvider.h"
#include "backend/address.h"

struct EvalPosition {
	Position position;
	eval_circuit_id_t evalCircuitId;

	inline EvalPosition(Position position, eval_circuit_id_t evalCircuitId)
		: position(position), evalCircuitId(evalCircuitId) {}
};

class EvalCircuitContainer {

public:
	EvalCircuitContainer() = default;
	EvalCircuitContainer(const EvalCircuitContainer&) = delete;
	EvalCircuitContainer& operator=(const EvalCircuitContainer&) = delete;
	eval_circuit_id_t addCircuit(eval_circuit_id_t parentEvalId, circuit_id_t circuitId);
	void removeCircuit(eval_circuit_id_t evalCircuitId);
	std::optional<CircuitNode> getNode(EvalPosition pos) const noexcept;
	std::optional<CircuitNode> getNode(Position pos, eval_circuit_id_t evalCircuitId) const noexcept;
	EvalCircuit* getCircuit(eval_circuit_id_t evalCircuitId) const noexcept;

	inline size_t size() const noexcept {
		return circuits.size();
	}
	inline bool empty() const noexcept {
		return circuits.empty();
	}

	std::optional<CircuitNode> traverse(const Address& address) const;
	std::optional<CircuitNode> traverse(const eval_circuit_id_t startingPoint, const Address& address) const;
	eval_circuit_id_t traverseToTopLevelIC(const Address& address) const;
	eval_circuit_id_t traverseToTopLevelIC(const eval_circuit_id_t startingPoint, const Address& address) const;

	std::optional<eval_circuit_id_t> getCircuitId(eval_circuit_id_t evalCircuitId) const noexcept;

private:
	std::vector<EvalCircuit*> circuits;
	IdProvider<eval_circuit_id_t> evalCircuitIdProvider;
};

#endif // evalCircuitContainer_h