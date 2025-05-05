#include "evaluator.h"

Evaluator::Evaluator(evaluator_id_t evaluatorId, CircuitManager& circuitManager, circuit_id_t circuitId, DataUpdateEventManager* dataUpdateEventManager)
	: evaluatorId(evaluatorId), paused(true),
	targetTickrate(0),
	addressTree(circuitId, Rotation::ZERO, dataUpdateEventManager, true),
	usingTickrate(false),
	circuitManager(circuitManager),
	receiver(dataUpdateEventManager) {
	setTickrate(40 * 60);
	const SharedCircuit circuit = circuitManager.getCircuit(circuitId);
	const BlockContainer* blockContainer = circuit->getBlockContainer();
	const Difference difference = blockContainer->getCreationDifference();
	receiver.linkFunction("blockDataRemoveConnection", std::bind(&Evaluator::removeCircuitIOExternal, this, std::placeholders::_1));
	receiver.linkFunction("blockDataSetConnection", std::bind(&Evaluator::registerConnectionIOExternal, this, std::placeholders::_1));
	receiver.linkFunction("circuitBlockDataConnectionPositionSet", std::bind(&Evaluator::registerConnectionIOExternal, this, std::placeholders::_1));

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
	BlockContainerCache bcCache(circuitManager);
	std::unique_lock<std::shared_mutex> lock = logicSimulatorWrapper.getSimulationUniqueLock();
	makeEditInPlace(difference, containerId, addressTree, diffCache, bcCache, false);
	lock.unlock();
}

