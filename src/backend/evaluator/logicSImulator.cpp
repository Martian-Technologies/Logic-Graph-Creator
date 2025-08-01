#include "logicSimulator.h"
#include "gateType.h"
#ifdef TRACY_PROFILER
	#include <tracy/Tracy.hpp>
#endif

LogicSimulator::LogicSimulator(EvalConfig& evalConfig) : evalConfig(evalConfig) {
	// Subscribe to EvalConfig changes to update the simulator accordingly
	evalConfig.subscribe([this]() {
		// Notify the simulation thread about config changes
		std::lock_guard<std::mutex> lk(cvMutex);
		cv.notify_all();
	});

	simulationThread = std::jthread(&LogicSimulator::simulationLoop, this);
	statesA.resize(1, logic_state_t::UNDEFINED);
	statesB.resize(1, logic_state_t::UNDEFINED);
}

LogicSimulator::~LogicSimulator() {
	{
		std::lock_guard<std::mutex> lk(cvMutex);
		running = false;
		cv.notify_all();
	}
}

void LogicSimulator::clearState() {}

unsigned int LogicSimulator::getAverageTickrate() const {
	// TODO: Implement a proper tickrate calculation
	long long targetTickrate = evalConfig.getTargetTickrate();
	return static_cast<unsigned int>(targetTickrate > 0 ? targetTickrate : 0);
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
			cv.notify_all();
			cv.wait(lk, [&]{ return !pauseRequest || !running; });
			isPaused.store(false, std::memory_order_release);
			if (!running) break;
			// reset nextTick after resuming from pause
			nextTick = clock::now();
		}

		if (evalConfig.isRunning()) {
			tickOnce();

			// handle timing after ticking
			if (evalConfig.isTickrateLimiterEnabled()) {
				long long targetTickrate = evalConfig.getTargetTickrate();
				if (targetTickrate > 0) {
					auto period = std::chrono::round<std::chrono::nanoseconds>(std::chrono::minutes { 1 }) / targetTickrate;
					nextTick += period;
					std::unique_lock lk(cvMutex);
					cv.wait_until(lk, nextTick, [&]{ return pauseRequest || !running || !evalConfig.isRunning(); });
				}
			}
		} else {
			// simulation is paused, so we wait for state change
			std::unique_lock lk(cvMutex);
			cv.wait(lk, [&]{ return pauseRequest || !running || evalConfig.isRunning(); });
			// reset nextTick when resuming simulation
			nextTick = clock::now();
		}
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
		updateGateLocation(simulatorId, SimGateType::AND, andGates.size() - 1);
		break;
	case GateType::OR:
		andGates.push_back({ simulatorId, true, true });
		updateGateLocation(simulatorId, SimGateType::AND, andGates.size() - 1);
		break;
	case GateType::NAND:
		andGates.push_back({ simulatorId, false, true });
		updateGateLocation(simulatorId, SimGateType::AND, andGates.size() - 1);
		break;
	case GateType::NOR:
		andGates.push_back({ simulatorId, true, false });
		updateGateLocation(simulatorId, SimGateType::AND, andGates.size() - 1);
		break;
	case GateType::XOR:
		xorGates.push_back({ simulatorId, false });
		updateGateLocation(simulatorId, SimGateType::XOR, xorGates.size() - 1);
		break;
	case GateType::XNOR:
		xorGates.push_back({ simulatorId, true });
		updateGateLocation(simulatorId, SimGateType::XOR, xorGates.size() - 1);
		break;
	case GateType::JUNCTION:
		junctions.push_back({ simulatorId });
		updateGateLocation(simulatorId, SimGateType::JUNCTION, junctions.size() - 1);
		break;
	case GateType::TRISTATE_BUFFER:
		tristateBuffers.push_back({ simulatorId, false });
		updateGateLocation(simulatorId, SimGateType::TRISTATE_BUFFER, tristateBuffers.size() - 1);
		break;
	case GateType::TRISTATE_BUFFER_INVERTED:
		tristateBuffers.push_back({ simulatorId, true });
		updateGateLocation(simulatorId, SimGateType::TRISTATE_BUFFER, tristateBuffers.size() - 1);
		break;
	case GateType::CONSTANT_OFF:
		constantGates.push_back({ simulatorId, logic_state_t::LOW });
		updateGateLocation(simulatorId, SimGateType::CONSTANT, constantGates.size() - 1);
		statesA[simulatorId] = logic_state_t::LOW;
		statesB[simulatorId] = logic_state_t::LOW;
		break;
	case GateType::CONSTANT_ON:
		constantGates.push_back({ simulatorId, logic_state_t::HIGH });
		updateGateLocation(simulatorId, SimGateType::CONSTANT, constantGates.size() - 1);
		statesA[simulatorId] = logic_state_t::HIGH;
		statesB[simulatorId] = logic_state_t::HIGH;
		break;
	case GateType::DUMMY_INPUT:
		copySelfOutputGates.push_back({ simulatorId });
		updateGateLocation(simulatorId, SimGateType::COPY_SELF_OUTPUT, copySelfOutputGates.size() - 1);
		statesA[simulatorId] = logic_state_t::LOW;
		statesB[simulatorId] = logic_state_t::LOW;
		break;
	case GateType::THROUGH:
		singleBuffers.push_back({ simulatorId, false });
		updateGateLocation(simulatorId, SimGateType::SINGLE_BUFFER, singleBuffers.size() - 1);
		break;
	case GateType::TICK_INPUT:
		constantResetGates.push_back({ simulatorId, logic_state_t::LOW });
		updateGateLocation(simulatorId, SimGateType::CONSTANT_RESET, constantResetGates.size() - 1);
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
	#ifdef TRACY_PROFILER
		ZoneScoped;
	#endif
	std::optional<std::vector<simulator_id_t>> outputIdsOpt = getOutputSimIdsFromGate(simulatorId);
	if (!outputIdsOpt.has_value()) {
		logError("Cannot remove gate: no output IDs found for simulator_id_t " + std::to_string(simulatorId), "LogicSimulator::removeGate");
		return;
	}

	// First, remove all references to this gate's outputs from other gates - OPTIMIZED VERSION
	{
		#ifdef TRACY_PROFILER
			ZoneScopedN("Removing references to gate outputs");
		#endif
		for (const auto& id : outputIdsOpt.value()) {
			auto depIt = outputDependencies.find(id);
			if (depIt != outputDependencies.end()) {
				for (const auto& [gateType, gateIndex] : depIt->second) {
					// Remove the reference from the specific gate
					switch (gateType) {
					case SimGateType::AND:
						if (gateIndex < andGates.size()) {
							andGates[gateIndex].removeIdRefs(id);
						}
						break;
					case SimGateType::XOR:
						if (gateIndex < xorGates.size()) {
							xorGates[gateIndex].removeIdRefs(id);
						}
						break;
					case SimGateType::JUNCTION:
						if (gateIndex < junctions.size()) {
							junctions[gateIndex].removeIdRefs(id);
						}
						break;
					case SimGateType::BUFFER:
						if (gateIndex < buffers.size()) {
							buffers[gateIndex].removeIdRefs(id);
						}
						break;
					case SimGateType::SINGLE_BUFFER:
						if (gateIndex < singleBuffers.size()) {
							singleBuffers[gateIndex].removeIdRefs(id);
						}
						break;
					case SimGateType::TRISTATE_BUFFER:
						if (gateIndex < tristateBuffers.size()) {
							tristateBuffers[gateIndex].removeIdRefs(id);
						}
						break;
					case SimGateType::CONSTANT:
						if (gateIndex < constantGates.size()) {
							constantGates[gateIndex].removeIdRefs(id);
						}
						break;
					case SimGateType::CONSTANT_RESET:
						if (gateIndex < constantResetGates.size()) {
							constantResetGates[gateIndex].removeIdRefs(id);
						}
						break;
					case SimGateType::COPY_SELF_OUTPUT:
						if (gateIndex < copySelfOutputGates.size()) {
							copySelfOutputGates[gateIndex].removeIdRefs(id);
						}
						break;
					}
				}
				outputDependencies.erase(depIt);
			}
			simulatorIdProvider.releaseId(id);
		}
	}

	{
		#ifdef TRACY_PROFILER
			ZoneScopedN("Removing gate from simulator");
		#endif
		// Find and remove the gate using the optimized lookup - OPTIMIZED VERSION
		auto locationIt = gateLocations.find(simulatorId);
		if (locationIt != gateLocations.end()) {
			SimGateType gateType = locationIt->second.first;
			size_t gateIndex = locationIt->second.second;
			
			switch (gateType) {
			case SimGateType::AND:
				if (gateIndex < andGates.size()) {
					andGates.erase(andGates.begin() + gateIndex);
					updateGateIndicesAfterRemoval(SimGateType::AND, gateIndex);
				}
				break;
			case SimGateType::XOR:
				if (gateIndex < xorGates.size()) {
					xorGates.erase(xorGates.begin() + gateIndex);
					updateGateIndicesAfterRemoval(SimGateType::XOR, gateIndex);
				}
				break;
			case SimGateType::JUNCTION:
				if (gateIndex < junctions.size()) {
					junctions.erase(junctions.begin() + gateIndex);
					updateGateIndicesAfterRemoval(SimGateType::JUNCTION, gateIndex);
				}
				break;
			case SimGateType::BUFFER:
				if (gateIndex < buffers.size()) {
					buffers.erase(buffers.begin() + gateIndex);
					updateGateIndicesAfterRemoval(SimGateType::BUFFER, gateIndex);
				}
				break;
			case SimGateType::SINGLE_BUFFER:
				if (gateIndex < singleBuffers.size()) {
					singleBuffers.erase(singleBuffers.begin() + gateIndex);
					updateGateIndicesAfterRemoval(SimGateType::SINGLE_BUFFER, gateIndex);
				}
				break;
			case SimGateType::TRISTATE_BUFFER:
				if (gateIndex < tristateBuffers.size()) {
					tristateBuffers.erase(tristateBuffers.begin() + gateIndex);
					updateGateIndicesAfterRemoval(SimGateType::TRISTATE_BUFFER, gateIndex);
				}
				break;
			case SimGateType::CONSTANT:
				if (gateIndex < constantGates.size()) {
					constantGates.erase(constantGates.begin() + gateIndex);
					updateGateIndicesAfterRemoval(SimGateType::CONSTANT, gateIndex);
				}
				break;
			case SimGateType::CONSTANT_RESET:
				if (gateIndex < constantResetGates.size()) {
					constantResetGates.erase(constantResetGates.begin() + gateIndex);
					updateGateIndicesAfterRemoval(SimGateType::CONSTANT_RESET, gateIndex);
				}
				break;
			case SimGateType::COPY_SELF_OUTPUT:
				if (gateIndex < copySelfOutputGates.size()) {
					copySelfOutputGates.erase(copySelfOutputGates.begin() + gateIndex);
					updateGateIndicesAfterRemoval(SimGateType::COPY_SELF_OUTPUT, gateIndex);
				}
				break;
			}
			
			removeGateLocation(simulatorId);
		}
	}
}

