#ifndef logicSimulator_h
#define logicSimulator_h

#include <atomic>
#include <chrono>
#include <thread>
#include <condition_variable>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

#include "logicState.h"
#include "gateType.h"
#include "gate.h"
#include "backend/container/block/blockDefs.h"

class LogicSimulator {
public:
	LogicSimulator();
	~LogicSimulator();
	void initialize();

	block_id_t addGate(const GateType& gateType, bool allowSubstituteDecomissioned = true);

	void connectGates(block_id_t sourceGate, size_t outputGroup, block_id_t targetGate, size_t inputGroup);
	void connectGates(block_id_t sourceGate, block_id_t targetGate, size_t inputGroup = 0);

	void disconnectGates(block_id_t sourceGate, size_t outputGroup, block_id_t targetGate, size_t inputGroup);
	void disconnectGates(block_id_t sourceGate, block_id_t targetGate, size_t inputGroup = 0);

	void decomissionGate(block_id_t gate);
	std::unordered_map<block_id_t, block_id_t> compressGates();

	void computeNextState();
	void swapStates();

	std::vector<std::vector<logic_state_t>> getCurrentState() const;
	void clearGates();
	void reserveGates(unsigned int numGates);

	void setState(block_id_t gate, size_t outputGroup, logic_state_t state);
	void setState(block_id_t gate, logic_state_t state);

	logic_state_t getState(block_id_t gate, size_t outputGroup) const;
	logic_state_t getState(block_id_t gate) const;

	void debugPrint();
	void signalToPause();
	void signalToProceed();
	bool threadIsWaiting() const;

	long long int getRealTickrate() const;
	void setTargetTickrate(unsigned long long tickrate);
	void triggerNextTickReset();

private:
	std::vector<Gate> gates;
	int numDecomissioned;

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
	void computePoolStates(Gate& gate);
};

#endif // logicSimulator_h