#ifndef logicState_h
#define logicState_h

enum class logic_state_t : unsigned char {
	LOW = 0,
	HIGH = 1,
	FLOATING = 2,
	UNDEFINED = 3
};

inline bool isHigh(const logic_state_t& state) {
	return state == logic_state_t::HIGH;
}

inline bool isLow(const logic_state_t& state) {
	return state == logic_state_t::LOW;
}

inline bool isValid(const logic_state_t& state) {
	return state == logic_state_t::HIGH || state == logic_state_t::LOW;
}

inline bool toBool(const logic_state_t& state) {
	return state == logic_state_t::HIGH;
}

inline logic_state_t fromBool(bool value) {
	return value ? logic_state_t::HIGH : logic_state_t::LOW;
}

#endif /* logicState_h */
