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
	void signalToProceed() { logicSimulator.signalToProceed(); }
	bool threadIsWaiting() const { return logicSimulator.threadIsWaiting(); }

	long long int getRealTickrate() const { return logicSimulator.getRealTickrate(); }
	void setTargetTickrate(unsigned long long tickrate) { logicSimulator.setTargetTickrate(tickrate); }
	void triggerNextTickReset() {logicSimulator.triggerNextTickReset();}
private:
	LogicSimulator logicSimulator;
};

#endif // logicSimulatorWrapper_h