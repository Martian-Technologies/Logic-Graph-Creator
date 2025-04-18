#include "evaluator.h"

Evaluator::Evaluator(evaluator_id_t evaluatorId, CircuitManager& circuitManager, circuit_id_t circuitId, DataUpdateEventManager* dataUpdateEventManager)
	: evaluatorId(evaluatorId), paused(true),
	targetTickrate(0),
	addressTree(circuitId, Rotation::ZERO, dataUpdateEventManager),
	usingTickrate(false),
	circuitManager(circuitManager),
	receiver(dataUpdateEventManager) {
	setTickrate(40 * 60);
	const auto circuit = circuitManager.getCircuit(circuitId);
	const auto blockContainer = circuit->getBlockContainer();
	const Difference difference = blockContainer->getCreationDifference();
	receiver.linkFunction("blockDataRemoveConnection", std::bind(&Evaluator::removeCircuitIO, this, std::placeholders::_1));

	makeEdit(std::make_shared<Difference>(difference), circuitId);

	// // connect makeEdit to circuit
	// circuit->connectListener(this, std::bind(&Evaluator::makeEdit, this, std::placeholders::_1, std::placeholders::_2));
}

void Evaluator::setPause(bool pause) {
	paused = pause;
	if (pause) {
		logInfo("Pausing simulation");
		logicSimulatorWrapper.signalToPause();
	} else {
		logInfo("Unpausing simulation");
		logicSimulatorWrapper.triggerNextTickReset();
		logicSimulatorWrapper.signalToProceed();
	}
}

void Evaluator::reset() {
	logicSimulatorWrapper.initialize(); // wipes all the states
}

void Evaluator::setTickrate(unsigned long long tickrate) {
	assert(tickrate > 0);
	targetTickrate = tickrate;
	if (usingTickrate) {
		logicSimulatorWrapper.setTargetTickrate(tickrate);
	} else {
		logicSimulatorWrapper.setTargetTickrate(1000000000); // 1000000000 clocks / min
	}
}

void Evaluator::setUseTickrate(bool useTickrate) {
	usingTickrate = useTickrate;
	if (useTickrate) {
		logicSimulatorWrapper.setTargetTickrate(targetTickrate);
	} else {
		logicSimulatorWrapper.setTargetTickrate(1000000000); // 1000000000 clocks / min
	}
}

long long int Evaluator::getRealTickrate() const {
	return paused ? 0 : logicSimulatorWrapper.getRealTickrate();
}

void Evaluator::makeEdit(DifferenceSharedPtr difference, circuit_id_t containerId) {
	DiffCache diffCache(circuitManager);
	std::unique_lock<std::shared_mutex> lock = logicSimulatorWrapper.getSimulationUniqueLock();
	makeEditInPlace(difference, containerId, addressTree, diffCache, false);
	lock.unlock();
}

