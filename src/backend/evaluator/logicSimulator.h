#ifndef logicSimulator_h
#define logicSimulator_h

#include "simulatorGates.h"

class LogicSimulator {
friend class SimulatorOptimizer;
public:
	LogicSimulator();
	~LogicSimulator();
	void clearState();
	void setTargetTickrate(unsigned int targetTickrate);
	unsigned int getAverageTickrate();
	void setPaused(bool paused);
	void setState(simulator_id_t id, logic_state_t state);
	void setStates(const std::vector<simulator_id_t>& ids, const std::vector<logic_state_t>& states);
	logic_state_t getState(simulator_id_t id);
	std::vector<logic_state_t> getStates(const std::vector<simulator_id_t>& ids);

private:
	std::thread simulationThread;
	std::atomic<bool> running;

	std::atomic<bool> paused{false};
	std::atomic<unsigned> targetTickrate{40*60}; // ticks per minute

	std::mutex cvMutex;
	std::condition_variable cv;

	std::vector<logic_state_t> statesA;
	std::vector<logic_state_t> statesB;
	std::mutex statesAMutex;
	std::mutex statesBMutex;

	std::vector<ANDLikeGate> andGates;
	std::vector<XORLikeGate> xorGates;
	std::vector<JunctionGate> junctions;
	std::vector<BufferGate> buffers;
	std::vector<SingleBufferGate> singleBuffers;
	std::vector<TristateBufferGate> tristateBuffers;
	// std::vector<ConstantGate> constantGates;
	std::vector<ConstantResetGate> constantResetGates; // for tick buttons mainly
	std::vector<CopySelfOutputGate> copySelfOutputGates;

	void simulationLoop();
	void tickOnce();
};

#endif // logicSimulator_h