void LogicSimulator::makeConnection(simulator_id_t sourceId, connection_port_id_t sourcePort, simulator_id_t destinationId, connection_port_id_t destinationPort) {
	std::optional<simulator_id_t> actualSourceId = getOutputPortId(sourceId, sourcePort);

	if (!actualSourceId.has_value()) {
		logError("Cannot resolve actual source ID for connection", "LogicSimulator::makeConnection");
		return;
	}

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
	// Use optimized lookup instead of linear search
	auto locationIt = gateLocations.find(simId);
	if (locationIt != gateLocations.end()) {
		SimGateType gateType = locationIt->second.first;
		size_t gateIndex = locationIt->second.second;
		
		switch (gateType) {
		case SimGateType::AND:
			if (gateIndex < andGates.size()) {
				andGates[gateIndex].addInput(inputId, portId);
				addOutputDependency(inputId, SimGateType::AND, gateIndex);
			}
			break;
		case SimGateType::XOR:
			if (gateIndex < xorGates.size()) {
				xorGates[gateIndex].addInput(inputId, portId);
				addOutputDependency(inputId, SimGateType::XOR, gateIndex);
			}
			break;
		case SimGateType::JUNCTION:
			if (gateIndex < junctions.size()) {
				junctions[gateIndex].addInput(inputId, portId);
				addOutputDependency(inputId, SimGateType::JUNCTION, gateIndex);
			}
			break;
		case SimGateType::BUFFER:
			if (gateIndex < buffers.size()) {
				buffers[gateIndex].addInput(inputId, portId);
				addOutputDependency(inputId, SimGateType::BUFFER, gateIndex);
			}
			break;
		case SimGateType::SINGLE_BUFFER:
			if (gateIndex < singleBuffers.size()) {
				singleBuffers[gateIndex].addInput(inputId, portId);
				addOutputDependency(inputId, SimGateType::SINGLE_BUFFER, gateIndex);
			}
			break;
		case SimGateType::TRISTATE_BUFFER:
			if (gateIndex < tristateBuffers.size()) {
				tristateBuffers[gateIndex].addInput(inputId, portId);
				addOutputDependency(inputId, SimGateType::TRISTATE_BUFFER, gateIndex);
			}
			break;
		case SimGateType::CONSTANT:
			if (gateIndex < constantGates.size()) {
				constantGates[gateIndex].addInput(inputId, portId);
				addOutputDependency(inputId, SimGateType::CONSTANT, gateIndex);
			}
			break;
		case SimGateType::CONSTANT_RESET:
			if (gateIndex < constantResetGates.size()) {
				constantResetGates[gateIndex].addInput(inputId, portId);
				addOutputDependency(inputId, SimGateType::CONSTANT_RESET, gateIndex);
			}
			break;
		case SimGateType::COPY_SELF_OUTPUT:
			if (gateIndex < copySelfOutputGates.size()) {
				copySelfOutputGates[gateIndex].addInput(inputId, portId);
				addOutputDependency(inputId, SimGateType::COPY_SELF_OUTPUT, gateIndex);
			}
			break;
		}
		return;
	}

	logError("Gate not found for addInputToGate", "LogicSimulator::addInputToGate");
}

