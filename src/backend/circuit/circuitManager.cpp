#include "circuitManager.h"

#include "backend/evaluator/evaluatorManager.h"

circuit_id_t CircuitManager::createNewCircuit(const std::string& name, const std::string& uuid) {
	circuit_id_t id = getNewCircuitId();
	const SharedCircuit circuit = std::make_shared<Circuit>(id, &blockDataManager, dataUpdateEventManager, name, uuid);
	circuits.emplace(id, circuit);
	UUIDToCircuits.emplace(uuid, circuit);
	for (auto& [object, func] : listenerFunctions) {
		circuit->connectListener(object, func);
	}

	setupBlockData(id);

	auto evaluatorId = evaluatorManager->createNewEvaluator(*this, id);
	SharedEvaluator eval = evaluatorManager->getEvaluator(evaluatorId);
	eval->setPause(false);
	eval->setUseTickrate(true);
	eval->setTickrate(2400);

	return id;
}

CircuitManager::CircuitManager(DataUpdateEventManager* dataUpdateEventManager, EvaluatorManager* evaluatorManager) :
	blockDataManager(dataUpdateEventManager), circuitBlockDataManager(dataUpdateEventManager), proceduralCircuitManager(this, dataUpdateEventManager),
	dataUpdateEventManager(dataUpdateEventManager), dataUpdateEventReceiver(dataUpdateEventManager), evaluatorManager(evaluatorManager) {
	dataUpdateEventReceiver.linkFunction("postBlockSizeChange", [this](const DataUpdateEventManager::EventData* eventData) {
		linkedFunctionForUpdates<Vector>(eventData);
	});
	dataUpdateEventReceiver.linkFunction("blockDataRemoveConnection", [this](const DataUpdateEventManager::EventData* eventData) {
		linkedFunctionForUpdates<connection_end_id_t>(eventData);
	});
	dataUpdateEventReceiver.linkFunction("blockDataSetConnection", [this](const DataUpdateEventManager::EventData* eventData) {
		linkedFunctionForUpdates<connection_end_id_t>(eventData);
	});
	dataUpdateEventReceiver.linkFunction("blockDataConnectionNameSet", [this](const DataUpdateEventManager::EventData* eventData) {
		linkedFunctionForUpdates<Vector>(eventData);
	});
}