void Evaluator::makeEditInPlace(DifferenceSharedPtr difference, circuit_id_t containerId, AddressTreeNode<EvaluatorGate>& addressTree, DiffCache& diffCache, bool insideIC) {
	const auto modifications = difference->getModifications();
	bool deletedBlocks = false;
	for (const auto& modification : modifications) {
		const auto& [modificationType, modificationData] = modification;
		switch (modificationType) {
		case Difference::REMOVED_BLOCK:
		{
			deletedBlocks = true;
			const auto& [position, rotation, blockType] = std::get<Difference::block_modification_t>(modificationData);
			const auto addresses = addressTree.getPositions(containerId, position);
			for (const auto& address : addresses) {
				const bool exists = addressTree.hasValue(address);
				if (!exists) { // integrated circuit
					const auto branch = addressTree.getBranch(address);
					const auto allValues = branch->getAllValues();
					for (const auto value : allValues) {
						logicSimulatorWrapper.deleteGate(value.gateId);
					}
					addressTree.nukeBranch(address);
				}
				else{
					const wrapper_gate_id_t blockId = addressTree.getValue(address).gateId;
					logicSimulatorWrapper.deleteGate(blockId);
					addressTree.removeValue(address);
				}
			}
			break;
		}
		case Difference::PLACE_BLOCK:
		{
			const auto [position, rotation, blockType] = std::get<Difference::block_modification_t>(modificationData);
			const GateType gateType = circuitToEvaluatorGatetype(blockType, insideIC);
			if (gateType != GateType::NONE) {
				const auto addresses = addressTree.addValue(position, containerId, EvaluatorGate{ 0, blockType, rotation });
				for (const auto& address : addresses) {
					const wrapper_gate_id_t blockId = logicSimulatorWrapper.createGate(gateType, true);
					addressTree.setValue(address, EvaluatorGate{ blockId, blockType, rotation });
				}
			}
			else {
				// check if it's a custom block
				const circuit_id_t integratedCircuitId = circuitManager.getCircuitBlockDataManager()->getCircuitId(blockType);
				if (integratedCircuitId == 0) {
					logError("makeEditInPlace: blockType is not a valid block type");
					break;
				}
				const auto addresses = addressTree.makeBranch(position, containerId, integratedCircuitId, rotation);
				const auto integratedDifference = diffCache.getDifference(integratedCircuitId);
				for (const auto& address : addresses) {
					auto branch = addressTree.getBranch(address);
					makeEditInPlace(integratedDifference, integratedCircuitId, *branch, diffCache, true);
				}
			}
			break;
		}
		case Difference::REMOVED_CONNECTION:
		{
			const auto& [outputBlockPosition, outputPosition, inputBlockPosition, inputPosition] = std::get<Difference::connection_modification_t>(modificationData);
			const auto outputOffset = outputPosition - outputBlockPosition;
			const auto inputOffset = inputPosition - inputBlockPosition;
			const auto outputAddresses = addressTree.getPositions(containerId, outputBlockPosition);
			const auto inputAddresses = addressTree.getPositions(containerId, inputBlockPosition);
			for (int i = 0; i < outputAddresses.size(); i++) {
				const auto outputAddress = outputAddresses[i];
				const auto inputAddress = inputAddresses[i];
				auto outputPoint = getConnectionPoint(addressTree, outputAddress, outputOffset, false);
				auto inputPoint = getConnectionPoint(addressTree, inputAddress, inputOffset, true);
				logicSimulatorWrapper.disconnectGates(outputPoint.first, outputPoint.second, inputPoint.first, inputPoint.second);
			}
			break;
		}
		case Difference::CREATED_CONNECTION:
		{
			const auto& [outputBlockPosition, outputPosition, inputBlockPosition, inputPosition] = std::get<Difference::connection_modification_t>(modificationData);
			const auto outputOffset = outputPosition - outputBlockPosition;
			const auto inputOffset = inputPosition - inputBlockPosition;
			const auto outputAddresses = addressTree.getPositions(containerId, outputBlockPosition);
			const auto inputAddresses = addressTree.getPositions(containerId, inputBlockPosition);
			for (int i = 0; i < outputAddresses.size(); i++) {
				const auto outputAddress = outputAddresses[i];
				const auto inputAddress = inputAddresses[i];
				auto outputPoint = getConnectionPoint(addressTree, outputAddress, outputOffset, false);
				auto inputPoint = getConnectionPoint(addressTree, inputAddress, inputOffset, true);
				logicSimulatorWrapper.connectGates(outputPoint.first, outputPoint.second, inputPoint.first, inputPoint.second);
			}
			break;
		}
		case Difference::MOVE_BLOCK:
		{
			const auto& [curPosition, newPosition] = std::get<Difference::move_modification_t>(modificationData);
			addressTree.moveData(containerId, curPosition, newPosition);
			break;
		}
		case Difference::SET_DATA: break;
		}
	}
}