void LogicSimulator::removeInputFromGate(simulator_id_t simId, simulator_id_t inputId, connection_port_id_t portId) {
	// Use optimized lookup instead of linear search
	auto locationIt = gateLocations.find(simId);
	if (locationIt != gateLocations.end()) {
		SimGateType gateType = locationIt->second.first;
		size_t gateIndex = locationIt->second.second;
		
		switch (gateType) {
		case SimGateType::AND:
			if (gateIndex < andGates.size()) {
				andGates[gateIndex].removeInput(inputId, portId);
				removeOutputDependency(inputId, SimGateType::AND, gateIndex);
			}
			break;
		case SimGateType::XOR:
			if (gateIndex < xorGates.size()) {
				xorGates[gateIndex].removeInput(inputId, portId);
				removeOutputDependency(inputId, SimGateType::XOR, gateIndex);
			}
			break;
		case SimGateType::JUNCTION:
			if (gateIndex < junctions.size()) {
				junctions[gateIndex].removeInput(inputId, portId);
				removeOutputDependency(inputId, SimGateType::JUNCTION, gateIndex);
			}
			break;
		case SimGateType::BUFFER:
			if (gateIndex < buffers.size()) {
				buffers[gateIndex].removeInput(inputId, portId);
				removeOutputDependency(inputId, SimGateType::BUFFER, gateIndex);
			}
			break;
		case SimGateType::SINGLE_BUFFER:
			if (gateIndex < singleBuffers.size()) {
				singleBuffers[gateIndex].removeInput(inputId, portId);
				removeOutputDependency(inputId, SimGateType::SINGLE_BUFFER, gateIndex);
			}
			break;
		case SimGateType::TRISTATE_BUFFER:
			if (gateIndex < tristateBuffers.size()) {
				tristateBuffers[gateIndex].removeInput(inputId, portId);
				removeOutputDependency(inputId, SimGateType::TRISTATE_BUFFER, gateIndex);
			}
			break;
		case SimGateType::CONSTANT:
			if (gateIndex < constantGates.size()) {
				constantGates[gateIndex].removeInput(inputId, portId);
				removeOutputDependency(inputId, SimGateType::CONSTANT, gateIndex);
			}
			break;
		case SimGateType::CONSTANT_RESET:
			if (gateIndex < constantResetGates.size()) {
				constantResetGates[gateIndex].removeInput(inputId, portId);
				removeOutputDependency(inputId, SimGateType::CONSTANT_RESET, gateIndex);
			}
			break;
		case SimGateType::COPY_SELF_OUTPUT:
			if (gateIndex < copySelfOutputGates.size()) {
				copySelfOutputGates[gateIndex].removeInput(inputId, portId);
				removeOutputDependency(inputId, SimGateType::COPY_SELF_OUTPUT, gateIndex);
			}
			break;
		}
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

	auto constantResetIt = std::find_if(constantResetGates.begin(), constantResetGates.end(),
		[simId](const ConstantResetGate& gate) { return gate.getId() == simId; });
	if (constantResetIt != constantResetGates.end()) {
		return constantResetIt->getOutputSimIds();
	}

	auto copySelfIt = std::find_if(copySelfOutputGates.begin(), copySelfOutputGates.end(),
		[simId](const CopySelfOutputGate& gate) { return gate.getId() == simId; });
	if (copySelfIt != copySelfOutputGates.end()) {
		return copySelfIt->getOutputSimIds();
	}

	return std::nullopt;
}

// Performance optimization helper methods
void LogicSimulator::updateGateLocation(simulator_id_t gateId, SimGateType gateType, size_t gateIndex) {
	gateLocations[gateId] = {gateType, gateIndex};
}

void LogicSimulator::removeGateLocation(simulator_id_t gateId) {
	gateLocations.erase(gateId);
}

void LogicSimulator::addOutputDependency(simulator_id_t outputId, SimGateType gateType, size_t gateIndex) {
	outputDependencies[outputId].emplace_back(gateType, gateIndex);
}

void LogicSimulator::removeOutputDependency(simulator_id_t outputId, SimGateType gateType, size_t gateIndex) {
	auto it = outputDependencies.find(outputId);
	if (it != outputDependencies.end()) {
		auto& deps = it->second;
		deps.erase(std::remove(deps.begin(), deps.end(), std::make_pair(gateType, gateIndex)), deps.end());
		if (deps.empty()) {
			outputDependencies.erase(it);
		}
	}
}

void LogicSimulator::updateGateIndicesAfterRemoval(SimGateType gateType, size_t removedIndex) {
	// Update gateLocations - decrement indices for gates that were shifted down
	for (auto& [gateId, location] : gateLocations) {
		if (location.first == gateType && location.second > removedIndex) {
			location.second--;
		}
	}
	
	// Update outputDependencies - decrement indices for gates that were shifted down
	for (auto& [outputId, deps] : outputDependencies) {
		for (auto& dep : deps) {
			if (dep.first == gateType && dep.second > removedIndex) {
				dep.second--;
			}
		}
	}
}