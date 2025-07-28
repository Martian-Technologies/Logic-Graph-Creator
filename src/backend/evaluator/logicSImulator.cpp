#include "logicSimulator.h"
#include "gateType.h"

LogicSimulator::LogicSimulator() {
	simulationThread = std::jthread(&LogicSimulator::simulationLoop, this);
}

LogicSimulator::~LogicSimulator() {
	{
		std::lock_guard<std::mutex> lk(cvMutex);
		running = false;
		cv.notify_all();
	}
}

void LogicSimulator::simulationLoop()
{
	using clock = std::chrono::steady_clock;
	auto nextTick = clock::now();

	while (running) {
		if (pauseRequest.load(std::memory_order_acquire))
		{
			std::unique_lock<std::mutex> lk(cvMutex);
			isPaused.store(true, std::memory_order_release);
			cv.notify_all();                           // acknowledge
			cv.wait(lk, [&]{ return !pauseRequest || !running; });
			isPaused.store(false, std::memory_order_release);
			if (!running) break;
		}
		tickOnce();
		auto period = std::chrono::round<std::chrono::nanoseconds>(std::chrono::minutes{1} / targetTickrate.load(std::memory_order_relaxed));

		nextTick += period;
		std::unique_lock lk(cvMutex);
		cv.wait_until(lk, nextTick, [&]{ return pauseRequest || !running; });
	}
}

inline void LogicSimulator::tickOnce() {
	std::unique_lock lkNext(statesBMutex);
	{
		std::shared_lock lkCur(statesAMutex);
		for (auto& gate : andGates) gate.tick(statesA, statesB);
		for (auto& gate : xorGates) gate.tick(statesA, statesB);
		for (auto& gate : constantResetGates) gate.tick(statesB);
		for (auto& gate : copySelfOutputGates) gate.tick(statesA, statesB);
		for (auto& gate : tristateBuffers) gate.tick(statesA, statesB);

		// junctions are special because they need to act instantly, so they run at the end of the tick
		for (auto& gate : junctions) gate.tick(statesB);
	}
	std::unique_lock lkCurEx(statesAMutex);
	std::swap(statesA, statesB);
}

void LogicSimulator::setState(simulator_id_t id, logic_state_t st) {
	std::scoped_lock lk(statesBMutex, statesAMutex);
	statesA[id] = st;
	statesB[id] = st;
}

void LogicSimulator::setStates(const std::vector<simulator_id_t>& ids, const std::vector<logic_state_t>& states) {
	if (ids.size() != states.size()) {
		throw std::invalid_argument("ids and states must have the same size");
	}
	std::scoped_lock lk(statesBMutex, statesAMutex);
	for (size_t i = 0; i < ids.size(); ++i) {
		statesA[ids[i]] = states[i];
		statesB[ids[i]] = states[i];
	}
}

logic_state_t LogicSimulator::getState(simulator_id_t id) const {
	std::shared_lock lk(statesAMutex);
	return statesA[id];
}

std::vector<logic_state_t> LogicSimulator::getStates(const std::vector<simulator_id_t>& ids) const {
	std::vector<logic_state_t> result(ids.size());
	std::shared_lock lk(statesAMutex);
	for (size_t i = 0; i < ids.size(); ++i) {
		result[i] = statesA[ids[i]];
	}
	return result;
}

