#include "logicSimulator.h"
#include "gateType.h"

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
		return false;
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
	std::unique_lock lkNext(statesWritingMutex);
	calculateNewStatesSimple();
	propagateNewStates();
	processJunctions();
	std::unique_lock lkCurEx(statesReadingMutex);
	std::swap(statesReading, statesWriting);
}

inline void LogicSimulator::realisticTickOnce() {
	std::unique_lock lkNext(statesWritingMutex);
	calculateNewStatesRealistic();
	propagateNewStates();
	processJunctions();
	std::unique_lock lkCurEx(statesReadingMutex);
	std::swap(statesReading, statesWriting);
}

inline void LogicSimulator::calculateNewStatesSimple() {
	for (auto& gate : andGates) {
		gate.setNewStateSimple(statesWriting, countL, countH, countZ, countX);
	}
	for (auto& gate : xorGates) {
		gate.setNewStateSimple(statesWriting, countL, countH, countZ, countX);
	}
}

inline void LogicSimulator::calculateNewStatesRealistic() {
	for (auto& gate : andGates) {
		gate.setNewStateRealistic(statesWriting, statesReading, countL, countH, countZ, countX);
	}
	for (auto& gate : xorGates) {
		gate.setNewStateRealistic(statesWriting, statesReading, countL, countH, countZ, countX);
	}
}

inline void LogicSimulator::propagateNewStates() {
	for (auto& gate : andGates) {
		gate.propagateNewState(statesWriting, statesReading, countL, countH, countZ, countX);
	}
	for (auto& gate : xorGates) {
		gate.propagateNewState(statesWriting, statesReading, countL, countH, countZ, countX);
	}
}

inline void LogicSimulator::processJunctions() {

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
	default:
		logError("Unknown gate type: {}", "LogicSimulator::addGate", static_cast<int>(gateType));
		return 0;
	}
}

void LogicSimulator::removeGate(simulator_id_t gateId) {

}

void LogicSimulator::makeConnection(simulator_id_t sourceId, connection_port_id_t sourcePort, simulator_id_t destinationId, connection_port_id_t destinationPort) {

}

void LogicSimulator::removeConnection(simulator_id_t sourceId, connection_port_id_t sourcePort, simulator_id_t destinationId, connection_port_id_t destinationPort) {

}

void LogicSimulator::endEdit() {

}

void LogicSimulator::setState(simulator_id_t id, logic_state_t state) {

}

void LogicSimulator::processPendingStateChanges() {

}

std::optional<simulator_id_t> LogicSimulator::getOutputPortId(simulator_id_t simId, connection_port_id_t portId) const {
	return simId; // update this later to support multi-output blocks
}

simulator_id_t LogicSimulator::addAndGate() {
	simulator_id_t id = simulatorIdProvider.getNewId();
	andGates.push_back(ANDLikeGate(id, false, false));
	expandDataVectors(id);
	return id;
}

simulator_id_t LogicSimulator::addOrGate() {
	simulator_id_t id = simulatorIdProvider.getNewId();
	andGates.push_back(ANDLikeGate(id, true, true));
	expandDataVectors(id);
	return id;
}

simulator_id_t LogicSimulator::addNandGate() {
	simulator_id_t id = simulatorIdProvider.getNewId();
	andGates.push_back(ANDLikeGate(id, false, true));
	expandDataVectors(id);
	return id;
}

simulator_id_t LogicSimulator::addNorGate() {
	simulator_id_t id = simulatorIdProvider.getNewId();
	andGates.push_back(ANDLikeGate(id, true, false));
	expandDataVectors(id);
	return id;
}

simulator_id_t LogicSimulator::addXorGate() {
	simulator_id_t id = simulatorIdProvider.getNewId();
	xorGates.push_back(XORLikeGate(id, false));
	expandDataVectors(id);
	return id;
}

simulator_id_t LogicSimulator::addXnorGate() {
	simulator_id_t id = simulatorIdProvider.getNewId();
	xorGates.push_back(XORLikeGate(id, true));
	expandDataVectors(id);
	return id;
}

void LogicSimulator::expandDataVectors(simulator_id_t maxId) {
	if (statesReading.size() <= maxId) {
		statesReading.resize(maxId + 1, logic_state_t::UNDEFINED);
	}
	if (statesWriting.size() <= maxId) {
		statesWriting.resize(maxId + 1, logic_state_t::UNDEFINED);
	}
	if (countL.size() <= maxId) {
		countL.resize(maxId + 1, 0);
	}
	if (countH.size() <= maxId) {
		countH.resize(maxId + 1, 0);
	}
	if (countZ.size() <= maxId) {
		countZ.resize(maxId + 1, 0);
	}
	if (countX.size() <= maxId) {
		countX.resize(maxId + 1, 0);
	}
}
