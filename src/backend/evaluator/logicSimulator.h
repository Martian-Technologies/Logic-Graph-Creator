#ifndef logicSimulator_h
#define logicSimulator_h

#include <barrier>
#include <thread>
#include <variant>

#include "evaluatorDefs.h"

struct Connection {
	output_socket_id_t sourceSocket;
	std::vector<input_socket_id_t> destinationSockets;
};

struct GateBothVector {
	GateType type;
	std::vector<input_socket_id_t> inputSockets;
	std::vector<output_socket_id_t> outputSockets;
};

struct GateSingle {
	GateType type;
	input_socket_id_t inputSocket;
	output_socket_id_t outputSocket;
};

struct GateInputVector {
	GateType type;
	std::vector<input_socket_id_t> inputSockets;
	output_socket_id_t outputSocket;
};

struct GateOutputVector {
	GateType type;
	input_socket_id_t inputSocket;
	std::vector<output_socket_id_t> outputSockets;
};

using Gate = std::variant<GateBothVector, GateSingle, GateInputVector, GateOutputVector>;

class LogicSimulator {
public:
	LogicSimulator();

	output_socket_id_t registerOutputSocket();
	std::vector<output_socket_id_t> registerOutputSockets(unsigned int count);
	input_socket_id_t registerInputSocket(unsigned int count);
	std::vector<input_socket_id_t> registerInputSockets(unsigned int count);
	eval_gate_id_t registerGate(input_socket_id_t inputSocket, output_socket_id_t outputSocket, GateType type);
	eval_gate_id_t registerGate(input_socket_id_t inputSocket, std::vector<output_socket_id_t> outputSockets, GateType type);
	eval_gate_id_t registerGate(std::vector<input_socket_id_t> inputSockets, output_socket_id_t outputSocket, GateType type);
	eval_gate_id_t registerGate(std::vector<input_socket_id_t> inputSockets, std::vector<output_socket_id_t> outputSockets, GateType type);
	void connect(output_socket_id_t sourceSocket, input_socket_id_t destinationSocket);
	void disconnect(output_socket_id_t sourceSocket, input_socket_id_t destinationSocket);

private:
	std::vector<logic_state_t> currentOutputSockets;
	std::vector<logic_state_t> nextOutputSockets;
	std::vector<unsigned int> inputCountSockets;
	std::vector<Connection> connections;
	std::vector<Gate> gates;

	unsigned int numThreads;

	std::barrier<> sync_after_state;
	std::barrier<> sync_after_propagation;
	std::barrier<> sync_after_swap;
};

#endif // logicSimulator_h