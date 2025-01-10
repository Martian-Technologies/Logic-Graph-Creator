#ifndef logicSimulator_h
#define logicSimulator_h

#include <atomic>
#include <chrono>
#include <thread>

#include "evaluatorDefs.h"
#include "backend/container/block/blockDefs.h"

class LogicSimulator {
public:
	LogicSimulator();
	~LogicSimulator();
	void initialize();
	eval_gate_id_t addGate(const GateType& gateType, bool allowSubstituteDecomissioned = true);
	void connectGates(eval_gate_id_t gate1, eval_gate_id_t gate2);
	void disconnectGates(eval_gate_id_t gate1, eval_gate_id_t gate2);
	void decomissionGate(eval_gate_id_t gate); // TODO: figure out a better way to do this maybe

	std::unordered_map<eval_gate_id_t, eval_gate_id_t> compressGates();

	void computeNextState();
	void propagatePowered();
	void swapStates();

	std::vector<logic_state_t> getCurrentState() const { return currentState; }
	void clearGates();
	void reserveGates(unsigned int numGates);

	void setState(eval_gate_id_t gate, logic_state_t state);

	void simulateNTicks(unsigned int n);

	logic_state_t getState(eval_gate_id_t gate) const { return currentState[gate]; }

	void debugPrint();
	void signalToPause();
	void signalToProceed();
	bool threadIsWaiting() const;

	long long int getRealTickrate() const { return realTickrate.load(std::memory_order_acquire); }
	void setTargetTickrate(unsigned long long tickrate);
	void triggerNextTickReset();

private:
	std::vector<logic_state_t> currentState, nextState;
	std::vector<GateType> gateTypes;
	std::vector<std::vector<eval_gate_id_t>> gateInputs, gateOutputs;
	std::vector<unsigned int> gateInputCountTotal, gateInputCountPowered;
	int numDecomissioned;

	// shit for threading
	std::thread tickrateMonitorThread;
	std::thread simulationThread;
	std::atomic<bool> running;
	std::atomic<bool> proceedFlag;
	std::atomic<bool> isWaiting;
	std::atomic<int> ticksRun;
	std::atomic<long long int> realTickrate;

	std::atomic<unsigned long long int> targetTickrate; // updates per minute

	std::atomic<int64_t> nextTick_us;

	void simulationLoop();
	void tickrateMonitor();
};

#endif // logicSimulator_h
