#include "logicSimulator.h"
#include "simulatorGates.h"
#include "gateType.h"
#ifdef TRACY_PROFILER
#include <tracy/Tracy.hpp>
#endif

LogicSimulator::LogicSimulator(
	EvalConfig& evalConfig,
	std::vector<simulator_id_t>& dirtySimulatorIds) :
	evalConfig(evalConfig),
	dirtySimulatorIds(dirtySimulatorIds) {
	// Subscribe to EvalConfig changes to update the simulator accordingly
	evalConfig.subscribe([this]() {
		// Notify the simulation thread about config changes
		std::lock_guard<std::mutex> lk(cvMutex);
		cv.notify_all();
	});

	simulationThread = std::thread(&LogicSimulator::simulationLoop, this);
	expandDataVectors(simulatorIdProvider.getNewId()); // reserve the 0th id to be used as an invalid id
}

LogicSimulator::~LogicSimulator() {
	{
		std::lock_guard<std::mutex> lk(cvMutex);
		running = false;
		cv.notify_all();
	}
	if (simulationThread.joinable()) {
		simulationThread.join();
	}
}

void LogicSimulator::clearState() { }

double LogicSimulator::getAverageTickrate() const {
	if (!evalConfig.isRunning()) {
		return 0.0;
	}
	double tickspeed = averageTickrate.load(std::memory_order_acquire);
	// if tickspeed close enough to target tickspeed, return target tickspeed (nicer for ui)
	double targetTickrate = evalConfig.getTargetTickrate();
	double percentageError = (tickspeed - targetTickrate) / targetTickrate;
	if (std::abs(percentageError) < 0.01) {
		return targetTickrate;
	}
	return tickspeed;
}

void LogicSimulator::simulationLoop() {
	using clock = std::chrono::steady_clock;
	auto nextTick = clock::now();
	auto lastTickTime = clock::now();
	bool isFirstTick = true;

	while (running) {
#ifdef TRACY_PROFILER
		ZoneScoped;
#endif
		if (pauseRequest.load(std::memory_order_acquire)) {
			averageTickrate.store(0.0, std::memory_order_release);
			std::unique_lock<std::mutex> lk(cvMutex);
			isPaused.store(true, std::memory_order_release);
			cv.notify_all();
			cv.wait(lk, [&] { return !pauseRequest || !running; });
			isPaused.store(false, std::memory_order_release);
			if (!running) break;
			// reset nextTick after resuming from pause
			nextTick = clock::now();
			lastTickTime = clock::now();
			isFirstTick = true;
		}

		processPendingStateChanges();

		// Sprint handling: if sprintCount > 0, execute ticks immediately, regardless of run/pause state.
		// Sprint ticks update the EMA tickrate just like normal ticks, but bypass the limiter wait.
		bool didSprint = false;
		while (running && !pauseRequest.load(std::memory_order_acquire) && evalConfig.consumeSprintTick()) {
			didSprint = true;
			auto currentTime = clock::now();
			if (evalConfig.isRealistic()) {
				realisticTickOnce();
			} else {
				tickOnceSimple();
			}
			updateEmaTickrate(currentTime, lastTickTime, isFirstTick);
			// Allow a pause to break a long sprint
			if (pauseRequest.load(std::memory_order_acquire)) break;
		}

		if (!didSprint && evalConfig.isRunning()) {
			auto currentTime = clock::now();

			if (evalConfig.isRealistic()) {
				realisticTickOnce();
			} else {
				tickOnceSimple();
			}

			// Calculate EMA for tickrate after each tick
			updateEmaTickrate(currentTime, lastTickTime, isFirstTick);

			// handle timing after ticking
			if (evalConfig.isTickrateLimiterEnabled()) {
				double targetTickrate = evalConfig.getTargetTickrate();
				if (targetTickrate > 0) {
					auto period = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::duration<double>(1.0 / targetTickrate));
					nextTick += period;
					std::unique_lock lk(cvMutex);
					cv.wait_until(lk, nextTick, [&] { return pauseRequest || !running || !evalConfig.isRunning(); });
				}
			}
		} else if (!didSprint) {
			// wait for state change or resume signal
			averageTickrate.store(0.0, std::memory_order_release);
			std::unique_lock lk(cvMutex);
			cv.wait(lk, [&] {
				// Check for pending state changes under lock to avoid race conditions
				std::lock_guard<std::mutex> stateLock(stateChangeQueueMutex);
				return pauseRequest || !running || evalConfig.isRunning() || evalConfig.getSprintCount() > 0 || !pendingStateChanges.empty();
			});
			// reset nextTick when resuming simulation
			nextTick = clock::now();
			lastTickTime = clock::now();
			isFirstTick = true;
		}
	}
}