// Gate management methods (moved from SimulatorOptimizer)
simulator_id_t LogicSimulator::addGate(const GateType gateType) {
	simulator_id_t simulatorId = simulatorIdProvider.getNewId();

	logInfo("Adding gate of type {} with simulator_id_t {}", "LogicSimulator::addGate", static_cast<int>(gateType), simulatorId);

	// extend the states if necessary
	if (statesA.size() <= simulatorId) {
		statesA.resize(simulatorId + 1, logic_state_t::UNDEFINED);
		statesB.resize(simulatorId + 1, logic_state_t::UNDEFINED);
	}
	statesA[simulatorId] = logic_state_t::UNDEFINED;
	statesB[simulatorId] = logic_state_t::UNDEFINED;

	switch (gateType) {
	case GateType::AND:
		andGates.push_back({ simulatorId, false, false });
		break;
	case GateType::OR:
		andGates.push_back({ simulatorId, true, true });
		break;
	case GateType::NAND:
		andGates.push_back({ simulatorId, false, true });
		break;
	case GateType::NOR:
		andGates.push_back({ simulatorId, true, false });
		break;
	case GateType::XOR:
		xorGates.push_back({ simulatorId, false });
		break;
	case GateType::XNOR:
		xorGates.push_back({ simulatorId, true });
		break;
	case GateType::JUNCTION:
		junctions.push_back({ simulatorId });
		break;
	case GateType::TRISTATE_BUFFER:
		tristateBuffers.push_back({ simulatorId, false });
		break;
	case GateType::TRISTATE_BUFFER_INVERTED:
		tristateBuffers.push_back({ simulatorId, true });
		break;
	case GateType::CONSTANT_OFF:
		constantGates.push_back({ simulatorId, logic_state_t::LOW });
		statesA[simulatorId] = logic_state_t::LOW;
		statesB[simulatorId] = logic_state_t::LOW;
		break;
	case GateType::CONSTANT_ON:
		constantGates.push_back({ simulatorId, logic_state_t::HIGH });
		statesA[simulatorId] = logic_state_t::HIGH;
		statesB[simulatorId] = logic_state_t::HIGH;
		break;
	case GateType::DUMMY_INPUT:
		copySelfOutputGates.push_back({ simulatorId });
		statesA[simulatorId] = logic_state_t::LOW;
		statesB[simulatorId] = logic_state_t::LOW;
		break;
	case GateType::THROUGH:
		singleBuffers.push_back({ simulatorId, false });
		break;
	case GateType::TICK_INPUT:
		constantResetGates.push_back({ simulatorId, logic_state_t::LOW });
		statesA[simulatorId] = logic_state_t::LOW;
		statesB[simulatorId] = logic_state_t::LOW;
		break;
	case GateType::NONE:
		logError("Cannot add gate of type NONE", "LogicSimulator::addGate");
		break;
	}
	return simulatorId;
}

void LogicSimulator::removeGate(simulator_id_t simulatorId) {
	std::optional<std::vector<simulator_id_t>> outputIdsOpt = getOutputSimIdsFromGate(simulatorId);
	if (!outputIdsOpt.has_value()) {
		logError("Cannot remove gate: no output IDs found for simulator_id_t " + std::to_string(simulatorId), "LogicSimulator::removeGate");
		return;
	}

	logInfo("Removing gate with simulator_id_t {}", "LogicSimulator::removeGate", simulatorId);

	// First, remove all references to this gate's outputs from other gates
	for (const auto& id : outputIdsOpt.value()) {
		logInfo("Removing all references to simulator_id_t {}", "LogicSimulator::removeGate", id);
		for (auto& gate : andGates) {
			gate.removeIdRefs(id);
		}
		for (auto& gate : xorGates) {
			gate.removeIdRefs(id);
		}
		for (auto& gate : junctions) {
			gate.removeIdRefs(id);
		}
		for (auto& gate : buffers) {
			gate.removeIdRefs(id);
		}
		for (auto& gate : singleBuffers) {
			gate.removeIdRefs(id);
		}
		for (auto& gate : tristateBuffers) {
			gate.removeIdRefs(id);
		}
		for (auto& gate : constantGates) {
			gate.removeIdRefs(id);
		}
		for (auto& gate : constantResetGates) {
			gate.removeIdRefs(id);
		}
		for (auto& gate : copySelfOutputGates) {
			gate.removeIdRefs(id);
		}
		simulatorIdProvider.releaseId(id);
	}

	// Now remove the gate itself from the appropriate gate vector
	andGates.erase(std::remove_if(andGates.begin(), andGates.end(),
		[simulatorId](const ANDLikeGate& gate) { return gate.getId() == simulatorId; }), andGates.end());

	xorGates.erase(std::remove_if(xorGates.begin(), xorGates.end(),
		[simulatorId](const XORLikeGate& gate) { return gate.getId() == simulatorId; }), xorGates.end());

	junctions.erase(std::remove_if(junctions.begin(), junctions.end(),
		[simulatorId](const JunctionGate& gate) { return gate.getId() == simulatorId; }), junctions.end());

	buffers.erase(std::remove_if(buffers.begin(), buffers.end(),
		[simulatorId](const BufferGate& gate) { return gate.getId() == simulatorId; }), buffers.end());

	singleBuffers.erase(std::remove_if(singleBuffers.begin(), singleBuffers.end(),
		[simulatorId](const SingleBufferGate& gate) { return gate.getId() == simulatorId; }), singleBuffers.end());

	tristateBuffers.erase(std::remove_if(tristateBuffers.begin(), tristateBuffers.end(),
		[simulatorId](const TristateBufferGate& gate) { return gate.getId() == simulatorId; }), tristateBuffers.end());

	constantGates.erase(std::remove_if(constantGates.begin(), constantGates.end(),
		[simulatorId](const ConstantGate& gate) { return gate.getId() == simulatorId; }), constantGates.end());

	constantResetGates.erase(std::remove_if(constantResetGates.begin(), constantResetGates.end(),
		[simulatorId](const ConstantResetGate& gate) { return gate.getId() == simulatorId; }), constantResetGates.end());

	copySelfOutputGates.erase(std::remove_if(copySelfOutputGates.begin(), copySelfOutputGates.end(),
		[simulatorId](const CopySelfOutputGate& gate) { return gate.getId() == simulatorId; }), copySelfOutputGates.end());
}

