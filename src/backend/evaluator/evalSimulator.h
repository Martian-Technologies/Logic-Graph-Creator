#ifndef evalSimulator_h
#define evalSimulator_h

#include "gateSubstituter.h"

class EvalSimulator {
public:
	EvalSimulator(EvalConfig& evalConfig, IdProvider<middle_id_t>& middleIdProvider) :
		evalConfig(evalConfig),
		middleIdProvider(middleIdProvider), gateSubstituter(evalConfig, middleIdProvider) {
	}
	SimPauseGuard beginEdit() {
		return gateSubstituter.beginEdit();
	}
	void endEdit(SimPauseGuard& pauseGuard) {
		gateSubstituter.endEdit(pauseGuard);
	}
	void addGate(SimPauseGuard& pauseGuard, const GateType gateType, const middle_id_t gateId) {
		gateSubstituter.addGate(pauseGuard, gateType, gateId);
	}
	void removeGate(SimPauseGuard& pauseGuard, const middle_id_t gateId) {
		gateSubstituter.removeGate(pauseGuard, gateId);
	}
	logic_state_t getState(EvalConnectionPoint point) const {
		return gateSubstituter.getState(point);
	}
	std::vector<logic_state_t> getStates(const std::vector<EvalConnectionPoint>& points) const {
		return gateSubstituter.getStates(points);
	}
	void setState(middle_id_t id, logic_state_t state) {
		gateSubstituter.setState(id, state);
	}
	void setStates(const std::vector<middle_id_t>& ids, const std::vector<logic_state_t>& states) {
		gateSubstituter.setStates(ids, states);
	}
	void makeConnection(SimPauseGuard& pauseGuard, EvalConnection connection) {
		gateSubstituter.makeConnection(pauseGuard, connection);
	}
	void removeConnection(SimPauseGuard& pauseGuard, const EvalConnection& connection) {
		gateSubstituter.removeConnection(pauseGuard, connection);
	}
	unsigned int getAverageTickrate() const {
		return gateSubstituter.getAverageTickrate();
	}
private:
	EvalConfig& evalConfig;
	IdProvider<middle_id_t>& middleIdProvider;
	GateSubstituter gateSubstituter;
};

#endif // evalSimulator_h