inline void LogicSimulator::updateEmaTickrate(
	const std::chrono::steady_clock::time_point& currentTime,
	std::chrono::steady_clock::time_point& lastTickTime,
	bool& isFirstTick) {
	if (!isFirstTick) {
		auto deltaTime = std::chrono::duration_cast<std::chrono::nanoseconds>(currentTime - lastTickTime);
		if (deltaTime.count() > 0) {
			// Calculate current tickrate in Hz (ticks per second)
			double currentTickrate = 1.0e9 / static_cast<double>(deltaTime.count());
			double dtSeconds = std::chrono::duration<double>(deltaTime).count();
			double alpha = 1.0 - std::exp(-dtSeconds * std::log(2.0) / tickrateHalflife);

			// Apply EMA: EMA_new = alpha * current + (1 - alpha) * EMA_old
			double currentEMA = averageTickrate.load(std::memory_order_acquire);
			double newEMA = alpha * currentTickrate + (1.0 - alpha) * currentEMA;
			averageTickrate.store(newEMA, std::memory_order_release);
		}
	} else {
		isFirstTick = false;
	}
	lastTickTime = currentTime;
}

inline void LogicSimulator::tickOnceSimple() {
#ifdef TRACY_PROFILER
	ZoneScoped;
#endif
	std::unique_lock lkNext(statesWritingMutex);
	calculateNewStatesSimple();
	propagateNewStates();
	std::unique_lock lkCurEx(statesReadingMutex);
	std::swap(statesReading, statesWriting);
	processJunctions();
}

inline void LogicSimulator::realisticTickOnce() {
#ifdef TRACY_PROFILER
	ZoneScoped;
#endif
	std::unique_lock lkNext(statesWritingMutex);
	calculateNewStatesRealistic();
	propagateNewStates();
	std::unique_lock lkCurEx(statesReadingMutex);
	std::swap(statesReading, statesWriting);
	processJunctions();
}

inline void LogicSimulator::calculateNewStatesSimple() {
#ifdef TRACY_PROFILER
	ZoneScoped;
#endif
	for (auto& gate : andGates) {
		gate.setNewStateSimple(statesWriting, statesReading, counts);
	}
	for (auto& gate : xorGates) {
		gate.setNewStateSimple(statesWriting, statesReading, counts);
	}
	for (auto& gate : tristateBuffers) {
		gate.setNewStateSimple(statesWriting, statesReading, counts);
	}
	for (auto& gate : constantGates) {
		gate.setNewStateSimple(statesWriting, statesReading, counts);
	}
	for (auto& gate : copySelfOutputGates) {
		gate.setNewStateSimple(statesWriting, statesReading, counts);
	}
}

inline void LogicSimulator::calculateNewStatesRealistic() {
#ifdef TRACY_PROFILER
	ZoneScoped;
#endif
	for (auto& gate : andGates) {
		gate.setNewStateRealistic(statesWriting, statesReading, counts);
	}
	for (auto& gate : xorGates) {
		gate.setNewStateRealistic(statesWriting, statesReading, counts);
	}
	for (auto& gate : tristateBuffers) {
		gate.setNewStateRealistic(statesWriting, statesReading, counts);
	}
	for (auto& gate : constantGates) {
		gate.setNewStateSimple(statesWriting, statesReading, counts);
	}
	for (auto& gate : copySelfOutputGates) {
		gate.setNewStateSimple(statesWriting, statesReading, counts);
	}
}

