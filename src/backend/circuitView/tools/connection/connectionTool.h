#ifndef connectionTool_h
#define connectionTool_h

#include "../circuitTool.h"

class ConnectionTool : public CircuitTool {
public:
	ConnectionTool();

	void activate() override final;

	inline std::vector<std::string> getModes() { return { "Single", "Tensor" }; }
	void setMode(std::string toolMode) override final;

private:
	std::string mode = "None";
	SharedCircuitTool activeConnectionTool = nullptr;
};

typedef std::shared_ptr<ConnectionTool> SharedConnectionTool;

#endif /* connectionTool_h */