void Evaluator::removeCircuitIO(const DataUpdateEventManager::EventData* data) {
	logError("removeCircuitIO: not implemented yet");
	const DataUpdateEventManager::EventDataWithValue<RemoveCircuitIOData>* eventData = dynamic_cast<const DataUpdateEventManager::EventDataWithValue<RemoveCircuitIOData>*>(data);
	if (eventData == nullptr) {
		logError("removeCircuitIO: eventData is null");
		return;
	}
	const std::pair<BlockType, connection_end_id_t> dataValue = eventData->get();
	const BlockType blockType = dataValue.first;
	const connection_end_id_t connectionId = dataValue.second;
	const auto blockData = circuitManager.getBlockDataManager()->getBlockData(blockType);

	if (blockData == nullptr) {
		logError("removeCircuitIO: blockData is null");
		return;
	}
	if (blockData->isDefaultData()) {
		logWarning("removeCircuitIO: blockData is default data");
		return;
	}
}

int Evaluator::getGroupIndex(EvaluatorGate gate, const Vector offset, bool trackInput) {
	int groupIndex = 0;
	const wrapper_gate_id_t blockId = gate.gateId;
	const BlockType blockType = gate.blockType;
	const BlockData* blockData = circuitManager.getBlockDataManager()->getBlockData(blockType);
	if (blockData == nullptr) {
		logError("getGroupIndex: blockData is null");
		return 0;
	}
	if (blockData->isDefaultData()) return 0;
	if (trackInput) {
		std::pair<connection_end_id_t, bool> idData = blockData->getInputConnectionId(offset, gate.rotation);
		if (!idData.second) return 0;
		int groupIndex = 0;
		for (connection_end_id_t i = 0; i < idData.first; i++) {
			groupIndex += blockData->isConnectionInput(i);
		}
		return groupIndex;
	} else {
		std::pair<connection_end_id_t, bool> idData = blockData->getOutputConnectionId(offset, gate.rotation);
		if (!idData.second) return 0;
		int groupIndex = 0;
		for (connection_end_id_t i = 0; i < idData.first; i++) {
			groupIndex += blockData->isConnectionOutput(i);
		}
		return groupIndex;
	}
}

std::pair<wrapper_gate_id_t, int> Evaluator::getConnectionPoint(AddressTreeNode<EvaluatorGate>& addressTree, const Address& address, const Vector& offset, bool trackInput) {
	if (addressTree.hasValue(address)) {
		const EvaluatorGate gate = addressTree.getValue(address);
		const int groupIndex = getGroupIndex(gate, offset, trackInput);
		return { gate.gateId, groupIndex };
	}
	// custom circuits here
	const auto branch = addressTree.getBranch(address);
	const circuit_id_t integratedCircuitId = branch->getContainerId();
	const auto integratedCircuit = circuitManager.getCircuit(integratedCircuitId);
	const BlockDataManager* blockDataManager = circuitManager.getBlockDataManager();
	const CircuitBlockDataManager* circuitBlockDataManager = circuitManager.getCircuitBlockDataManager();
	const CircuitBlockData* integratedCircuitBlockData = circuitBlockDataManager->getCircuitBlockData(integratedCircuitId);
	if (integratedCircuitBlockData == nullptr) {
		logError("getConnectionPoint: integratedCircuitBlockData is null");
		return { 0, 0 };
	}
	const BlockData* blockData = blockDataManager->getBlockData(integratedCircuitBlockData->getBlockType());
	if (blockData == nullptr) {
		logError("getConnectionPoint: blockData is null");
		return { 0, 0 };
	}
	connection_end_id_t connectionId = 0;
	if (trackInput) {
		const auto inputConnectionIdData = blockData->getInputConnectionId(offset, branch->getRotation());
		if (inputConnectionIdData.second) {
			connectionId = inputConnectionIdData.first;
		}
		else {
			logError("getConnectionPoint: input connection id not found");
		}
	}
	else {
		const auto outputConnectionIdData = blockData->getOutputConnectionId(offset, branch->getRotation());
		if (outputConnectionIdData.second) {
			connectionId = outputConnectionIdData.first;
		}
		else {
			logError("getConnectionPoint: output connection id not found");
		}
	}
	const Position* connectionPosition = integratedCircuitBlockData->getConnectionIdToPosition(connectionId);
	if (connectionPosition == nullptr) {
		logError("getConnectionPoint: connection position is null");
		return { 0, 0 };
	}
	return { branch->getValue(*connectionPosition).gateId, 0 };
}

