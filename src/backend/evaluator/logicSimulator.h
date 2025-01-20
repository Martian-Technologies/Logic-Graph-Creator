#ifndef logicSimulator_h
#define logicSimulator_h

#include <barrier>
#include <thread>
#include <variant>

#include "evaluatorDefs.h"

struct SocketsBothVector {
	std::vector<input_socket_id_t> inputSockets;
	std::vector<output_socket_id_t> outputSockets;
};

struct SocketsSingle {
	input_socket_id_t inputSocket;
	output_socket_id_t outputSocket;
};

struct SocketsInputVector {
	std::vector<input_socket_id_t> inputSockets;
	output_socket_id_t outputSocket;
};

struct SocketsOutputVector {
	input_socket_id_t inputSocket;
	std::vector<output_socket_id_t> outputSockets;
};

struct Gate {
	GateType type;
	std::variant<SocketsBothVector, SocketsSingle, SocketsInputVector, SocketsOutputVector> sockets;
	std::vector<input_socket_id_t> getInputSockets();
	std::vector<output_socket_id_t> getOutputSockets();
};

class LogicSimulator {
public:
	LogicSimulator();
	~LogicSimulator();

	output_socket_id_t registerOutputSocket();
	std::vector<output_socket_id_t> registerOutputSockets(unsigned int count);
	input_socket_id_t registerInputSocket();
	std::vector<input_socket_id_t> registerInputSockets(unsigned int count);

	eval_gate_id_t registerGate(input_socket_id_t inputSocket, output_socket_id_t outputSocket, GateType type);
	eval_gate_id_t registerGate(input_socket_id_t inputSocket, std::vector<output_socket_id_t> outputSockets, GateType type);
	eval_gate_id_t registerGate(std::vector<input_socket_id_t> inputSockets, output_socket_id_t outputSocket, GateType type);
	eval_gate_id_t registerGate(std::vector<input_socket_id_t> inputSockets, std::vector<output_socket_id_t> outputSockets, GateType type);

	void decomissionGate(eval_gate_id_t gateId);
	void connect(output_socket_id_t sourceSocket, input_socket_id_t destinationSocket);
	void disconnect(output_socket_id_t sourceSocket, input_socket_id_t destinationSocket);
	void signalToPause();
	void signalToResume();
	void waitForPause();

	std::unordered_map<eval_gate_id_t, eval_gate_id_t> compressGates();

	void setTickrate(double tickrate);
	void setSprint(bool sprint);

	Gate getGate(eval_gate_id_t gateId) { return gates[gateId]; }
	logic_state_t getOutputSocketState(output_socket_id_t outputSocket) { return nextOutputSockets[outputSocket]; }
	logic_state_t getInputSocketState(input_socket_id_t inputSocket) { return inputCountSockets[inputSocket]; }
	void setOutputSocketState(output_socket_id_t outputSocket, logic_state_t state);

private:
	std::vector<logic_state_t> currentOutputSockets;
	std::vector<logic_state_t> nextOutputSockets;
	std::vector<unsigned int> inputCountSockets;
	std::vector<unsigned int> inputTotalCountSockets;
	std::vector<std::vector<input_socket_id_t>> connectionDestinations;
	std::vector<Gate> gates;

	std::atomic<bool> running;

	std::atomic<bool> pause;
	std::atomic<bool> paused;
	std::atomic<double> tickrate;
	std::atomic<bool> sprint;
	std::atomic<int64_t> nextTick_us;

	std::thread thread;

	void simulationLoop();
	void calculateStates();
	void propagateStates();

	inline void calculateStateBasic(unsigned int type, input_socket_id_t input, output_socket_id_t output);
};

#endif // logicSimulator_h