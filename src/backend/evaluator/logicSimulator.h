#ifndef logicSimulator_h
#define logicSimulator_h

#include "simulatorGates.h"
#include "gateType.h"
#include "idProvider.h"
#include "evalConnection.h"
#include "evalConfig.h"

enum class SimGateType : int {
	AND = 0,
	XOR = 1,
	JUNCTION = 2,
	TRISTATE_BUFFER = 3,
	CONSTANT = 4,
	CONSTANT_RESET = 5,
	COPY_SELF_OUTPUT = 6
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
	double getAverageTickrate() const;
	void setState(simulator_id_t id, logic_state_t state);

	logic_state_t getState(simulator_id_t id) const;
	std::vector<logic_state_t> getStates(const std::vector<simulator_id_t>& ids) const;
	std::optional<simulator_id_t> getInputPortId(simulator_id_t simId, connection_port_id_t portId) const;
	std::optional<simulator_id_t> getOutputPortId(simulator_id_t simId, connection_port_id_t portId) const;

	simulator_id_t addGate(const GateType gateType);
	void removeGate(simulator_id_t gateId);
	void makeConnection(simulator_id_t sourceId, connection_port_id_t sourcePort, simulator_id_t destinationId, connection_port_id_t destinationPort);
	void removeConnection(simulator_id_t sourceId, connection_port_id_t sourcePort, simulator_id_t destinationId, connection_port_id_t destinationPort);
	void endEdit();

	struct GateLocation {
		SimGateType gateType;
		size_t gateIndex;

		GateLocation() : gateType(SimGateType::AND), gateIndex(0) {}
		GateLocation(SimGateType type, size_t index) : gateType(type), gateIndex(index) {}
	};

	std::unordered_map<simulator_id_t, GateLocation> gateLocations;

private:
	EvalConfig& evalConfig;
	std::thread simulationThread;
	std::atomic<bool> running { true };

	std::atomic<bool> pauseRequest { false };
	std::atomic<bool> isPaused { false };
	std::mutex cvMutex;
	std::condition_variable cv;

	std::vector<logic_state_t> statesReading;
	std::vector<logic_state_t> statesWriting;
	std::vector<unsigned int> countL;
	std::vector<unsigned int> countH;
	std::vector<unsigned int> countZ;
	std::vector<unsigned int> countX;

	mutable std::shared_mutex statesReadingMutex;
	std::mutex statesWritingMutex;

	struct StateChange {
		simulator_id_t id;
		logic_state_t state;
	};
	std::queue<StateChange> pendingStateChanges;
	std::mutex stateChangeQueueMutex;

	std::vector<ANDLikeGate> andGates;
	std::vector<XORLikeGate> xorGates;
	// std::vector<JunctionGate> junctions;
	// std::vector<TristateBufferGate> tristateBuffers;
	// std::vector<ConstantGate> constantGates;
	// std::vector<ConstantResetGate> constantResetGates;
	// std::vector<CopySelfOutputGate> copySelfOutputGates;

	IdProvider<simulator_id_t> simulatorIdProvider;

	struct GateDependency {
		simulator_id_t dependentId;
		connection_port_id_t sourcePort;
		connection_port_id_t destinationPort;

		GateDependency() : dependentId(0), sourcePort(0), destinationPort(0) {}
		explicit GateDependency(simulator_id_t id, connection_port_id_t srcPort, connection_port_id_t destPort)
			: dependentId(id), sourcePort(srcPort), destinationPort(destPort) {}

		bool operator==(const GateDependency& other) const {
			return dependentId == other.dependentId && sourcePort == other.sourcePort && destinationPort == other.destinationPort;
		}
	};

	std::unordered_map<simulator_id_t, std::vector<GateDependency>> inputDependencies;
	std::unordered_map<simulator_id_t, std::vector<GateDependency>> outputDependencies;

	void simulationLoop();
	inline void tickOnceSimple();
	inline void realisticTickOnce();
	inline void calculateNewStatesSimple();
	inline void calculateNewStatesRealistic();
	inline void propagateNewStates();
	inline void processJunctions();

	void processPendingStateChanges();

	inline void updateEmaTickrate(
		const std::chrono::steady_clock::time_point& currentTime,
		std::chrono::steady_clock::time_point& lastTickTime,
		bool& isFirstTick);

	std::atomic<double> averageTickrate { 0.0 };
	double tickrateHalflife { 0.25 };

	std::vector<simulator_id_t>& dirtySimulatorIds;

	simulator_id_t addAndGate();
	simulator_id_t addOrGate();
	simulator_id_t addNandGate();
	simulator_id_t addNorGate();
	simulator_id_t addXorGate();
	simulator_id_t addXnorGate();

	void expandDataVectors(simulator_id_t maxId);

	void addGateDependency(simulator_id_t sourceId, connection_port_id_t sourcePort, simulator_id_t destinationId, connection_port_id_t destinationPort);
	void removeGateDependency(simulator_id_t sourceId, connection_port_id_t sourcePort, simulator_id_t destinationId, connection_port_id_t destinationPort);

	std::optional<std::vector<simulator_id_t>> getOccupiedIds(simulator_id_t gateId) const;
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