GateType circuitToEvaluatorGatetype(BlockType blockType, bool insideIC) {
	switch (blockType) {
	case BlockType::AND: return GateType::AND;
	case BlockType::OR: return GateType::OR;
	case BlockType::XOR: return GateType::XOR;
	case BlockType::NAND: return GateType::NAND;
	case BlockType::NOR: return GateType::NOR;
	case BlockType::XNOR: return GateType::XNOR;
	case BlockType::SWITCH: {
		if (insideIC) {
			return GateType::JUNCTION;
		}
		else {
			return GateType::DEFAULT_RETURN_CURRENTSTATE;
		}
	};
	case BlockType::BUTTON: {
		if (insideIC) {
			return GateType::JUNCTION;
		}
		else {
			return GateType::DEFAULT_RETURN_CURRENTSTATE;
		}
	};
	case BlockType::TICK_BUTTON: {
		if (insideIC) {
			return GateType::JUNCTION;
		}
		else {
			return GateType::TICK_INPUT;
		}
	};
	case BlockType::LIGHT: {
		if (insideIC) {
			return GateType::JUNCTION;
		}
		else {
			return GateType::COPYINPUT;
		}
	};
	case BlockType::JUNCTION: return GateType::JUNCTION;
	case BlockType::TRISTATE_BUFFER: return GateType::TRISTATE_BUFFER;
	default: return GateType::NONE;
	}
}

logic_state_t Evaluator::getState(const Address& address) {
	if (addressTree.hasBranch(address)) {
		return logic_state_t::LOW;
	}
	if (!addressTree.hasValue(address)) {
		return logic_state_t::UNDEFINED;
	}
	const wrapper_gate_id_t blockId = addressTree.getValue(address).gateId;

	std::shared_lock<std::shared_mutex> lock = logicSimulatorWrapper.getSimulationSharedLock();
	const logic_state_t state = logicSimulatorWrapper.getState(blockId, 0); // TODO: 0 temp
	lock.unlock();

	return state;
}

bool Evaluator::getBoolState(const Address& address) {
	return isHigh(getState(address));
}

std::vector<logic_state_t> Evaluator::getBulkStates(const std::vector<Address>& addresses) {
	std::vector<logic_state_t> states;
	states.reserve(addresses.size());
	std::shared_lock<std::shared_mutex> lock = logicSimulatorWrapper.getSimulationSharedLock();
	for (const auto& address : addresses) {
		// check if the address is valid
		if (addressTree.hasBranch(address)) {
			states.push_back(logic_state_t::LOW);
			continue;
		}
		const bool exists = addressTree.hasValue(address);
		if (!exists) {
			states.push_back(logic_state_t::UNDEFINED);
		} else {
			const wrapper_gate_id_t blockId = addressTree.getValue(address).gateId;
			states.push_back(logicSimulatorWrapper.getState(blockId, 0)); // TODO: 0 temp
		}
	}
	lock.unlock();
	return states;
}

void Evaluator::setState(const Address& address, logic_state_t state) {
	const auto gate = addressTree.getValue(address, EvaluatorGate{0, BlockType::NONE, Rotation::ZERO});
	if (gate.blockType == BlockType::NONE) {
		logError("setState: gate is not a valid block type");
		return;
	}
	const wrapper_gate_id_t blockId = gate.gateId;
	std::unique_lock<std::shared_mutex> lock = logicSimulatorWrapper.getSimulationUniqueLock();
	logicSimulatorWrapper.setState(blockId, 0, state); // TODO: 0 temp
	lock.unlock();
}
