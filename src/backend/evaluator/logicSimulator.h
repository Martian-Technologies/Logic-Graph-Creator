#ifndef logicSimulator_h
#define logicSimulator_h

#include "simulatorGates.h"
#include "gateType.h"
#include "idProvider.h"
#include "evalConnection.h"
#include "evalConfig.h"

// Gate type indices for performance optimization
enum class SimGateType : int {
	AND = 0,
	XOR = 1,
	JUNCTION = 2,
	BUFFER = 3,
	SINGLE_BUFFER = 4,
	TRISTATE_BUFFER = 5,
	CONSTANT = 6,
	CONSTANT_RESET = 7,
	COPY_SELF_OUTPUT = 8
};

class LogicSimulator {
friend class SimulatorOptimizer;
friend class SimPauseGuard;
public:
	LogicSimulator(
		EvalConfig& evalConfig,
		std::vector<simulator_id_t>& dirtySimulatorIds);
	~LogicSimulator();
	void clearState();
	float getAverageTickrate() const;
	void setState(simulator_id_t id, logic_state_t state);
	void setStates(const std::vector<simulator_id_t>& ids, const std::vector<logic_state_t>& states);

	void setStateImmediate(simulator_id_t id, logic_state_t state);
	void setStatesImmediate(const std::vector<simulator_id_t>& ids, const std::vector<logic_state_t>& states);

	logic_state_t getState(simulator_id_t id) const;
	std::vector<logic_state_t> getStates(const std::vector<simulator_id_t>& ids) const;

	simulator_id_t addGate(const GateType gateType);
	void removeGate(simulator_id_t gateId);
	void makeConnection(simulator_id_t sourceId, connection_port_id_t sourcePort, simulator_id_t destinationId, connection_port_id_t destinationPort);
	void removeConnection(simulator_id_t sourceId, connection_port_id_t sourcePort, simulator_id_t destinationId, connection_port_id_t destinationPort);
	std::optional<simulator_id_t> getOutputPortId(simulator_id_t simId, connection_port_id_t portId) const;

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
	std::thread simulationThread;
	std::atomic<bool> running { true };

	std::atomic<bool> pauseRequest { false };
	std::atomic<bool> isPaused { false };
	std::mutex cvMutex;
	std::condition_variable cv;

	std::vector<logic_state_t> statesA;
	std::vector<logic_state_t> statesB;

	mutable std::shared_mutex statesAMutex;
	std::mutex statesBMutex;

	struct StateChange {
		simulator_id_t id;
		logic_state_t state;
	};
	std::queue<StateChange> pendingStateChanges;
	std::mutex stateChangeQueueMutex;

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

	struct GateDependency {
		SimGateType gateType;
		size_t gateIndex;

		GateDependency(SimGateType type, size_t index) : gateType(type), gateIndex(index) {}

		bool operator==(const GateDependency& other) const {
			return gateType == other.gateType && gateIndex == other.gateIndex;
		}
	};

	struct GateLocation {
		SimGateType gateType;
		size_t gateIndex;

		GateLocation() : gateType(SimGateType::AND), gateIndex(0) {}
		GateLocation(SimGateType type, size_t index) : gateType(type), gateIndex(index) {}
	};

	std::unordered_map<simulator_id_t, std::vector<GateDependency>> outputDependencies;
	std::unordered_map<simulator_id_t, GateLocation> gateLocations;

	void simulationLoop();
	inline void tickOnce();
	inline void realisticTickOnce();
	void processPendingStateChanges();

	void addInputToGate(simulator_id_t simId, simulator_id_t inputId, connection_port_id_t portId);
	void removeInputFromGate(simulator_id_t simId, simulator_id_t inputId, connection_port_id_t portId);
	std::optional<std::vector<simulator_id_t>> getOutputSimIdsFromGate(simulator_id_t simId) const;

	void updateGateLocation(simulator_id_t gateId, SimGateType gateType, size_t gateIndex);
	void removeGateLocation(simulator_id_t gateId);
	void addOutputDependency(simulator_id_t outputId, SimGateType gateType, size_t gateIndex);
	void removeOutputDependency(simulator_id_t outputId, SimGateType gateType, size_t gateIndex);
	void updateGateIndicesAfterRemoval(SimGateType gateType, size_t removedIndex);

	std::atomic<float> averageTickrate { 0.0 };
	float tickrateHalflife { 0.25 };

	std::vector<simulator_id_t>& dirtySimulatorIds;
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

#endif /* logicSimulator_h */