inline void LogicSimulator::propagateNewStates() {
#ifdef TRACY_PROFILER
	ZoneScoped;
#endif
	for (auto& gate : andGates) {
		gate.propagateNewState(statesWriting, statesReading, counts);
	}
	for (auto& gate : xorGates) {
		gate.propagateNewState(statesWriting, statesReading, counts);
	}
	for (auto& gate : tristateBuffers) {
		gate.propagateNewState(statesWriting, statesReading, counts);
	}
	for (auto& gate : constantGates) {
		gate.propagateNewState(statesWriting, statesReading, counts);
	}
	for (auto& gate : copySelfOutputGates) {
		gate.propagateNewState(statesWriting, statesReading, counts);
	}
}

inline void LogicSimulator::processJunctions() {
#ifdef TRACY_PROFILER
	ZoneScoped;
#endif
	for (auto& junction : junctions) {
		junction.process(statesWriting, statesReading, counts);
	}
}

logic_state_t LogicSimulator::getState(simulator_id_t id) const {
	std::shared_lock lk(statesReadingMutex);
	return statesReading[id];
}

std::vector<logic_state_t> LogicSimulator::getStates(const std::vector<simulator_id_t>& ids) const {
	std::vector<logic_state_t> result(ids.size());
	std::shared_lock lk(statesReadingMutex);
	for (size_t i = 0; i < ids.size(); ++i) {
		const size_t id = ids[i];
		if (id < statesReading.size()) {
			result[i] = statesReading[id];
		} else {
			result[i] = logic_state_t::UNDEFINED;
		}
	}
	return result;
}

simulator_id_t LogicSimulator::addGate(const GateType gateType) {
	switch (gateType) {
	case GateType::AND:
		return addAndGate();
	case GateType::OR:
		return addOrGate();
	case GateType::NAND:
		return addNandGate();
	case GateType::NOR:
		return addNorGate();
	case GateType::XOR:
		return addXorGate();
	case GateType::XNOR:
		return addXnorGate();
	case GateType::DUMMY_INPUT:
		return addCopySelfOutputGate();
	case GateType::TICK_INPUT:
		return addConstantOffGate();
	case GateType::CONSTANT_OFF:
		return addConstantOffGate();
	case GateType::CONSTANT_ON:
		return addConstantOnGate();
	case GateType::TRISTATE_BUFFER:
		return addTristateBufferGate();
	case GateType::TRISTATE_BUFFER_INVERTED:
		return addTristateBufferInvertedGate();
	case GateType::JUNCTION:
		return addJunction();
	default:
		logError("Unknown gate type: {}", "LogicSimulator::addGate", static_cast<int>(gateType));
		return 0;
	}
}

void LogicSimulator::removeGate(simulator_id_t gateId) {
	if (outputDependencies.contains(gateId)) {
		std::vector<GateDependency> dependencies = outputDependencies[gateId];
		for (const auto& dep : dependencies) {
			removeConnection(gateId, dep.sourcePort, dep.dependentId, dep.destinationPort);
		}
		outputDependencies.erase(gateId);
	}
	if (inputDependencies.contains(gateId)) {
		std::vector<GateDependency> dependencies = inputDependencies[gateId];
		for (const auto& dep : dependencies) {
			removeConnection(dep.dependentId, dep.sourcePort, gateId, dep.destinationPort);
		}
		inputDependencies.erase(gateId);
	}
	std::optional<std::vector<simulator_id_t>> occupiedIdsOpt = getOccupiedIds(gateId);
	if (occupiedIdsOpt) {
		for (const simulator_id_t& id : occupiedIdsOpt.value()) {
			simulatorIdProvider.releaseId(id);
			dirtySimulatorIds.push_back(id);
		}
	}

	auto locationIt = gateLocations.find(gateId);
	if (locationIt == gateLocations.end()) {
		logError("Gate not found: {}", "LogicSimulator::removeGate", gateId);
		return;
	}
	SimGateType gateType = locationIt->second.gateType;
	size_t gateIndex = locationIt->second.gateIndex;

	auto fixMovedIndex = [&](auto& vec) {
		const size_t last = vec.size() - 1;
		if (gateIndex != last) {
			std::swap(vec[gateIndex], vec[last]);
			simulator_id_t movedId = vec[gateIndex].getId();
			gateLocations[movedId].gateIndex = gateIndex;
		}
		vec.pop_back();
	};

	switch (gateType) {
	case SimGateType::AND: if (!andGates.empty()) fixMovedIndex(andGates); break;
	case SimGateType::XOR: if (!xorGates.empty()) fixMovedIndex(xorGates); break;
	case SimGateType::TRISTATE_BUFFER: if (!tristateBuffers.empty()) fixMovedIndex(tristateBuffers); break;
	case SimGateType::CONSTANT: if (!constantGates.empty()) fixMovedIndex(constantGates); break;
	case SimGateType::COPY_SELF_OUTPUT: if (!copySelfOutputGates.empty()) fixMovedIndex(copySelfOutputGates); break;
	case SimGateType::JUNCTION: if (!junctions.empty()) fixMovedIndex(junctions); break;
	}

	gateLocations.erase(gateId);
	inputDependencies.erase(gateId);
	outputDependencies.erase(gateId);
}

