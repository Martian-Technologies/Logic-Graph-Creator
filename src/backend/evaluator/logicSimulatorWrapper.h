#ifndef logicSimulatorWrapper_h
#define logicSimulatorWrapper_h

#include "logicSimulator.h"

typedef unsigned int wrapper_gate_id_t;

class LogicSimulatorWrapper {
public:
	LogicSimulatorWrapper();
	void initialize() { logicSimulator.initialize(); }
	wrapper_gate_id_t createGate(const GateType& gateType, bool allowSubstituteDecomissioned = true);
	void deleteGate(wrapper_gate_id_t gateId);
	void connectGates(wrapper_gate_id_t sourceGate, size_t outputGroup, wrapper_gate_id_t targetGate, size_t inputGroup);
	void disconnectGates(wrapper_gate_id_t sourceGate, size_t outputGroup, wrapper_gate_id_t targetGate, size_t inputGroup);

	logic_state_t getState(wrapper_gate_id_t gateId, size_t outputGroup) const;
	void setState(wrapper_gate_id_t gateId, size_t outputGroup, logic_state_t state);

	void signalToPause() { logicSimulator.signalToPause(); }
	void signalToProceed();
	bool threadIsWaiting() const { return logicSimulator.threadIsWaiting(); }

	long long int getRealTickrate() const { return logicSimulator.getRealTickrate(); }
	void setTargetTickrate(unsigned long long tickrate) { logicSimulator.setTargetTickrate(tickrate); }
	void triggerNextTickReset() {logicSimulator.triggerNextTickReset();}
private:
	struct BufferGate {
		wrapper_gate_id_t gateId;
		std::vector<wrapper_gate_id_t> bufferInputs;
		std::vector<wrapper_gate_id_t> bufferOutputs;
		std::vector<std::pair<simulator_gate_id_t, size_t>> externalInputs;
		std::vector<std::pair<simulator_gate_id_t, size_t>> externalOutputs;
		simulator_gate_id_t bufferGateId;
	};
	LogicSimulator logicSimulator;
	std::vector<std::optional<simulator_gate_id_t>> wrapperToSimulatorGateIdMap;
	std::vector<BufferGate> bufferGates;
	std::unordered_set<wrapper_gate_id_t> allBufferGateIds;

	BufferGate& getBufferGate(wrapper_gate_id_t gateId) {
		auto it = std::find_if(bufferGates.begin(), bufferGates.end(), [gateId](const BufferGate& bg) {
			return bg.gateId == gateId;
			});
		if (it != bufferGates.end()) {
			return *it;
		}
		throw std::runtime_error("Buffer gate not found");
	};
	std::vector<wrapper_gate_id_t> findConnectedBufferGates(BufferGate& bufferGate);
	void recreateBuffers(std::vector<wrapper_gate_id_t>& allBufferGateIdsToRemake);

	void debugPrintBufferGates();
};

#endif // logicSimulatorWrapper_h