#include "evaluator.h"

Evaluator::Evaluator(evaluator_id_t evaluatorId, CircuitManager& circuitManager, circuit_id_t circuitId)
	: evaluatorId(evaluatorId), paused(true),
	targetTickrate(0),
	logicSimulator(),
	addressTree(circuitId),
	usingTickrate(false),
	circuitManager(circuitManager) {

	setTickrate(40 * 60); // 1000000000 clocks / min
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
		logicSimulator.signalToPause();
	} else {
		logInfo("Unpausing simulation");
		logicSimulator.triggerNextTickReset();
		logicSimulator.signalToProceed();
	}
}

void Evaluator::reset() {
	logicSimulator.initialize(); // wipes all the states
}

void Evaluator::setTickrate(unsigned long long tickrate) {
	assert(tickrate > 0);
	targetTickrate = tickrate;
	if (usingTickrate) {
		logicSimulator.setTargetTickrate(tickrate);
	} else {
		logicSimulator.setTargetTickrate(1000000000); // 1000000000 clocks / min
	}
}

void Evaluator::setUseTickrate(bool useTickrate) {
	usingTickrate = useTickrate;
	if (useTickrate) {
		logicSimulator.setTargetTickrate(targetTickrate);
	} else {
		logicSimulator.setTargetTickrate(1000000000); // 1000000000 clocks / min
	}
}

long long int Evaluator::getRealTickrate() const {
	return paused ? 0 : logicSimulator.getRealTickrate();
}


void Evaluator::runNTicks(unsigned long long n) {
	// TODO: make this happen in the thread via leaky bucket
	logicSimulator.simulateNTicks(n);
}

void Evaluator::makeEdit(DifferenceSharedPtr difference, circuit_id_t containerId) {
	makeEditInPlace(difference, containerId, addressTree);
}

void Evaluator::makeEditInPlace(DifferenceSharedPtr difference, circuit_id_t containerId, AddressTreeNode<block_id_t>& addressTree) {
	logicSimulator.signalToPause();
	// wait for the thread to pause
	while (!logicSimulator.threadIsWaiting()) {
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
				const block_id_t blockId = addressTree.getValue(address);
				logicSimulator.decomissionGate(blockId);
				addressTree.removeValue(address);
			}
			break;
		}
		case Difference::PLACE_BLOCK:
		{
			const auto& [position, rotation, blockType] = std::get<Difference::block_modification_t>(modificationData);
			const GateType gateType = circuitToEvaluatorGatetype(blockType);
			if (gateType != GateType::NONE) {
				const auto addresses = addressTree.addValue(position, containerId, 0);
				for (const auto& address : addresses) {
					const block_id_t blockId = logicSimulator.addGate(gateType, true);
					addressTree.setValue(address, blockId);
				}
			}
			else {
				// check if it's a custom block
				
			}
			break;
		}
		case Difference::REMOVED_CONNECTION:
		{
			const auto& [outputPosition, inputPosition] = std::get<Difference::connection_modification_t>(modificationData);
			const auto outputAddresses = addressTree.getPositions(containerId, outputPosition);
			const auto inputAddresses = addressTree.getPositions(containerId, inputPosition);
			for (int i = 0; i < outputAddresses.size(); i++) {
				const auto outputAddress = outputAddresses[i];
				const auto inputAddress = inputAddresses[i];
				const block_id_t outputBlockId = addressTree.getValue(outputAddress);
				const block_id_t inputBlockId = addressTree.getValue(inputAddress);
				logicSimulator.disconnectGates(outputBlockId, inputBlockId);
			}
			break;
		}
		case Difference::CREATED_CONNECTION:
		{
			const auto& [outputPosition, inputPosition] = std::get<Difference::connection_modification_t>(modificationData);
			const auto outputAddresses = addressTree.getPositions(containerId, outputPosition);
			const auto inputAddresses = addressTree.getPositions(containerId, inputPosition);
			for (int i = 0; i < outputAddresses.size(); i++) {
				const auto outputAddress = outputAddresses[i];
				const auto inputAddress = inputAddresses[i];
				const block_id_t outputBlockId = addressTree.getValue(outputAddress);
				const block_id_t inputBlockId = addressTree.getValue(inputAddress);
				logicSimulator.connectGates(outputBlockId, inputBlockId);
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
	if (deletedBlocks) {
		const auto gateMap = logicSimulator.compressGates();
		addressTree.remap(gateMap);
	}
	if (!paused) {
		logicSimulator.signalToProceed();
	}
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
	case BlockType::LIGHT: return GateType::OR;
	default: return GateType::NONE;
	}
}

logic_state_t Evaluator::getState(const Address& address) {
	const block_id_t blockId = addressTree.getValue(address);

	logicSimulator.signalToPause();
	while (!logicSimulator.threadIsWaiting()) {
		std::this_thread::yield();
	}
	const logic_state_t state = logicSimulator.getState(blockId);
	if (!paused) {
		logicSimulator.signalToProceed();
	}
	return state;
}

std::vector<logic_state_t> Evaluator::getBulkStates(const std::vector<Address>& addresses) {
	std::vector<logic_state_t> states;
	states.reserve(addresses.size());
	logicSimulator.signalToPause();
	while (!logicSimulator.threadIsWaiting()) {
		std::this_thread::yield();
	}
	for (const auto& address : addresses) {
		const block_id_t blockId = addressTree.getValue(address);
		states.push_back(logicSimulator.getState(blockId));
	}
	if (!paused) {
		logicSimulator.signalToProceed();
	}
	return states;
}

void Evaluator::setState(const Address& address, logic_state_t state) {
	const block_id_t blockId = addressTree.getValue(address);
	logicSimulator.signalToPause();
	while (!logicSimulator.threadIsWaiting()) {
		std::this_thread::yield();
	}
	logicSimulator.setState(blockId, state);
	if (!paused) {
		logicSimulator.signalToProceed();
	}
}
