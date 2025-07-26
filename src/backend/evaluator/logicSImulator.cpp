#include "logicSimulator.h"

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