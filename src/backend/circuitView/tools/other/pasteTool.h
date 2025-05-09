#ifndef pasteTool_h
#define pasteTool_h

#include "../circuitTool.h"
#include "backend/circuit/parsedCircuit.h"
#include "computerAPI/circuits/circuitFileManager.h"
class Backend;

class PasteTool : public CircuitTool {
public:
	static inline std::vector<std::string> getModes_() { return {}; }
	static inline std::string getPath_() { return "paste tool"; }
	inline std::string getPath() const override final { return getPath_(); }
	inline unsigned int getStackId() const override final { return 1; }

	void activate() override final;
	void updateElements() override final;
	bool place(const Event* event);

private:
	bool validatePlacement() const;
};

#endif /* pasteTool_h */ 