void LogicSimulator::makeConnection(simulator_id_t sourceId, connection_port_id_t sourcePort, simulator_id_t destinationId, connection_port_id_t destinationPort) {
	logInfo("Making connection from simulator_id_t {} port {} to simulator_id_t {} port {}", "LogicSimulator::makeConnection",
		sourceId, sourcePort, destinationId, destinationPort);

	std::optional<simulator_id_t> actualSourceId = getOutputPortId(sourceId, sourcePort);

	if (!actualSourceId.has_value()) {
		logError("Cannot resolve actual source ID for connection", "LogicSimulator::makeConnection");
		return;
	}

	logInfo("Source simulator_id_t {} port {} resolved to actual source ID {}", "LogicSimulator::makeConnection",
		sourceId, sourcePort, actualSourceId.value());

	addInputToGate(destinationId, actualSourceId.value(), destinationPort);
}

void LogicSimulator::removeConnection(simulator_id_t sourceId, connection_port_id_t sourcePort, simulator_id_t destinationId, connection_port_id_t destinationPort) {
	removeInputFromGate(destinationId, sourceId, destinationPort);
}

std::optional<simulator_id_t> LogicSimulator::getOutputPortId(simulator_id_t simId, connection_port_id_t portId) const {
	auto andIt = std::find_if(andGates.begin(), andGates.end(),
		[simId](const ANDLikeGate& gate) { return gate.getId() == simId; });
	if (andIt != andGates.end()) {
		return andIt->getIdOfOutputPort(portId);
	}

	auto xorIt = std::find_if(xorGates.begin(), xorGates.end(),
		[simId](const XORLikeGate& gate) { return gate.getId() == simId; });
	if (xorIt != xorGates.end()) {
		return xorIt->getIdOfOutputPort(portId);
	}

	auto junctionIt = std::find_if(junctions.begin(), junctions.end(),
		[simId](const JunctionGate& gate) { return gate.getId() == simId; });
	if (junctionIt != junctions.end()) {
		return junctionIt->getIdOfOutputPort(portId);
	}

	auto bufferIt = std::find_if(buffers.begin(), buffers.end(),
		[simId](const BufferGate& gate) { return gate.getId() == simId; });
	if (bufferIt != buffers.end()) {
		return bufferIt->getIdOfOutputPort(portId);
	}

	auto singleBufferIt = std::find_if(singleBuffers.begin(), singleBuffers.end(),
		[simId](const SingleBufferGate& gate) { return gate.getId() == simId; });
	if (singleBufferIt != singleBuffers.end()) {
		return singleBufferIt->getIdOfOutputPort(portId);
	}

	auto tristateIt = std::find_if(tristateBuffers.begin(), tristateBuffers.end(),
		[simId](const TristateBufferGate& gate) { return gate.getId() == simId; });
	if (tristateIt != tristateBuffers.end()) {
		return tristateIt->getIdOfOutputPort(portId);
	}

	auto constantIt = std::find_if(constantGates.begin(), constantGates.end(),
		[simId](const ConstantGate& gate) { return gate.getId() == simId; });
	if (constantIt != constantGates.end()) {
		return constantIt->getIdOfOutputPort(portId);
	}

	auto constantResetIt = std::find_if(constantResetGates.begin(), constantResetGates.end(),
		[simId](const ConstantResetGate& gate) { return gate.getId() == simId; });
	if (constantResetIt != constantResetGates.end()) {
		return constantResetIt->getIdOfOutputPort(portId);
	}

	auto copySelfIt = std::find_if(copySelfOutputGates.begin(), copySelfOutputGates.end(),
		[simId](const CopySelfOutputGate& gate) { return gate.getId() == simId; });
	if (copySelfIt != copySelfOutputGates.end()) {
		return copySelfIt->getIdOfOutputPort(portId);
	}

	return std::nullopt;
}