void Evaluator::makeEditInPlace(DifferenceSharedPtr difference, circuit_id_t containerId, AddressTreeNode& addressTree, DiffCache& diffCache, BlockContainerCache& bcCache, bool insideIC) {
	const std::vector<Difference::Modification> modifications = difference->getModifications();
	bool deletedBlocks = false;
	std::unordered_set<Address> addressesThatGotUnlinked;
	std::unordered_set<Address> addressesToRelink;
	for (const Difference::Modification& modification : modifications) {
		const auto& [modificationType, modificationData] = modification;
		switch (modificationType) {
		case Difference::REMOVED_BLOCK:
		{
			deletedBlocks = true;
			const auto& [position, rotation, blockType] = std::get<Difference::block_modification_t>(modificationData);
			const std::vector<Address> addresses = addressTree.getPositions(containerId, position);
			for (const Address& address : addresses) {
				const bool isRootLevel = address.size() == 1;
				if (!isRootLevel) {
					const Address addressChopped = address.chopLastPosition();
					const bool unlinkedThisAddress = addressesThatGotUnlinked.find(addressChopped) != addressesThatGotUnlinked.end();
					if (!unlinkedThisAddress) { // TODO: naive, should be smarter
						AddressTreeNode* branch = addressTree.getParentBranch(address);
						const std::unordered_map<connection_end_id_t, EvaluatorIOJunction>& IOs = branch->getAllConnectionIOs();
						for (const std::pair<const connection_end_id_t, EvaluatorIOJunction>& io : IOs) {
							unlinkConnectionIO(*branch, io.first, bcCache);
						}
						addressesThatGotUnlinked.insert(addressChopped);
						addressesToRelink.insert(addressChopped);
					}
				}
				const bool exists = addressTree.hasValue(address);
				if (!exists) { // integrated circuit
					const AddressTreeNode* branch = addressTree.getBranch(address);
					if (branch == nullptr) {
						logError("makeEditInPlace: branch is null");
						continue;
					}
					const std::vector<EvaluatorGate> allValues = branch->getAllValues();
					for (const EvaluatorGate value : allValues) {
						logicSimulatorWrapper.deleteGate(value.gateId);
					}
					const std::vector<EvaluatorIOJunction> allIOs = branch->getAllIOs();
					for (const EvaluatorIOJunction io : allIOs) {
						logicSimulatorWrapper.deleteGate(io.junctionId);
					}
					addressTree.nukeBranch(address);
				} else {
					const wrapper_gate_id_t blockId = addressTree.getValue(address)->gateId;
					logicSimulatorWrapper.deleteGate(blockId);
					addressTree.removeValue(address);
				}
			}
			break;
		}
		case Difference::PLACE_BLOCK:
		{
			const auto [position, rotation, blockType] = std::get<Difference::block_modification_t>(modificationData);
			const GateType gateType = circuitToEvaluatorGatetype(blockType);
			if (gateType != GateType::NONE) {
				const std::vector<Address> addresses = addressTree.addValue(position, containerId, EvaluatorGate { 0, blockType, rotation });
				for (const Address& address : addresses) {
					if (address.size() != 1) {
						const Address addressChopped = address.chopLastPosition();
						addressesToRelink.insert(addressChopped);
					}
					const wrapper_gate_id_t blockId = logicSimulatorWrapper.createGate(gateType, true);
					addressTree.setValue(address, EvaluatorGate { blockId, blockType, rotation });
				}
			} else {
				// check if it's a custom block
				const circuit_id_t integratedCircuitId = circuitManager.getCircuitBlockDataManager()->getCircuitId(blockType);
				if (integratedCircuitId == 0) {
					logError("makeEditInPlace: blockType is not a valid block type");
					break;
				}
				const std::vector<Address> addresses = addressTree.makeBranch(position, containerId, integratedCircuitId, rotation);
				const std::shared_ptr<Difference> integratedDifference = diffCache.getDifference(integratedCircuitId);
				const BlockData* blockData = circuitManager.getBlockDataManager()->getBlockData(blockType);

				if (blockData == nullptr) {
					logError("makeEditInPlace: blockData is null");
					break;
				}
				const std::vector<connection_end_id_t> connectionIds = blockData->getConnectionIds();

				for (const Address& address : addresses) {
					if (address.size() != 1) {
						const Address addressChopped = address.chopLastPosition();
						addressesToRelink.insert(addressChopped);
					}
					AddressTreeNode* branch = addressTree.getBranch(address);
					if (branch == nullptr) {
						logError("makeEditInPlace: branch is null");
						continue;
					}
					makeEditInPlace(integratedDifference, integratedCircuitId, *branch, diffCache, bcCache, true);
					for (const connection_end_id_t connectionId : connectionIds) {
						registerConnectionIO(*branch, connectionId, bcCache);
					}
				}
			}
			break;
		}
		case Difference::REMOVED_CONNECTION:
		{
			const auto& [outputBlockPosition, outputPosition, inputBlockPosition, inputPosition] = std::get<Difference::connection_modification_t>(modificationData);
			const Vector outputOffset = outputPosition - outputBlockPosition;
			const Vector inputOffset = inputPosition - inputBlockPosition;
			const std::vector<Address>& outputAddresses = addressTree.getPositions(containerId, outputBlockPosition);
			const std::vector<Address>& inputAddresses = addressTree.getPositions(containerId, inputBlockPosition);
			for (size_t i = 0; i < outputAddresses.size(); i++) {
				const Address& outputAddress = outputAddresses.at(i);
				const Address& inputAddress = inputAddresses.at(i);
				std::pair<wrapper_gate_id_t, int> outputPoint = getConnectionPoint(addressTree, outputAddress, outputOffset, false);
				std::pair<wrapper_gate_id_t, int> inputPoint = getConnectionPoint(addressTree, inputAddress, inputOffset, true);
				logicSimulatorWrapper.disconnectGates(outputPoint.first, outputPoint.second, inputPoint.first, inputPoint.second);
			}
			break;
		}
		case Difference::CREATED_CONNECTION:
		{
			const auto& [outputBlockPosition, outputPosition, inputBlockPosition, inputPosition] = std::get<Difference::connection_modification_t>(modificationData);
			const Vector outputOffset = outputPosition - outputBlockPosition;
			const Vector inputOffset = inputPosition - inputBlockPosition;
			const std::vector<Address>& outputAddresses = addressTree.getPositions(containerId, outputBlockPosition);
			const std::vector<Address>& inputAddresses = addressTree.getPositions(containerId, inputBlockPosition);
			for (size_t i = 0; i < outputAddresses.size(); i++) {
				const Address& outputAddress = outputAddresses[i];
				const Address& inputAddress = inputAddresses[i];
				std::pair<wrapper_gate_id_t, int> outputPoint = getConnectionPoint(addressTree, outputAddress, outputOffset, false);
				std::pair<wrapper_gate_id_t, int> inputPoint = getConnectionPoint(addressTree, inputAddress, inputOffset, true);
				logicSimulatorWrapper.connectGates(outputPoint.first, outputPoint.second, inputPoint.first, inputPoint.second);
			}
			break;
		}
		case Difference::MOVE_BLOCK:
		{
			const auto& [curPosition, newPosition] = std::get<Difference::move_modification_t>(modificationData);
			const std::vector<Address>& addresses = addressTree.getPositions(containerId, curPosition);
			for (const Address& address : addresses) {
				const bool isRootLevel = address.size() == 1;
				if (!isRootLevel) {
					const Address addressChopped = address.chopLastPosition();
					const bool unlinkedThisAddress = addressesThatGotUnlinked.find(addressChopped) != addressesThatGotUnlinked.end();
					if (!unlinkedThisAddress) { // TODO: naive, should be smarter
						AddressTreeNode* branch = addressTree.getParentBranch(address);
						if (branch == nullptr) {
							logError("makeEditInPlace: branch is null");
							continue;
						}
						const std::unordered_map<connection_end_id_t, EvaluatorIOJunction>& IOs = branch->getAllConnectionIOs();
						for (const std::pair<const connection_end_id_t, EvaluatorIOJunction>& io : IOs) {
							unlinkConnectionIO(*branch, io.first, bcCache);
						}
						addressesThatGotUnlinked.insert(addressChopped);
						addressesToRelink.insert(addressChopped);
					}
				}
			}
			addressTree.moveData(containerId, curPosition, newPosition);
			break;
		}
		case Difference::SET_DATA: break;
		}
	}
	for (const Address& address : addressesToRelink) {
		AddressTreeNode* branch = addressTree.getBranch(address);
		if (branch == nullptr) {
			logError("makeEditInPlace: branch is null");
			continue;
		}
		const std::unordered_map<connection_end_id_t, EvaluatorIOJunction> IOs = branch->getAllConnectionIOs();
		for (const std::pair<const connection_end_id_t, EvaluatorIOJunction>& io : IOs) {
			linkConnectionIO(*branch, io.first, bcCache);
		}
	}
}

