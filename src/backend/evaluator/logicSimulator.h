#ifndef logicSimulator_h
#define logicSimulator_h

#include "simulatorGates.h"

class LogicSimulator {
friend class SimulatorOptimizer;
friend class SimPauseGuard;
public:
	LogicSimulator();
	~LogicSimulator();
	void clearState();
	void setTargetTickrate(unsigned int targetTickrate) {
		this->targetTickrate = targetTickrate;
	}
	void setThrottlingTickrate(bool throttlingTickrate) {
		this->throttlingTickrate = throttlingTickrate;
	}
	unsigned int getAverageTickrate();
	void setState(simulator_id_t id, logic_state_t state);
	void setStates(const std::vector<simulator_id_t>& ids, const std::vector<logic_state_t>& states);
	logic_state_t getState(simulator_id_t id) const;
	std::vector<logic_state_t> getStates(const std::vector<simulator_id_t>& ids) const;

private:
	std::jthread simulationThread;
	std::atomic<bool> running { true };

	std::atomic<unsigned> targetTickrate { 40 * 60 }; // ticks per minute
	std::atomic<bool> throttlingTickrate { true };

	std::atomic<bool> pauseRequest { false };
	std::atomic<bool> isPaused { false };
	std::mutex cvMutex;
	std::condition_variable cv;

	std::vector<logic_state_t> statesA;
	std::vector<logic_state_t> statesB;

	mutable std::shared_mutex statesAMutex;
	std::mutex statesBMutex;

	std::vector<ANDLikeGate> andGates;
	std::vector<XORLikeGate> xorGates;
	std::vector<JunctionGate> junctions;
	std::vector<BufferGate> buffers;
	std::vector<SingleBufferGate> singleBuffers;
	std::vector<TristateBufferGate> tristateBuffers;
	std::vector<ConstantGate> constantGates;
	std::vector<ConstantResetGate> constantResetGates; // for tick buttons mainly
	std::vector<CopySelfOutputGate> copySelfOutputGates;

	void simulationLoop();
	inline void tickOnce();
};

class SimPauseGuard {
public:
	explicit SimPauseGuard(LogicSimulator& s) : sim(s) {
		{
			std::lock_guard<std::mutex> lk(sim.cvMutex);
			sim.pauseRequest.store(true, std::memory_order_release);
			sim.cv.notify_all();
		}
		// wait until the sim thread *confirms* it is paused
		std::unique_lock<std::mutex> lk(sim.cvMutex);
		sim.cv.wait(lk, [&]{ return sim.isPaused.load(std::memory_order_acquire); });
	}
	~SimPauseGuard() {
		{
			std::lock_guard<std::mutex> lk(sim.cvMutex);
			sim.pauseRequest.store(false, std::memory_order_release);
			sim.cv.notify_all();
		}
	}

private:
	LogicSimulator& sim;
};

#endif // logicSimulator_h