void LogicSimulator::addInputToGate(simulator_id_t simId, simulator_id_t inputId, connection_port_id_t portId) {
	auto andIt = std::find_if(andGates.begin(), andGates.end(),
		[simId](ANDLikeGate& gate) { return gate.getId() == simId; });
	if (andIt != andGates.end()) {
		andIt->addInput(inputId, portId);
		return;
	}

	auto xorIt = std::find_if(xorGates.begin(), xorGates.end(),
		[simId](XORLikeGate& gate) { return gate.getId() == simId; });
	if (xorIt != xorGates.end()) {
		xorIt->addInput(inputId, portId);
		return;
	}

	auto junctionIt = std::find_if(junctions.begin(), junctions.end(),
		[simId](JunctionGate& gate) { return gate.getId() == simId; });
	if (junctionIt != junctions.end()) {
		junctionIt->addInput(inputId, portId);
		return;
	}

	auto bufferIt = std::find_if(buffers.begin(), buffers.end(),
		[simId](BufferGate& gate) { return gate.getId() == simId; });
	if (bufferIt != buffers.end()) {
		bufferIt->addInput(inputId, portId);
		return;
	}

	auto singleBufferIt = std::find_if(singleBuffers.begin(), singleBuffers.end(),
		[simId](SingleBufferGate& gate) { return gate.getId() == simId; });
	if (singleBufferIt != singleBuffers.end()) {
		singleBufferIt->addInput(inputId, portId);
		return;
	}

	auto tristateIt = std::find_if(tristateBuffers.begin(), tristateBuffers.end(),
		[simId](TristateBufferGate& gate) { return gate.getId() == simId; });
	if (tristateIt != tristateBuffers.end()) {
		tristateIt->addInput(inputId, portId);
		return;
	}

	auto constantIt = std::find_if(constantGates.begin(), constantGates.end(),
		[simId](ConstantGate& gate) { return gate.getId() == simId; });
	if (constantIt != constantGates.end()) {
		constantIt->addInput(inputId, portId);
		return;
	}

	auto constantResetIt = std::find_if(constantResetGates.begin(), constantResetGates.end(),
		[simId](ConstantResetGate& gate) { return gate.getId() == simId; });
	if (constantResetIt != constantResetGates.end()) {
		constantResetIt->addInput(inputId, portId);
		return;
	}

	auto copySelfIt = std::find_if(copySelfOutputGates.begin(), copySelfOutputGates.end(),
		[simId](CopySelfOutputGate& gate) { return gate.getId() == simId; });
	if (copySelfIt != copySelfOutputGates.end()) {
		copySelfIt->addInput(inputId, portId);
		return;
	}

	logError("Gate not found for addInputToGate", "LogicSimulator::addInputToGate");
}