void LogicSimulator::makeConnection(simulator_id_t sourceId, connection_port_id_t sourcePort, simulator_id_t destinationId, connection_port_id_t destinationPort) {
	std::optional<simulator_id_t> sourcePortId = getOutputPortId(sourceId, sourcePort);
	if (!sourcePortId) {
		logError("Source output port not found: {}", "LogicSimulator::makeConnection", sourceId);
		return;
	}
	dirtySimulatorIds.push_back(sourcePortId.value());
	std::optional<simulator_id_t> destinationInputPortIdOpt = getInputPortId(destinationId, destinationPort);
	if (!destinationInputPortIdOpt) {
		logError("Destination input port not found: {}", "LogicSimulator::makeConnection", destinationId);
		return;
	}
	simulator_id_t destinationInputPortId = destinationInputPortIdOpt.value();
	auto locationIt = gateLocations.find(sourceId);
	if (locationIt == gateLocations.end()) {
		logError("Source gate not found: {}", "LogicSimulator::makeConnection", sourceId);
		return;
	}
	SimGateType gateType = locationIt->second.gateType;
	size_t gateIndex = locationIt->second.gateIndex;
	bool success = false;
	switch (gateType) {
	case SimGateType::AND:
		andGates[gateIndex].addOutput(sourcePort, destinationInputPortId);
		success = true;
		break;
	case SimGateType::XOR:
		xorGates[gateIndex].addOutput(sourcePort, destinationInputPortId);
		success = true;
		break;
	case SimGateType::TRISTATE_BUFFER:
		tristateBuffers[gateIndex].addOutput(sourcePort, destinationInputPortId);
		success = true;
		break;
	case SimGateType::CONSTANT:
		constantGates[gateIndex].addOutput(sourcePort, destinationInputPortId);
		success = true;
		break;
	case SimGateType::COPY_SELF_OUTPUT:
		copySelfOutputGates[gateIndex].addOutput(sourcePort, destinationInputPortId);
		success = true;
		break;
	case SimGateType::JUNCTION:
		junctions[gateIndex].addOutput(sourcePort, destinationInputPortId);
		success = true;
		break;
	}
	if (success) {
		logic_state_t sourceState = statesReading[sourcePortId.value()];
		switch (sourceState) {
		case logic_state_t::LOW:
			counts.L[destinationInputPortId]++;
			break;
		case logic_state_t::HIGH:
			counts.H[destinationInputPortId]++;
			break;
		case logic_state_t::FLOATING:
			counts.Z[destinationInputPortId]++;
			break;
		case logic_state_t::UNDEFINED:
			counts.X[destinationInputPortId]++;
			break;
		}
		addGateDependency(sourceId, sourcePort, destinationId, destinationPort);
	}
}

