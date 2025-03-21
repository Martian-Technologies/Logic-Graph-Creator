#include "evaluator.h"

Evaluator::Evaluator(evaluator_id_t evaluatorId, CircuitManager& circuitManager, circuit_id_t circuitId)
	: evaluatorId(evaluatorId), paused(true),
	targetTickrate(0),
	addressTree(circuitId),
	usingTickrate(false),
	circuitManager(circuitManager) {

	setTickrate(40 * 60);
	const auto circuit = circuitManager.getCircuit(circuitId);
	const auto blockContainer = circuit->getBlockContainer();
	const Difference difference = blockContainer->getCreationDifference();

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
	makeEditInPlace(difference, containerId, addressTree);
}

void Evaluator::makeEditInPlace(DifferenceSharedPtr difference, circuit_id_t containerId, AddressTreeNode<EvaluatorGate>& addressTree) {
	logicSimulatorWrapper.signalToPause();
	// wait for the thread to pause
	while (!logicSimulatorWrapper.threadIsWaiting()) {
		std::this_thread::yield();
	}
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
					const auto allValues = branch.getAllValues();
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
			const auto& [position, rotation, blockType] = std::get<Difference::block_modification_t>(modificationData);
			const GateType gateType = circuitToEvaluatorGatetype(blockType);
			if (gateType != GateType::NONE) {
				const auto addresses = addressTree.addValue(position, containerId, EvaluatorGate{ 0, blockType });
				for (const auto& address : addresses) {
					const wrapper_gate_id_t blockId = logicSimulatorWrapper.createGate(gateType, true);
					addressTree.setValue(address, EvaluatorGate{ blockId, blockType });
				}
			}
			else {
				// check if it's a custom block
				const circuit_id_t integratedCircuitId = circuitManager.getCircuitBlockDataManager()->getCircuitId(blockType);
				if (integratedCircuitId == 0) {
					logError("makeEditInPlace: blockType is not a valid block type");
					break;
				}
				const auto addresses = addressTree.makeBranch(position, containerId, integratedCircuitId);
				const auto integratedCircuit = circuitManager.getCircuit(integratedCircuitId);
				const auto integratedBlockContainer = integratedCircuit->getBlockContainer();
				const auto integratedDifference = std::make_shared<Difference>(integratedBlockContainer->getCreationDifference());
				for (const auto& address : addresses) {
					AddressTreeNode<EvaluatorGate>& branch = addressTree.getBranch(address);
					makeEditInPlace(integratedDifference, integratedCircuitId, branch);
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
				const EvaluatorGate outputBlock = addressTree.getValue(outputAddress);
				const EvaluatorGate inputBlock = addressTree.getValue(inputAddress);
				int outputGroupIndex = getGroupIndex(outputBlock, outputOffset, false);
				int inputGroupIndex = getGroupIndex(inputBlock, inputOffset, true);
				logicSimulatorWrapper.disconnectGates(outputBlock.gateId, outputGroupIndex, inputBlock.gateId, inputGroupIndex);
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
				const EvaluatorGate outputBlock = addressTree.getValue(outputAddress);
				const EvaluatorGate inputBlock = addressTree.getValue(inputAddress);
				int outputGroupIndex = getGroupIndex(outputBlock, outputOffset, false);
				int inputGroupIndex = getGroupIndex(inputBlock, inputOffset, true);
				logicSimulatorWrapper.connectGates(outputBlock.gateId, outputGroupIndex, inputBlock.gateId, inputGroupIndex);
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
	if (!paused) {
		logicSimulatorWrapper.signalToProceed();
	}
}

int Evaluator::getGroupIndex(EvaluatorGate gate, const Vector& offset, bool trackInput) {
	int groupIndex = 0;
	const wrapper_gate_id_t blockId = gate.gateId;
	const BlockType blockType = gate.blockType;
	const BlockData* blockData = circuitManager.getBlockDataManager()->getBlockData(blockType);
	if (blockData == nullptr) {
		logError("getGroupIndex: blockData is null");
		return 0;
	}

	const auto connections = blockData->getConnections();
	for (int j = 0; j < connections.size(); j++) {
		if (connections[j].first == offset) {
			break;
		}
		if (connections[j].second == trackInput) {
			groupIndex++;
		}
	}
	return groupIndex;
}

GateType circuitToEvaluatorGatetype(BlockType blockType) {
	switch (blockType) {
	case BlockType::AND: return GateType::AND;
	case BlockType::OR: return GateType::OR;
	case BlockType::XOR: return GateType::XOR;
	case BlockType::NAND: return GateType::NAND;
	case BlockType::NOR: return GateType::NOR;
	case BlockType::XNOR: return GateType::XNOR;
	case BlockType::SWITCH: return GateType::DEFAULT_RETURN_CURRENTSTATE;
	case BlockType::BUTTON: return GateType::DEFAULT_RETURN_CURRENTSTATE;
	case BlockType::TICK_BUTTON: return GateType::TICK_INPUT;
	case BlockType::LIGHT: return GateType::COPYINPUT;
	case BlockType::JUNCTION: return GateType::JUNCTION;
	case BlockType::TRISTATE_BUFFER: return GateType::TRISTATE_BUFFER;
	default: return GateType::NONE;
	}
}

logic_state_t Evaluator::getState(const Address& address) {
	const wrapper_gate_id_t blockId = addressTree.getValue(address).gateId;

	logicSimulatorWrapper.signalToPause();
	while (!logicSimulatorWrapper.threadIsWaiting()) {
		std::this_thread::yield();
	}
	const logic_state_t state = logicSimulatorWrapper.getState(blockId, 0); // TODO: 0 temp
	if (!paused) {
		logicSimulatorWrapper.signalToProceed();
	}
	return state;
}

bool Evaluator::getBoolState(const Address& address) {
	return isHigh(getState(address));
}

std::vector<logic_state_t> Evaluator::getBulkStates(const std::vector<Address>& addresses) {
	std::vector<logic_state_t> states;
	states.reserve(addresses.size());
	logicSimulatorWrapper.signalToPause();
	while (!logicSimulatorWrapper.threadIsWaiting()) {
		std::this_thread::yield();
	}
	for (const auto& address : addresses) {
		// check if the address is valid
		const bool exists = addressTree.hasValue(address);
		if (!exists) {
			states.push_back(logic_state_t::UNDEFINED);
		} else {
			const wrapper_gate_id_t blockId = addressTree.getValue(address).gateId;
			states.push_back(logicSimulatorWrapper.getState(blockId, 0)); // TODO: 0 temp
		}
	}
	if (!paused) {
		logicSimulatorWrapper.signalToProceed();
	}
	return states;
}

void Evaluator::setState(const Address& address, logic_state_t state) {
	const wrapper_gate_id_t blockId = addressTree.getValue(address).gateId;
	logicSimulatorWrapper.signalToPause();
	while (!logicSimulatorWrapper.threadIsWaiting()) {
		std::this_thread::yield();
	}
	logicSimulatorWrapper.setState(blockId, 0, state); // TODO: 0 temp
	if (!paused) {
		logicSimulatorWrapper.signalToProceed();
	}
}