int Evaluator::getGroupIndex(const EvaluatorGate& gate, Vector offset, bool trackInput) {
	int groupIndex = 0;
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
		for (connection_end_id_t i = 0; i < idData.first; i++) {
			groupIndex += blockData->isConnectionInput(i);
		}
		return groupIndex;
	} else {
		std::pair<connection_end_id_t, bool> idData = blockData->getOutputConnectionId(offset, gate.rotation);
		if (!idData.second) return 0;
		for (connection_end_id_t i = 0; i < idData.first; i++) {
			groupIndex += blockData->isConnectionOutput(i);
		}
		return groupIndex;
	}
}

std::pair<wrapper_gate_id_t, int> Evaluator::getConnectionPoint(AddressTreeNode& branch, const Address& address, Vector offset, bool trackInput) {
	if (branch.hasValue(address)) {
		const EvaluatorGate* gate = branch.getValue(address);
		const int groupIndex = getGroupIndex(*gate, offset, trackInput);
		return { gate->gateId, groupIndex };
	}
	// custom circuits here
	const AddressTreeNode* integratedCircuitBranch = branch.getBranch(address);
	const circuit_id_t integratedCircuitId = integratedCircuitBranch->getContainerId();
	const SharedCircuit integratedCircuit = circuitManager.getCircuit(integratedCircuitId);
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
		const std::pair<connection_end_id_t, bool> inputConnectionIdData = blockData->getInputConnectionId(offset, integratedCircuitBranch->getRotation());
		if (inputConnectionIdData.second) {
			connectionId = inputConnectionIdData.first;
		} else {
			logError("getConnectionPoint: input connection id not found");
		}
	} else {
		const std::pair<connection_end_id_t, bool> outputConnectionIdData = blockData->getOutputConnectionId(offset, integratedCircuitBranch->getRotation());
		if (outputConnectionIdData.second) {
			connectionId = outputConnectionIdData.first;
		} else {
			logError("getConnectionPoint: output connection id not found");
		}
	}
	const Position* connectionPosition = integratedCircuitBlockData->getConnectionIdToPosition(connectionId);
	if (connectionPosition == nullptr) {
		logError("getConnectionPoint: connection position is null");
		return { 0, 0 };
	}
	// return { branch->getValue(*connectionPosition).gateId, 0 };
	return { integratedCircuitBranch->getConnectionIO(connectionId)->junctionId, 0 };
}

