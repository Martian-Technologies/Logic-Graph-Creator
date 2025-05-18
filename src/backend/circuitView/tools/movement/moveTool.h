#ifndef moveTool_h
#define moveTool_h

#include "../circuitTool.h"
#include "../selectionHelpers/selectionHelperTool.h"

class MoveTool : public CircuitTool {
public:
	void reset() override final;
	void activate() override final;

	inline std::vector<std::string> getModes() { return { }; }
	static inline std::vector<std::string> getModes_() { return { "Area", "Tensor" }; }
	static inline std::string getPath_() { return "move"; }
	inline std::vector<std::string> getModes() const override final { return getModes_(); }
	inline std::string getPath() const override final { return getPath_(); }
	void setMode(std::string toolMode) override final;

	void updateElements() override final;

	bool rotateCW(const Event* event);
	bool rotateCCW(const Event* event);
	bool click(const Event* event);
	bool unclick(const Event* event);

private:
	std::string mode = "None";
	Rotation amountToRotate = Rotation::ZERO;
	SharedSelectionHelperTool activeSelectionHelper = nullptr;
};

#endif /* moveTool_h */
