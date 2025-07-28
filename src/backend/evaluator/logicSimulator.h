#ifndef logicSimulator_h
#define logicSimulator_h

#include "simulatorGates.h"
#include "gateType.h"
#include "idProvider.h"
#include "evalConnection.h"
#include "evalConfig.h"

class LogicSimulator {
friend class SimulatorOptimizer;
friend class SimPauseGuard;
public:
	LogicSimulator(EvalConfig& evalConfig);
	~LogicSimulator();
	void clearState();
	unsigned int getAverageTickrate() const;
	void setState(simulator_id_t id, logic_state_t state);
	void setStates(const std::vector<simulator_id_t>& ids, const std::vector<logic_state_t>& states);
	logic_state_t getState(simulator_id_t id) const;
	std::vector<logic_state_t> getStates(const std::vector<simulator_id_t>& ids) const;

	simulator_id_t addGate(const GateType gateType);
	void removeGate(simulator_id_t gateId);
	void makeConnection(simulator_id_t sourceId, connection_port_id_t sourcePort, simulator_id_t destinationId, connection_port_id_t destinationPort);
	void removeConnection(simulator_id_t sourceId, connection_port_id_t sourcePort, simulator_id_t destinationId, connection_port_id_t destinationPort);
	std::optional<simulator_id_t> getOutputPortId(simulator_id_t simId, connection_port_id_t portId) const;

	// EvalConfig integration methods
	bool isRunning() const { return evalConfig.isRunning(); }
	void setRunning(bool running) { evalConfig.setRunning(running); }
	bool isTickrateLimiterEnabled() const { return evalConfig.isTickrateLimiterEnabled(); }
	void setTickrateLimiter(bool enabled) { evalConfig.setTickrateLimiter(enabled); }
	long long getTargetTickrate() const { return evalConfig.getTargetTickrate(); }
	void setTargetTickrate(long long tickrate) { evalConfig.setTargetTickrate(tickrate); }
	bool isRealistic() const { return evalConfig.isRealistic(); }
	void setRealistic(bool realistic) { evalConfig.setRealistic(realistic); }

private:
	EvalConfig& evalConfig;
	std::jthread simulationThread;
	std::atomic<bool> running { true };

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

	IdProvider<simulator_id_t> simulatorIdProvider;

	void simulationLoop();
	inline void tickOnce();

	void addInputToGate(simulator_id_t simId, simulator_id_t inputId, connection_port_id_t portId);
	void removeInputFromGate(simulator_id_t simId, simulator_id_t inputId, connection_port_id_t portId);
	std::optional<std::vector<simulator_id_t>> getOutputSimIdsFromGate(simulator_id_t simId) const;
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