void LogicSimulator::removeConnection(simulator_id_t sourceId, connection_port_id_t sourcePort, simulator_id_t destinationId, connection_port_id_t destinationPort) {
	std::optional<simulator_id_t> sourcePortId = getOutputPortId(sourceId, sourcePort);
	if (!sourcePortId) {
		logError("Source output port not found: {}", "LogicSimulator::makeConnection", sourceId);
		return;
	}
	dirtySimulatorIds.push_back(sourcePortId.value());
	std::optional<simulator_id_t> destinationInputPortIdOpt = getInputPortId(destinationId, destinationPort);
	if (!destinationInputPortIdOpt) {
		logError("Destination input port not found: {}", "LogicSimulator::makeConnection", destinationId);
		return;
	}
	simulator_id_t destinationInputPortId = destinationInputPortIdOpt.value();
	auto locationIt = gateLocations.find(sourceId);
	if (locationIt == gateLocations.end()) {
		logError("Source gate not found: {}", "LogicSimulator::makeConnection", sourceId);
		return;
	}
	SimGateType gateType = locationIt->second.gateType;
	size_t gateIndex = locationIt->second.gateIndex;
	bool success = false;
	switch (gateType) {
	case SimGateType::AND:
		andGates[gateIndex].removeOutput(sourcePort, destinationInputPortId);
		success = true;
		break;
	case SimGateType::XOR:
		xorGates[gateIndex].removeOutput(sourcePort, destinationInputPortId);
		success = true;
		break;
	case SimGateType::TRISTATE_BUFFER:
		tristateBuffers[gateIndex].removeOutput(sourcePort, destinationInputPortId);
		success = true;
		break;
	case SimGateType::CONSTANT:
		constantGates[gateIndex].removeOutput(sourcePort, destinationInputPortId);
		success = true;
		break;
	case SimGateType::COPY_SELF_OUTPUT:
		copySelfOutputGates[gateIndex].removeOutput(sourcePort, destinationInputPortId);
		success = true;
		break;
	case SimGateType::JUNCTION:
		junctions[gateIndex].removeOutput(sourcePort, destinationInputPortId);
		success = true;
		break;
	}
	if (success) {
		logic_state_t sourceState = statesReading[sourcePortId.value()];
		switch (sourceState) {
		case logic_state_t::LOW:
			counts.L[destinationInputPortId]--;
			break;
		case logic_state_t::HIGH:
			counts.H[destinationInputPortId]--;
			break;
		case logic_state_t::FLOATING:
			counts.Z[destinationInputPortId]--;
			break;
		case logic_state_t::UNDEFINED:
			counts.X[destinationInputPortId]--;
			break;
		}
		removeGateDependency(sourceId, sourcePort, destinationId, destinationPort);
	}
}

void LogicSimulator::endEdit() {
	processJunctions();
}

void LogicSimulator::setState(simulator_id_t id, logic_state_t state) {
	std::unique_lock lockWriting(statesWritingMutex, std::try_to_lock);
	std::unique_lock lockReading(statesReadingMutex, std::try_to_lock);
	if (lockWriting.owns_lock() && lockReading.owns_lock()) {
		auto it = gateLocations.find(id);
		if (it != gateLocations.end()) {
			SimGateType gateType = it->second.gateType;
			size_t gateIndex = it->second.gateIndex;
			switch (gateType) {
			case SimGateType::AND:
				andGates[gateIndex].setState(statesWriting, statesReading, counts, state);
				break;
			case SimGateType::XOR:
				xorGates[gateIndex].setState(statesWriting, statesReading, counts, state);
				break;
			case SimGateType::TRISTATE_BUFFER:
				tristateBuffers[gateIndex].setState(statesWriting, statesReading, counts, state);
				break;
			case SimGateType::CONSTANT:
				constantGates[gateIndex].setState(statesWriting, statesReading, counts, state);
				break;
			case SimGateType::COPY_SELF_OUTPUT:
				copySelfOutputGates[gateIndex].setState(statesWriting, statesReading, counts, state);
				break;
			}
		}
		processJunctions();
	} else {
		std::lock_guard<std::mutex> lock(stateChangeQueueMutex);
		pendingStateChanges.push({ id, state });
		cv.notify_one();
	}
}

