#ifndef evalSimulator_h
#define evalSimulator_h
#ifdef TRACY_PROFILER
#include <tracy/Tracy.hpp>
#endif

#include "gateSubstituter.h"

class EvalSimulator {
public:
	EvalSimulator(EvalConfig& evalConfig, IdProvider<middle_id_t>& middleIdProvider) :
		evalConfig(evalConfig),
		middleIdProvider(middleIdProvider), gateSubstituter(evalConfig, middleIdProvider) {
	}
	inline SimPauseGuard beginEdit() {
		#ifdef TRACY_PROFILER
			ZoneScoped;
		#endif
		return gateSubstituter.beginEdit();
	}
	inline void endEdit(SimPauseGuard& pauseGuard) {
		#ifdef TRACY_PROFILER
			ZoneScoped;
		#endif
		gateSubstituter.endEdit(pauseGuard);
	}
	inline void addGate(SimPauseGuard& pauseGuard, const GateType gateType, const middle_id_t gateId) {
		#ifdef TRACY_PROFILER
			ZoneScoped;
		#endif
		gateSubstituter.addGate(pauseGuard, gateType, gateId);
	}
	inline void removeGate(SimPauseGuard& pauseGuard, const middle_id_t gateId) {
		#ifdef TRACY_PROFILER
			ZoneScoped;
		#endif
		gateSubstituter.removeGate(pauseGuard, gateId);
	}
	inline logic_state_t getState(EvalConnectionPoint point) const {
		#ifdef TRACY_PROFILER
			ZoneScoped;
		#endif
		return gateSubstituter.getState(point);
	}
	inline std::vector<logic_state_t> getStates(const std::vector<EvalConnectionPoint>& points) const {
		#ifdef TRACY_PROFILER
			ZoneScoped;
		#endif
		return gateSubstituter.getStates(points);
	}
	inline void setState(EvalConnectionPoint point, logic_state_t state) {
		#ifdef TRACY_PROFILER
			ZoneScoped;
		#endif
		gateSubstituter.setState(point, state);
	}
	inline void setStates(const std::vector<EvalConnectionPoint>& points, const std::vector<logic_state_t>& states) {
		#ifdef TRACY_PROFILER
			ZoneScoped;
		#endif
		gateSubstituter.setStates(points, states);
	}
	inline void makeConnection(SimPauseGuard& pauseGuard, EvalConnection connection) {
		#ifdef TRACY_PROFILER
			ZoneScoped;
		#endif
		gateSubstituter.makeConnection(pauseGuard, connection);
	}
	inline void removeConnection(SimPauseGuard& pauseGuard, EvalConnection connection) {
		#ifdef TRACY_PROFILER
			ZoneScoped;
		#endif
		gateSubstituter.removeConnection(pauseGuard, connection);
	}
	inline unsigned int getAverageTickrate() const {
		#ifdef TRACY_PROFILER
			ZoneScoped;
		#endif
		return gateSubstituter.getAverageTickrate();
	}
private:
	EvalConfig& evalConfig;
	IdProvider<middle_id_t>& middleIdProvider;
	GateSubstituter gateSubstituter;
};

#endif // evalSimulator_h