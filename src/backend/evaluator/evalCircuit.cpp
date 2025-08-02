#include "evalCircuit.h"

std::optional<CircuitNode> EvalCircuit::getNode(Position pos) const noexcept {
	const CircuitNode* node = circuitNodes.get(pos);
	if (node) {
		return *node;
	}
	return std::nullopt;
}