void LogicSimulator::processPendingStateChanges() {
	std::queue<StateChange> localQueue;
	{
		std::lock_guard<std::mutex> lock(stateChangeQueueMutex);
		std::swap(localQueue, pendingStateChanges);
	}

	if (!localQueue.empty()) {
		std::scoped_lock lk(statesWritingMutex, statesReadingMutex);
		while (!localQueue.empty()) {
			const StateChange& change = localQueue.front();

			simulator_id_t id = change.id;
			logic_state_t state = change.state;

			auto it = gateLocations.find(id);
			if (it != gateLocations.end()) {
				SimGateType gateType = it->second.gateType;
				size_t gateIndex = it->second.gateIndex;
				switch (gateType) {
				case SimGateType::AND:
					andGates[gateIndex].setState(statesWriting, statesReading, counts, state);
					break;
				case SimGateType::XOR:
					xorGates[gateIndex].setState(statesWriting, statesReading, counts, state);
					break;
				case SimGateType::TRISTATE_BUFFER:
					tristateBuffers[gateIndex].setState(statesWriting, statesReading, counts, state);
					break;
				case SimGateType::CONSTANT:
					constantGates[gateIndex].setState(statesWriting, statesReading, counts, state);
					break;
				case SimGateType::COPY_SELF_OUTPUT:
					copySelfOutputGates[gateIndex].setState(statesWriting, statesReading, counts, state);
					break;
				}
			}

			localQueue.pop();
		}
		processJunctions();
	}
}

std::optional<simulator_id_t> LogicSimulator::getInputPortId(simulator_id_t simId, connection_port_id_t portId) const {
	auto locationIt = gateLocations.find(simId);
	if (locationIt == gateLocations.end()) {
		return std::nullopt;
	}
	SimGateType gateType = locationIt->second.gateType;
	size_t gateIndex = locationIt->second.gateIndex;
	switch (gateType) {
	case SimGateType::AND:
		return andGates[gateIndex].getInputPortId(portId);
	case SimGateType::XOR:
		return xorGates[gateIndex].getInputPortId(portId);
	case SimGateType::TRISTATE_BUFFER:
		return tristateBuffers[gateIndex].getInputPortId(portId);
	case SimGateType::CONSTANT:
		return constantGates[gateIndex].getInputPortId(portId);
	case SimGateType::COPY_SELF_OUTPUT:
		return copySelfOutputGates[gateIndex].getInputPortId(portId);
	case SimGateType::JUNCTION:
		return junctions[gateIndex].getInputPortId(portId);
	}
	return std::nullopt;
}

std::optional<simulator_id_t> LogicSimulator::getOutputPortId(simulator_id_t simId, connection_port_id_t portId) const {
	auto locationIt = gateLocations.find(simId);
	if (locationIt == gateLocations.end()) {
		return std::nullopt;
	}
	SimGateType gateType = locationIt->second.gateType;
	size_t gateIndex = locationIt->second.gateIndex;
	switch (gateType) {
	case SimGateType::AND:
		return andGates[gateIndex].getOutputPortId(portId);
	case SimGateType::XOR:
		return xorGates[gateIndex].getOutputPortId(portId);
	case SimGateType::TRISTATE_BUFFER:
		return tristateBuffers[gateIndex].getOutputPortId(portId);
	case SimGateType::CONSTANT:
		return constantGates[gateIndex].getOutputPortId(portId);
	case SimGateType::COPY_SELF_OUTPUT:
		return copySelfOutputGates[gateIndex].getOutputPortId(portId);
	case SimGateType::JUNCTION:
		return junctions[gateIndex].getOutputPortId(portId);
	}
	return std::nullopt;
}

simulator_id_t LogicSimulator::addAndGate() {
	simulator_id_t id = simulatorIdProvider.getNewId();
	expandDataVectors(id);
	andGates.push_back(ANDLikeGate(id, false, false));
	andGates.back().resetState(evalConfig.isRealistic(), statesWriting, statesReading, counts);
	gateLocations[id] = GateLocation(SimGateType::AND, andGates.size() - 1);
	return id;
}

