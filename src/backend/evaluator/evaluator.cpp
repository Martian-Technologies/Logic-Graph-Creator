#include "evaluator.h"

Evaluator::Evaluator(evaluator_id_t evaluatorId, SharedCircuit circuit)
	: evaluatorId(evaluatorId), paused(true),
	targetTickrate(0),
	logicSimulator(),
	addressTree(circuit->getCircuitId()),
	usingTickrate(false) {
	setTickrate(40);
	const auto blockContainer = circuit->getBlockContainer();
	const Difference difference = blockContainer->getCreationDifference();

	makeEdit(std::make_shared<Difference>(difference), circuit->getCircuitId());

	// connect makeEdit to circuit
	circuit->connectListener(this, std::bind(&Evaluator::makeEdit, this, std::placeholders::_1, std::placeholders::_2));
}

logic_state_t Evaluator::getState(const Address& address) {
	logicSimulator.signalToPause();
	const output_socket_id_t outputSocket = outputSocketTree.getValue(address);
	logicSimulator.waitForPause();
	const logic_state_t state = logicSimulator.getOutputSocketState(outputSocket);
	if (!paused) {
		logicSimulator.signalToResume();
	}
	return state;
}

std::vector<logic_state_t> Evaluator::getBulkStates(const std::vector<Address>& addresses) {
	logicSimulator.signalToPause();
	std::vector<output_socket_id_t> outputSockets;
	std::vector<logic_state_t> states;
	outputSockets.reserve(addresses.size());
	states.reserve(addresses.size());
	for (const Address& address : addresses) {
		const eval_gate_id_t outputSocket = outputSocketTree.getValue(address);
		outputSockets.push_back(outputSocket);
	}
	logicSimulator.waitForPause();
	for (const output_socket_id_t outputSocket : outputSockets) {
		states.push_back(logicSimulator.getOutputSocketState(outputSocket));
	}
	if (!paused) {
		logicSimulator.signalToResume();
	}
	return states;
}

void Evaluator::setTickrate(double tickrate) {
	targetTickrate = tickrate;
	logicSimulator.setTickrate(tickrate);
}
void Evaluator::setUseTickrate(bool useTickrate) {
	usingTickrate = useTickrate;
	logicSimulator.setSprint(!useTickrate);
}
double Evaluator::getRealTickrate() const { return 0; }

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
	default:
		throw std::invalid_argument("circuitToEvaluatorGatetype: invalid blockType");
	}
}

void Evaluator::makeEdit(DifferenceSharedPtr difference, circuit_id_t containerId) {
	logicSimulator.signalToPause();
	logicSimulator.waitForPause();
	const auto modifications = difference->getModifications();
	bool deletedBlocks = false;
	for (const auto & modification : modifications) {
		const auto& [modificationType, modificationData] = modification;
		switch (modificationType) {
		// case Difference::REMOVED_BLOCK:
		// {
		// 	deletedBlocks = true;
		// 	const auto& [position, rotation, blockType] = std::get<Difference::block_modification_t>(modificationData);
		// 	const auto address = Address(position);
		// 	const eval_gate_id_t gateId = gateTree.getValue(address);
		// 	const auto inputSockets = logicSimulator.getGate(gateId).getInputSockets();
		// 	const auto outputSockets = logicSimulator.getGate(gateId).getOutputSockets();
		// 	gateTree.removeValueAt(address);
		// 	for (const auto& inputSocket : inputSockets) {
		// 		inputSocketTree.removeValue(inputSocket);
		// 	}
		// 	for (const auto& outputSocket : outputSockets) {
		// 		outputSocketTree.removeValue(outputSocket);
		// 	}
		// 	logicSimulator.decomissionGate(gateId);
		// 	// NOT DONE

		// 	break;
		// }
		case Difference::PLACE_BLOCK:
		{
			const auto& [position, rotation, blockType] = std::get<Difference::block_modification_t>(modificationData);
			const auto address = Address(position);
			const GateType gateType = circuitToEvaluatorGatetype(blockType);
			const input_socket_id_t inputSocket = logicSimulator.registerInputSocket();
			const output_socket_id_t outputSocket = logicSimulator.registerOutputSocket();
			const eval_gate_id_t gateId = logicSimulator.registerGate(inputSocket, outputSocket, gateType);
			gateTree.addValue(address, gateId);
			inputSocketTree.addValue(address, inputSocket);
			outputSocketTree.addValue(address, outputSocket);
			break;
		}
		case Difference::CREATED_CONNECTION:
		{
			const auto& [source, destination] = std::get<Difference::connection_modification_t>(modificationData);
			const input_socket_id_t inputSocket = inputSocketTree.getValue(Address(destination));
			const output_socket_id_t outputSocket = outputSocketTree.getValue(Address(source));
			logicSimulator.connect(outputSocket, inputSocket);
			break;
		}
		}
	}
	// if (deletedBlocks) {
	// 	const auto gateMap = logicSimulator.compressGates();
	// 	gateTree.remap(gateMap);
	// }
	if (!paused) {
		logicSimulator.signalToResume();
	}
}
void Evaluator::setPause(bool pause) {
	paused = pause;
	if (paused) {
		logicSimulator.signalToPause();
	} else {
		logicSimulator.signalToResume();
	}
}
void Evaluator::setState(const Address& address, logic_state_t state) {
	logicSimulator.signalToPause();
	const output_socket_id_t outputSocket = outputSocketTree.getValue(address);
	logicSimulator.waitForPause();
	logicSimulator.setOutputSocketState(outputSocket, state);
	if (!paused) {
		logicSimulator.signalToResume();
	}
}