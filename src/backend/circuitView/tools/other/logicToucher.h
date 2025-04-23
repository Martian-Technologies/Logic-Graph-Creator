#ifndef logicToucher_h
#define logicToucher_h

#include "../circuitTool.h"

class LogicToucher : public CircuitTool {
public:
	void activate() override final;
	static inline std::vector<std::string> getModes_() { return {}; }
	static inline std::string getPath_() { return "interactive/state changer"; }
	inline std::string getPath() const override final { return getPath_(); }
	inline unsigned int getStackId() const override final { return 1; }

	inline void reset() override final { clicked = false; elementCreator.clear(); }
	bool press(const Event* event);
	bool unpress(const Event* event);
	bool pointerMove(const Event* event);

private:

	bool clicked = false;
	Position clickPosition;
};

#endif /* logicToucher_h */