GateType circuitToEvaluatorGatetype(BlockType blockType) {
	switch (blockType) {
	case BlockType::AND: return GateType::AND;
	case BlockType::OR: return GateType::OR;
	case BlockType::XOR: return GateType::XOR;
	case BlockType::NAND: return GateType::NAND;
	case BlockType::NOR: return GateType::NOR;
	case BlockType::XNOR: return GateType::XNOR;
	case BlockType::SWITCH: {
		return GateType::DEFAULT_RETURN_CURRENTSTATE;
	};
	case BlockType::BUTTON: {
		return GateType::DEFAULT_RETURN_CURRENTSTATE;
	};
	case BlockType::TICK_BUTTON: {
		return GateType::TICK_INPUT;
	};
	case BlockType::LIGHT: {
		return GateType::COPYINPUT;
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
	const wrapper_gate_id_t blockId = addressTree.getValue(address)->gateId;

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
	for (const Address& address : addresses) {
		// check if the address is valid
		if (addressTree.hasBranch(address)) {
			states.push_back(logic_state_t::LOW);
			continue;
		}
		const bool exists = addressTree.hasValue(address);
		if (!exists) {
			states.push_back(logic_state_t::UNDEFINED);
		} else {
			const wrapper_gate_id_t blockId = addressTree.getValue(address)->gateId;
			states.push_back(logicSimulatorWrapper.getState(blockId, 0)); // TODO: 0 temp
		}
	}
	lock.unlock();
	return states;
}

void Evaluator::setState(const Address& address, logic_state_t state) {
	const EvaluatorGate* gate = addressTree.getValue(address);
	if (!gate || gate->blockType == BlockType::NONE) {
		logError("setState: gate is not a valid block type");
		return;
	}
	const wrapper_gate_id_t blockId = gate->gateId;
	std::unique_lock<std::shared_mutex> lock = logicSimulatorWrapper.getSimulationUniqueLock();
	logicSimulatorWrapper.setState(blockId, 0, state); // TODO: 0 temp
	debugPrint(blockId);
	lock.unlock();
}

void Evaluator::debugPrint(wrapper_gate_id_t blockId) {
	const Gate& simGate = logicSimulatorWrapper.getGate(blockId);
	const simulator_gate_id_t simulatorGateId = logicSimulatorWrapper.getSimulatorGateId(blockId);
	logInfo("_______________________________");
	logInfo("Gate ID: " + std::to_string(simulatorGateId));
	logInfo("Gate Type: {}", "", static_cast<int>(simGate.type));
	logInfo("Inputs:");
	for (size_t i = 0; i < simGate.getInputGroupCount(); ++i) {
		logInfo("Group {}: ", "", static_cast<int>(i));
		for (const GateConnection input : simGate.inputGroups[i]) {
			logInfo("	Input: {} {}", "", static_cast<int>(input.gateId), static_cast<int>(input.group));
		}
	}
	logInfo("Outputs:");
	for (size_t i = 0; i < simGate.getOutputGroupCount(); ++i) {
		logInfo("Group {}: ", "", static_cast<int>(i));
		for (const GateConnection output : simGate.outputGroups[i]) {
			logInfo("	Output: {} {}", "", static_cast<int>(output.gateId), static_cast<int>(output.group));
		}
	}
}

void Evaluator::registerConnectionIOExternal(const DataUpdateEventManager::EventData* data) {
	const DataUpdateEventManager::EventDataWithValue<CircuitIOUpdateData>* eventData = dynamic_cast<const DataUpdateEventManager::EventDataWithValue<CircuitIOUpdateData>*>(data);
	if (eventData == nullptr) {
		logError("registerConnectionIO: eventData is null");
		return;
	}
	const std::pair<BlockType, connection_end_id_t> dataValue = eventData->get();
	const BlockType blockType = dataValue.first;
	const connection_end_id_t connectionId = dataValue.second;
	const BlockData* blockData = circuitManager.getBlockDataManager()->getBlockData(blockType);

	if (blockData == nullptr) {
		logError("removeCircuitIO: blockData is null");
		return;
	}
	if (blockData->isDefaultData()) {
		logWarning("removeCircuitIO: blockData is default data");
		return;
	}
	const circuit_id_t integratedCircuitId = circuitManager.getCircuitBlockDataManager()->getCircuitId(blockType);
	const std::vector<AddressTreeNode*> branches = addressTree.getBranches(integratedCircuitId);
	BlockContainerCache blockContainerCache = BlockContainerCache(circuitManager);
	std::unique_lock<std::shared_mutex> lock = logicSimulatorWrapper.getSimulationUniqueLock();
	for (AddressTreeNode* branch : branches) {
		if (branch == nullptr) {
			logError("registerConnectionIO: branch is null");
			continue;
		}
		if (!(branch->isNodeRoot())) {
			registerConnectionIO(*branch, connectionId, blockContainerCache);
		}
	}
}

void Evaluator::registerConnectionIO(AddressTreeNode& branch, connection_end_id_t connectionId, BlockContainerCache& blockContainerCache) {
	const bool exists = branch.hasConnectionIO(connectionId);
	if (!exists) {
		const wrapper_gate_id_t junctionId = logicSimulatorWrapper.createGate(GateType::JUNCTION, true);
		branch.addConnectionIO(connectionId, EvaluatorIOJunction {
				junctionId,
				true,
				{0, 0},
				GateType::NONE,
				BlockType::NONE,
			}
			);
	}
	linkConnectionIO(branch, connectionId, blockContainerCache);
}

void Evaluator::unlinkCircuitIOExternal(const DataUpdateEventManager::EventData* data) {
	const DataUpdateEventManager::EventDataWithValue<CircuitIOUpdateData>* eventData = dynamic_cast<const DataUpdateEventManager::EventDataWithValue<CircuitIOUpdateData>*>(data);
	if (eventData == nullptr) {
		logError("removeCircuitIO: eventData is null");
		return;
	}
	const std::pair<BlockType, connection_end_id_t> dataValue = eventData->get();
	const BlockType blockType = dataValue.first;
	const connection_end_id_t connectionId = dataValue.second;
	const BlockData* blockData = circuitManager.getBlockDataManager()->getBlockData(blockType);

	if (blockData == nullptr) {
		logError("removeCircuitIO: blockData is null");
		return;
	}
	if (blockData->isDefaultData()) {
		logWarning("removeCircuitIO: blockData is default data");
		return;
	}
	const circuit_id_t integratedCircuitId = circuitManager.getCircuitBlockDataManager()->getCircuitId(blockType);
	const std::vector<AddressTreeNode*> branches = addressTree.getBranches(integratedCircuitId);
	BlockContainerCache blockContainerCache = BlockContainerCache(circuitManager);
	std::unique_lock<std::shared_mutex> lock = logicSimulatorWrapper.getSimulationUniqueLock();
	for (AddressTreeNode* branch : branches) {
		if (branch == nullptr) {
			logError("unlinkCircuitIO: branch is null");
			continue;
		}
		if (!(branch->isNodeRoot())) {
			unlinkConnectionIO(*branch, connectionId, blockContainerCache);
		}
	}
}

void Evaluator::removeCircuitIOExternal(const DataUpdateEventManager::EventData* data) {
	const DataUpdateEventManager::EventDataWithValue<CircuitIOUpdateData>* eventData = dynamic_cast<const DataUpdateEventManager::EventDataWithValue<CircuitIOUpdateData>*>(data);
	if (eventData == nullptr) {
		logError("removeCircuitIO: eventData is null");
		return;
	}
	const std::pair<BlockType, connection_end_id_t> dataValue = eventData->get();
	const BlockType blockType = dataValue.first;
	const connection_end_id_t connectionId = dataValue.second;
	const BlockData* blockData = circuitManager.getBlockDataManager()->getBlockData(blockType);

	if (blockData == nullptr) {
		logError("removeCircuitIO: blockData is null");
		return;
	}
	if (blockData->isDefaultData()) {
		logWarning("removeCircuitIO: blockData is default data");
		return;
	}
	const circuit_id_t integratedCircuitId = circuitManager.getCircuitBlockDataManager()->getCircuitId(blockType);
	const std::vector<AddressTreeNode*> branches = addressTree.getBranches(integratedCircuitId);
	BlockContainerCache blockContainerCache = BlockContainerCache(circuitManager);
	std::unique_lock<std::shared_mutex> lock = logicSimulatorWrapper.getSimulationUniqueLock();
	for (AddressTreeNode* branch : branches) {
		if (branch == nullptr) {
			logError("removeCircuitIO: branch is null");
			continue;
		}
		if (!(branch->isNodeRoot())) {
			removeCircuitIO(*branch, connectionId, blockContainerCache);
		}
	}
}

void Evaluator::removeCircuitIO(AddressTreeNode& branch, connection_end_id_t connectionId, BlockContainerCache& blockContainerCache) {
	const bool exists = branch.hasConnectionIO(connectionId);
	if (!exists) {
		logError("removeCircuitIO: connectionId does not exist");
		return;
	}
	unlinkConnectionIO(branch, connectionId, blockContainerCache);
	const EvaluatorIOJunction* connectionIO = branch.getConnectionIO(connectionId);
	const wrapper_gate_id_t blockId = connectionIO->junctionId;
	logicSimulatorWrapper.deleteGate(blockId);
	branch.removeConnectionIO(connectionId);
}

bool Evaluator::isIOanInput(connection_end_id_t connectionId, AddressTreeNode& branch) const {
	const BlockType blockType = circuitManager.getCircuitBlockDataManager()->getCircuitBlockData(branch.getContainerId())->getBlockType();
	const BlockData* blockData = circuitManager.getBlockDataManager()->getBlockData(blockType);
	if (blockData == nullptr) {
		logError("isIOanInput: blockData is null");
		return false;
	}
	return blockData->isConnectionInput(connectionId);
}

std::pair<bool, std::pair<wrapper_gate_id_t, int>> Evaluator::getConnectionAtPosition(AddressTreeNode& branch, const Position& position, BlockContainerCache& blockContainerCache, bool trackInput) {
	const BlockContainer* blockContainer = blockContainerCache.getBlockContainer(branch.getContainerId());
	const Block* block = blockContainer->getBlock(position);
	if (block == nullptr) {
		return { false, {0, 0} };
	}
	const Position blockPosition = block->getPosition();
	const Rotation blockRotation = block->getRotation();
	const Vector offset = position - blockPosition;
	// getConnectionPoint
	const Address addr = Address(blockPosition);
	const EvaluatorGate* gate = branch.getValue(blockPosition);
	if (gate) {
		const int groupIndex = getGroupIndex(*gate, offset, trackInput);
		return { true, { gate->gateId, groupIndex } };
	}
	// check for custom circuits here
	const AddressTreeNode* integratedCircuitBranch = branch.getBranch(addr);
	if (!integratedCircuitBranch) {
		return { false, {0, 0} };
	}
	std::pair<connection_end_id_t, bool> connectionIdData = block->getInputConnectionId(position);
	if (connectionIdData.second == trackInput) {
		return { true, {integratedCircuitBranch->getConnectionIO(connectionIdData.first)->junctionId, 0} };
	}
	logError("getConnectionPoint: connection id not found");
	return { false, {0, 0} };
}

const Position* Evaluator::getConnectionTargetPosition(AddressTreeNode& branch, connection_end_id_t connectionId) const {
	return circuitManager.getCircuitBlockDataManager()->getCircuitBlockData(branch.getContainerId())->getConnectionIdToPosition(connectionId);
}

void Evaluator::linkConnectionIO(AddressTreeNode& branch, connection_end_id_t connectionId, BlockContainerCache& blockContainerCache) {
	EvaluatorIOJunction* connectionIO = branch.getConnectionIO(connectionId);
	if (!connectionIO) {
		logError("linkConnectionIO: connectionId does not exist");
		return;
	}
	if (!connectionIO->isFloating) {
		unlinkConnectionIO(branch, connectionId, blockContainerCache);
	}
	const bool isInput = isIOanInput(connectionId, branch);
	const Position* position = getConnectionTargetPosition(branch, connectionId);
	if (position == nullptr) {
		logError("linkConnectionIO: position is null");
		return;
	}
	const std::pair<bool, std::pair<wrapper_gate_id_t, int>> connectionPoint = getConnectionAtPosition(branch, *position, blockContainerCache, isInput);
	if (!connectionPoint.first) return;
	wrapper_gate_id_t blockId = connectionPoint.second.first;
	int groupIndex = connectionPoint.second.second;
	wrapper_gate_id_t junctionId = connectionIO->junctionId;
	GateType originalGateType = logicSimulatorWrapper.getGateType(blockId);
	BlockType originalBlockType = branch.getValue(*position)->blockType;
	const bool needToReplace = (originalGateType == GateType::DEFAULT_RETURN_CURRENTSTATE || originalGateType == GateType::TICK_INPUT || originalGateType == GateType::COPYINPUT);
	if (needToReplace) {
		const EvaluatorGate* originalGate = branch.getValue(*position);
		std::vector<std::pair<wrapper_gate_id_t, int>> connections;
		if (isInput) connections = logicSimulatorWrapper.get1x1GateOutputs(blockId);
		else connections = logicSimulatorWrapper.get1x1GateInputs(blockId);

		logicSimulatorWrapper.deleteGate(blockId);
		blockId = logicSimulatorWrapper.createGate(GateType::JUNCTION, true);
		if (isInput) {
			for (const std::pair<wrapper_gate_id_t, int> output : connections) {
				logicSimulatorWrapper.connectGates(blockId, 0, output.first, output.second);
			}
		} else {
			for (const std::pair<wrapper_gate_id_t, int> input : connections) {
				logicSimulatorWrapper.connectGates(input.first, input.second, blockId, 0);
			}
		}
		EvaluatorGate gate {
			blockId,
			BlockType::JUNCTION,
			originalGate->rotation
		};
		branch.setValue(*position, gate);
	}
	if (isInput) {
		logicSimulatorWrapper.connectGates(junctionId, 0, blockId, groupIndex);
	} else {
		logicSimulatorWrapper.connectGates(blockId, groupIndex, junctionId, 0);
	}
	connectionIO->isFloating = false;
	connectionIO->outputTarget = {
		blockId,
		groupIndex
	};
	connectionIO->originalTargetGateType = originalGateType;
	connectionIO->originalTargetBlockType = originalBlockType;
}

void Evaluator::unlinkConnectionIO(AddressTreeNode& branch, connection_end_id_t connectionId, BlockContainerCache& blockContainerCache) {
	EvaluatorIOJunction* connectionIO = branch.getConnectionIO(connectionId);
	if (!connectionIO) {
		logError("unlinkConnectionIO: connectionId does not exist");
		return;
	}
	if (connectionIO->isFloating) {
		return;
	}
	const bool isInput = isIOanInput(connectionId, branch);
	wrapper_gate_id_t blockId = connectionIO->outputTarget.first;
	const int groupIndex = connectionIO->outputTarget.second;
	const wrapper_gate_id_t junctionId = connectionIO->junctionId;
	GateType originalGateType = connectionIO->originalTargetGateType;
	if (isInput) {
		logicSimulatorWrapper.disconnectGates(junctionId, 0, blockId, groupIndex);
		if (originalGateType == GateType::DEFAULT_RETURN_CURRENTSTATE || originalGateType == GateType::TICK_INPUT) {
			const std::vector<std::pair<wrapper_gate_id_t, int>>& inputs = logicSimulatorWrapper.get1x1GateInputs(blockId);
			if (inputs.size() == 0) {
				const Position* originalGatePosition = branch.getPositionOfGateById(blockId);
				if (originalGatePosition == nullptr) {
					logError("unlinkConnectionIO: originalGatePosition is null");
					return;
				}
				const EvaluatorGate* originalGate = branch.getValue(*originalGatePosition);

				const std::vector<std::pair<wrapper_gate_id_t, int>>& outputs = logicSimulatorWrapper.get1x1GateOutputs(blockId);
				logicSimulatorWrapper.deleteGate(blockId);
				blockId = logicSimulatorWrapper.createGate(originalGateType, true);
				for (const std::pair<wrapper_gate_id_t, int> output : outputs) {
					logicSimulatorWrapper.connectGates(blockId, 0, output.first, output.second);
				}
				EvaluatorGate gate {
					blockId,
					connectionIO->originalTargetBlockType,
					originalGate->rotation
				};
				branch.setValue(*originalGatePosition, gate);
				originalGateType = GateType::NONE;
			}
		}
	} else {
		logicSimulatorWrapper.disconnectGates(blockId, groupIndex, junctionId, 0);
		if (originalGateType == GateType::COPYINPUT) {
			const std::vector<std::pair<wrapper_gate_id_t, int>> outputs = logicSimulatorWrapper.get1x1GateOutputs(blockId);
			if (outputs.size() == 0) {
				const Position* originalGatePosition = branch.getPositionOfGateById(blockId);
				const std::vector<std::pair<wrapper_gate_id_t, int>> inputs = logicSimulatorWrapper.get1x1GateInputs(blockId);
				logicSimulatorWrapper.deleteGate(blockId);
				blockId = logicSimulatorWrapper.createGate(originalGateType, true);
				for (const std::pair<wrapper_gate_id_t, int> input : inputs) {
					logicSimulatorWrapper.connectGates(input.first, input.second, blockId, 0);
				}
				EvaluatorGate gate {
					blockId,
					connectionIO->originalTargetBlockType,
					branch.getValue(*originalGatePosition)->rotation
				};
				branch.setValue(*originalGatePosition, gate);
				originalGateType = GateType::NONE;
			}
		}
	}
	connectionIO->isFloating = true;
	connectionIO->outputTarget = { 0, 0 };
	connectionIO->originalTargetGateType = originalGateType;
}