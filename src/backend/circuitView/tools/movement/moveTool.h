#ifndef moveTool_h
#define moveTool_h

#include "../circuitTool.h"
#include "../selectionHelpers/selectionHelperTool.h"

class MoveTool : public CircuitTool {
public:
	MoveTool();

	void reset() override final;
	void activate() override final;

	inline std::vector<std::string> getModes() { return { "Area", "Tensor" }; }
	void setMode(std::string toolMode) override final;

	void updateElements() override final;

	bool click(const Event* event);
	bool unclick(const Event* event);

private:
	std::string mode = "None";
	SharedSelectionHelperTool activeSelectionHelper = nullptr;
};

#endif /* moveTool_h */
