#ifndef evalSimulator_h
#define evalSimulator_h

#include "gateSubstituter.h"

class EvalSimulator {
public:
	EvalSimulator(EvalConfig& evalConfig, IdProvider<middle_id_t>& middleIdProvider, std::vector<simulator_id_t>& dirtySimulatorIds) :
		evalConfig(evalConfig),
		middleIdProvider(middleIdProvider),
		gateSubstituter(evalConfig, middleIdProvider, dirtySimulatorIds) {}
	inline SimPauseGuard beginEdit() {
		return gateSubstituter.beginEdit();
	}
	inline void endEdit(SimPauseGuard& pauseGuard) {
		gateSubstituter.endEdit(pauseGuard);
	}
	inline void addGate(SimPauseGuard& pauseGuard, const GateType gateType, const middle_id_t gateId) {
		gateSubstituter.addGate(pauseGuard, gateType, gateId);
	}
	inline void removeGate(SimPauseGuard& pauseGuard, const middle_id_t gateId) {
		gateSubstituter.removeGate(pauseGuard, gateId);
	}
	inline logic_state_t getState(EvalConnectionPoint point) const {
		return gateSubstituter.getState(point);
	}
	inline std::vector<logic_state_t> getStates(const std::vector<EvalConnectionPoint>& points) const {
		return gateSubstituter.getStates(points);
	}
	inline std::vector<logic_state_t> getPinStates(const std::vector<EvalConnectionPoint>& points) const {
		return gateSubstituter.getPinStates(points);
	}
	inline std::vector<logic_state_t> getStatesFromSimulatorIds(const std::vector<simulator_id_t>& simulatorIds) const {
		return gateSubstituter.getStatesFromSimulatorIds(simulatorIds);
	}
	inline std::vector<SimulatorStateAndPinSimId> getSimulatorIds(const std::vector<EvalConnectionPoint>&points) const {
		return gateSubstituter.getSimulatorIds(points);
	}
	inline std::vector<simulator_id_t> getBlockSimulatorIds(const std::vector<std::optional<EvalConnectionPoint>>& points) const {
		return gateSubstituter.getBlockSimulatorIds(points);
	}
	inline std::vector<simulator_id_t> getPinSimulatorIds(const std::vector<std::optional<EvalConnectionPoint>>& points) const {
		return gateSubstituter.getPinSimulatorIds(points);
	}
	inline void setState(EvalConnectionPoint point, logic_state_t state) {
		gateSubstituter.setState(point, state);
	}
	inline void setStates(const std::vector<EvalConnectionPoint>& points, const std::vector<logic_state_t>& states) {
		gateSubstituter.setStates(points, states);
	}
	inline void makeConnection(SimPauseGuard& pauseGuard, EvalConnection connection) {
		gateSubstituter.makeConnection(pauseGuard, connection);
	}
	inline void removeConnection(SimPauseGuard& pauseGuard, EvalConnection connection) {
		gateSubstituter.removeConnection(pauseGuard, connection);
	}
	inline double getAverageTickrate() const {
		return gateSubstituter.getAverageTickrate();
	}
private:
	EvalConfig& evalConfig;
	IdProvider<middle_id_t>& middleIdProvider;
	GateSubstituter gateSubstituter;
};

#endif /* evalSimulator_h */
