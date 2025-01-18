#ifndef evaluator_h
#define evaluator_h

#include "logicSimulator.h"
#include "backend/circuit/circuit.h"
#include "backend/container/difference.h"
#include "addressTree.h"
#include "backend/address.h"
#include "evaluatorDefs.h"

class Evaluator {
public:
	Evaluator(SharedCircuit circuit);
	// pause/unpause used once the evaluator is "started" 
	void setPause(bool pause);
	void reset();
	void setTickrate(double tickrate);
	void setUseTickrate(bool useTickrate);
	double getRealTickrate() const;
	void runNTicks(unsigned long long n);
	void makeEdit(DifferenceSharedPtr difference, circuit_id_t containerId);
	logic_state_t getState(const Address& address);
	void setState(const Address& address, logic_state_t state);
	std::vector<logic_state_t> getBulkStates(const std::vector<Address>& addresses);
	std::vector<logic_state_t> getBulkStates(const std::vector<Address>& addresses, const Address& addressOrigin);
	void setBulkStates(const std::vector<Address>& addresses, const std::vector<logic_state_t>& states);
	void setBulkStates(const std::vector<Address>& addresses, const std::vector<logic_state_t>& states, const Address& addressOrigin);

private:
	bool paused;
	bool usingTickrate;
	double targetTickrate;
	LogicSimulator logicSimulator;
	AddressTreeNode<eval_gate_id_t> gateTree;
	AddressTreeNode<input_socket_id_t> inputSocketTree;
	AddressTreeNode<output_socket_id_t> outputSocketTree;
};

GateType circuitToEvaluatorGatetype(BlockType blockType);

#endif // evaluator_h