simulator_id_t LogicSimulator::addOrGate() {
	simulator_id_t id = simulatorIdProvider.getNewId();
	expandDataVectors(id);
	andGates.push_back(ANDLikeGate(id, true, true));
	andGates.back().resetState(evalConfig.isRealistic(), statesWriting, statesReading, counts);
	gateLocations[id] = GateLocation(SimGateType::AND, andGates.size() - 1);
	return id;
}

simulator_id_t LogicSimulator::addNandGate() {
	simulator_id_t id = simulatorIdProvider.getNewId();
	expandDataVectors(id);
	andGates.push_back(ANDLikeGate(id, false, true));
	andGates.back().resetState(evalConfig.isRealistic(), statesWriting, statesReading, counts);
	gateLocations[id] = GateLocation(SimGateType::AND, andGates.size() - 1);
	return id;
}

simulator_id_t LogicSimulator::addNorGate() {
	simulator_id_t id = simulatorIdProvider.getNewId();
	expandDataVectors(id);
	andGates.push_back(ANDLikeGate(id, true, false));
	andGates.back().resetState(evalConfig.isRealistic(), statesWriting, statesReading, counts);
	gateLocations[id] = GateLocation(SimGateType::AND, andGates.size() - 1);
	return id;
}

simulator_id_t LogicSimulator::addXorGate() {
	simulator_id_t id = simulatorIdProvider.getNewId();
	expandDataVectors(id);
	xorGates.push_back(XORLikeGate(id, false));
	xorGates.back().resetState(evalConfig.isRealistic(), statesWriting, statesReading, counts);
	gateLocations[id] = GateLocation(SimGateType::XOR, xorGates.size() - 1);
	return id;
}

simulator_id_t LogicSimulator::addXnorGate() {
	simulator_id_t id = simulatorIdProvider.getNewId();
	expandDataVectors(id);
	xorGates.push_back(XORLikeGate(id, true));
	xorGates.back().resetState(evalConfig.isRealistic(), statesWriting, statesReading, counts);
	gateLocations[id] = GateLocation(SimGateType::XOR, xorGates.size() - 1);
	return id;
}

simulator_id_t LogicSimulator::addConstantOnGate() {
	simulator_id_t id = simulatorIdProvider.getNewId();
	expandDataVectors(id);
	constantGates.push_back(ConstantGate(id, logic_state_t::HIGH));
	constantGates.back().resetState(evalConfig.isRealistic(), statesWriting, statesReading, counts);
	gateLocations[id] = GateLocation(SimGateType::CONSTANT, constantGates.size() - 1);
	return id;
}

simulator_id_t LogicSimulator::addConstantOffGate() {
	simulator_id_t id = simulatorIdProvider.getNewId();
	expandDataVectors(id);
	constantGates.push_back(ConstantGate(id, logic_state_t::LOW));
	constantGates.back().resetState(evalConfig.isRealistic(), statesWriting, statesReading, counts);
	gateLocations[id] = GateLocation(SimGateType::CONSTANT, constantGates.size() - 1);
	return id;
}

simulator_id_t LogicSimulator::addCopySelfOutputGate() {
	simulator_id_t id = simulatorIdProvider.getNewId();
	expandDataVectors(id);
	copySelfOutputGates.push_back(CopySelfOutputGate(id));
	copySelfOutputGates.back().resetState(evalConfig.isRealistic(), statesWriting, statesReading, counts);
	gateLocations[id] = GateLocation(SimGateType::COPY_SELF_OUTPUT, copySelfOutputGates.size() - 1);
	return id;
}

simulator_id_t LogicSimulator::addTristateBufferGate() {
	simulator_id_t id = simulatorIdProvider.getNewId();
	simulator_id_t enableId = simulatorIdProvider.getNewId();
	expandDataVectors(id);
	expandDataVectors(enableId);
	tristateBuffers.push_back(TristateBuffer(id, enableId, false));
	tristateBuffers.back().resetState(evalConfig.isRealistic(), statesWriting, statesReading, counts);
	gateLocations[id] = GateLocation(SimGateType::TRISTATE_BUFFER, tristateBuffers.size() - 1);
	return id;
}

