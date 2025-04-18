#ifndef logicSimulator_h
#define logicSimulator_h

#include <atomic>
#include <chrono>
#include <thread>
#include <condition_variable>
#include <shared_mutex>

#include "logicState.h"
#include "gateType.h"
#include "gate.h"
#include "backend/container/block/blockDefs.h"

typedef unsigned int simulator_gate_id_t;

class LogicSimulator {
public:
	LogicSimulator();
	~LogicSimulator();
	void initialize();

	simulator_gate_id_t addGate(const GateType& gateType, bool allowSubstituteDecomissioned = true);

	void connectGates(simulator_gate_id_t sourceGate, size_t outputGroup, simulator_gate_id_t targetGate, size_t inputGroup);

	void disconnectGates(simulator_gate_id_t sourceGate, size_t outputGroup, simulator_gate_id_t targetGate, size_t inputGroup);

	void decomissionGate(simulator_gate_id_t gate);
	std::unordered_map<simulator_gate_id_t, simulator_gate_id_t> compressGates();

	void computeNextState();
	void swapStates();

	std::vector<std::vector<logic_state_t>> getCurrentState() const;
	void clearGates();

	void setState(simulator_gate_id_t gate, size_t outputGroup, logic_state_t state);

	logic_state_t getState(simulator_gate_id_t gate, size_t outputGroup) const;

	void signalToPause();
	void signalToProceed();
	bool threadIsWaiting() const;

	long long int getRealTickrate() const;
	void setTargetTickrate(unsigned long long tickrate);
	void triggerNextTickReset();

	int getDecomissionedCount() const {
		return numDecomissioned;
	}

	std::unique_lock<std::shared_mutex> getSimulationUniqueLock() {
		return std::unique_lock<std::shared_mutex>(simulationMutex);
	}
	std::shared_lock<std::shared_mutex> getSimulationSharedLock() {
		return std::shared_lock<std::shared_mutex>(simulationMutex);
	}

private:
	std::vector<Gate> gates;
	int numDecomissioned;
	int rollingAvgLength;
	int rollingAvgIndex;
	std::vector<int> rollingAvg;

	void updateRollingAverage(int newValue);

	std::thread tickrateMonitorThread;
	std::thread simulationThread;
	std::atomic<bool> running;
	std::atomic<bool> proceedFlag;
	std::atomic<bool> isWaiting;
	std::atomic<int> ticksRun;
	std::atomic<long long int> realTickrate;
	std::atomic<unsigned long long int> targetTickrate;
	std::atomic<int64_t> nextTick_us;

	mutable std::shared_mutex simulationMutex;
	std::mutex simulationMutex2;
	std::condition_variable simulationCv;
	std::mutex killThreadsMux;
	std::condition_variable killThreadsCv;

	void simulationLoop();
	void tickrateMonitor();

	void computeGateStates(Gate& gate);
	void computeJunctionStates(Gate& gate);
};

#endif // logicSimulator_h