void LogicSimulator::removeInputFromGate(simulator_id_t simId, simulator_id_t inputId, connection_port_id_t portId) {
	auto andIt = std::find_if(andGates.begin(), andGates.end(),
		[simId](ANDLikeGate& gate) { return gate.getId() == simId; });
	if (andIt != andGates.end()) {
		andIt->removeInput(inputId, portId);
		return;
	}

	auto xorIt = std::find_if(xorGates.begin(), xorGates.end(),
		[simId](XORLikeGate& gate) { return gate.getId() == simId; });
	if (xorIt != xorGates.end()) {
		xorIt->removeInput(inputId, portId);
		return;
	}

	auto junctionIt = std::find_if(junctions.begin(), junctions.end(),
		[simId](JunctionGate& gate) { return gate.getId() == simId; });
	if (junctionIt != junctions.end()) {
		junctionIt->removeInput(inputId, portId);
		return;
	}

	auto bufferIt = std::find_if(buffers.begin(), buffers.end(),
		[simId](BufferGate& gate) { return gate.getId() == simId; });
	if (bufferIt != buffers.end()) {
		bufferIt->removeInput(inputId, portId);
		return;
	}

	auto singleBufferIt = std::find_if(singleBuffers.begin(), singleBuffers.end(),
		[simId](SingleBufferGate& gate) { return gate.getId() == simId; });
	if (singleBufferIt != singleBuffers.end()) {
		singleBufferIt->removeInput(inputId, portId);
		return;
	}

	auto tristateIt = std::find_if(tristateBuffers.begin(), tristateBuffers.end(),
		[simId](TristateBufferGate& gate) { return gate.getId() == simId; });
	if (tristateIt != tristateBuffers.end()) {
		tristateIt->removeInput(inputId, portId);
		return;
	}

	auto constantIt = std::find_if(constantGates.begin(), constantGates.end(),
		[simId](ConstantGate& gate) { return gate.getId() == simId; });
	if (constantIt != constantGates.end()) {
		constantIt->removeInput(inputId, portId);
		return;
	}

	auto constantResetIt = std::find_if(constantResetGates.begin(), constantResetGates.end(),
		[simId](ConstantResetGate& gate) { return gate.getId() == simId; });
	if (constantResetIt != constantResetGates.end()) {
		constantResetIt->removeInput(inputId, portId);
		return;
	}

	auto copySelfIt = std::find_if(copySelfOutputGates.begin(), copySelfOutputGates.end(),
		[simId](CopySelfOutputGate& gate) { return gate.getId() == simId; });
	if (copySelfIt != copySelfOutputGates.end()) {
		copySelfIt->removeInput(inputId, portId);
		return;
	}

	logError("Gate not found for removeInputFromGate", "LogicSimulator::removeInputFromGate");
}

std::optional<std::vector<simulator_id_t>> LogicSimulator::getOutputSimIdsFromGate(simulator_id_t simId) const {
	auto andIt = std::find_if(andGates.begin(), andGates.end(),
		[simId](const ANDLikeGate& gate) { return gate.getId() == simId; });
	if (andIt != andGates.end()) {
		return andIt->getOutputSimIds();
	}

	auto xorIt = std::find_if(xorGates.begin(), xorGates.end(),
		[simId](const XORLikeGate& gate) { return gate.getId() == simId; });
	if (xorIt != xorGates.end()) {
		return xorIt->getOutputSimIds();
	}

	auto junctionIt = std::find_if(junctions.begin(), junctions.end(),
		[simId](const JunctionGate& gate) { return gate.getId() == simId; });
	if (junctionIt != junctions.end()) {
		return junctionIt->getOutputSimIds();
	}

	auto bufferIt = std::find_if(buffers.begin(), buffers.end(),
		[simId](const BufferGate& gate) { return gate.getId() == simId; });
	if (bufferIt != buffers.end()) {
		return bufferIt->getOutputSimIds();
	}

	auto singleBufferIt = std::find_if(singleBuffers.begin(), singleBuffers.end(),
		[simId](const SingleBufferGate& gate) { return gate.getId() == simId; });
	if (singleBufferIt != singleBuffers.end()) {
		return singleBufferIt->getOutputSimIds();
	}

	auto tristateIt = std::find_if(tristateBuffers.begin(), tristateBuffers.end(),
		[simId](const TristateBufferGate& gate) { return gate.getId() == simId; });
	if (tristateIt != tristateBuffers.end()) {
		return tristateIt->getOutputSimIds();
	}

	auto constantIt = std::find_if(constantGates.begin(), constantGates.end(),
		[simId](const ConstantGate& gate) { return gate.getId() == simId; });
	if (constantIt != constantGates.end()) {
		return constantIt->getOutputSimIds();
	}

	return std::nullopt;
}