simulator_id_t LogicSimulator::addTristateBufferInvertedGate() {
	simulator_id_t id = simulatorIdProvider.getNewId();
	simulator_id_t enableId = simulatorIdProvider.getNewId();
	expandDataVectors(id);
	expandDataVectors(enableId);
	tristateBuffers.push_back(TristateBuffer(id, enableId, true));
	tristateBuffers.back().resetState(evalConfig.isRealistic(), statesWriting, statesReading, counts);
	gateLocations[id] = GateLocation(SimGateType::TRISTATE_BUFFER, tristateBuffers.size() - 1);
	return id;
}

simulator_id_t LogicSimulator::addJunction() {
	simulator_id_t id = simulatorIdProvider.getNewId();
	expandDataVectors(id);
	junctions.push_back(Junction(id));
	gateLocations[id] = GateLocation(SimGateType::JUNCTION, junctions.size() - 1);
	return id;
}

void LogicSimulator::expandDataVectors(simulator_id_t maxId) {
	if (statesReading.size() <= maxId) {
		statesReading.resize(maxId + 1, logic_state_t::UNDEFINED);
	}
	if (statesWriting.size() <= maxId) {
		statesWriting.resize(maxId + 1, logic_state_t::UNDEFINED);
	}
	if (counts.L.size() <= maxId) {
		counts.L.resize(maxId + 1, 0);
	}
	if (counts.H.size() <= maxId) {
		counts.H.resize(maxId + 1, 0);
	}
	if (counts.Z.size() <= maxId) {
		counts.Z.resize(maxId + 1, 0);
	}
	if (counts.X.size() <= maxId) {
		counts.X.resize(maxId + 1, 0);
	}
}

void LogicSimulator::addGateDependency(simulator_id_t sourceId, connection_port_id_t sourcePort, simulator_id_t destinationId, connection_port_id_t destinationPort) {
	inputDependencies[destinationId].emplace_back(sourceId, sourcePort, destinationPort);
	outputDependencies[sourceId].emplace_back(destinationId, sourcePort, destinationPort);
}

void LogicSimulator::removeGateDependency(simulator_id_t sourceId, connection_port_id_t sourcePort, simulator_id_t destinationId, connection_port_id_t destinationPort) {
	{
		auto it = inputDependencies.find(destinationId);
		if (it != inputDependencies.end()) {
			auto& vec = it->second;
			auto it2 = std::find(vec.begin(), vec.end(), GateDependency(sourceId, sourcePort, destinationPort));
			if (it2 != vec.end()) {
				vec.erase(it2);
				if (vec.empty()) {
					inputDependencies.erase(it);
				}
			}
		}
	}
	{
		auto it = outputDependencies.find(sourceId);
		if (it != outputDependencies.end()) {
			auto& vec = it->second;
			auto it2 = std::find(vec.begin(), vec.end(), GateDependency(destinationId, sourcePort, destinationPort));
			if (it2 != vec.end()) {
				vec.erase(it2);
				if (vec.empty()) {
					outputDependencies.erase(it);
				}
			}
		}
	}
}

std::optional<std::vector<simulator_id_t>> LogicSimulator::getOccupiedIds(simulator_id_t gateId) const {
	auto locationIt = gateLocations.find(gateId);
	if (locationIt == gateLocations.end()) {
		return std::nullopt;
	}
	SimGateType gateType = locationIt->second.gateType;
	size_t gateIndex = locationIt->second.gateIndex;
	switch (gateType) {
	case SimGateType::AND:
		return andGates[gateIndex].getOccupiedIds();
	case SimGateType::XOR:
		return xorGates[gateIndex].getOccupiedIds();
	case SimGateType::TRISTATE_BUFFER:
		return tristateBuffers[gateIndex].getOccupiedIds();
	case SimGateType::CONSTANT:
		return constantGates[gateIndex].getOccupiedIds();
	case SimGateType::COPY_SELF_OUTPUT:
		return copySelfOutputGates[gateIndex].getOccupiedIds();
	case SimGateType::JUNCTION:
		return junctions[gateIndex].getOccupiedIds();
	}
	return std::nullopt;
}