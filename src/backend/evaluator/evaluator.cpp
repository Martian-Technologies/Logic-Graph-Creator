#include "evaluator.h"

Evaluator::Evaluator(evaluator_id_t evaluatorId, CircuitManager& circuitManager, circuit_id_t circuitId, DataUpdateEventManager* dataUpdateEventManager)
	: evaluatorId(evaluatorId), paused(true),
	targetTickrate(0),
	usingTickrate(false),
	circuitManager(circuitManager),
	circuitLattice(),
	receiver(dataUpdateEventManager) {
	setTickrate(40 * 60);
	const auto circuit = circuitManager.getCircuit(circuitId);
	circuitIds.push_back(circuitId);
	const auto blockContainer = circuit->getBlockContainer();
	const Difference difference = blockContainer->getCreationDifference();
	// receiver.linkFunction("blockDataRemoveConnection", std::bind(&Evaluator::removeCircuitIO, this, std::placeholders::_1));

	makeEdit(std::make_shared<Difference>(difference), circuitId);
}
