#include "logicSimulator.h"

LogicSimulator::LogicSimulator()
    : numThreads(4),
    sync_after_state(numThreads),
    sync_after_propagation(numThreads),
    sync_after_swap(numThreads) {
}

output_socket_id_t LogicSimulator::registerOutputSocket() {
    output_socket_id_t id = currentOutputSockets.size();
    currentOutputSockets.push_back(false);
    nextOutputSockets.push_back(false);
    return id;
}

std::vector<output_socket_id_t> LogicSimulator::registerOutputSockets(unsigned int count) {
    std::vector<output_socket_id_t> ids;
    for (unsigned int i = 0; i < count; i++) {
        ids.push_back(registerOutputSocket());
    }
    return ids;
}

input_socket_id_t LogicSimulator::registerInputSocket(unsigned int count) {
    input_socket_id_t id = inputCountSockets.size();
    inputCountSockets.push_back(count);
    return id;
}

std::vector<input_socket_id_t> LogicSimulator::registerInputSockets(unsigned int count) {
    std::vector<input_socket_id_t> ids;
    for (unsigned int i = 0; i < count; i++) {
        ids.push_back(registerInputSocket(0));
    }
    return ids;
}

eval_gate_id_t LogicSimulator::registerGate(input_socket_id_t inputSocket, output_socket_id_t outputSocket, GateType type) {
    gates.push_back(GateSingle{ type, inputSocket, outputSocket });
    return gates.size() - 1;
}

eval_gate_id_t LogicSimulator::registerGate(input_socket_id_t inputSocket, std::vector<output_socket_id_t> outputSockets, GateType type) {
    gates.push_back(GateOutputVector{ type, inputSocket, outputSockets });
    return gates.size() - 1;
}

eval_gate_id_t LogicSimulator::registerGate(std::vector<input_socket_id_t> inputSockets, output_socket_id_t outputSocket, GateType type) {
    gates.push_back(GateInputVector{ type, inputSockets, outputSocket });
    return gates.size() - 1;
}

eval_gate_id_t LogicSimulator::registerGate(std::vector<input_socket_id_t> inputSockets, std::vector<output_socket_id_t> outputSockets, GateType type) {
    gates.push_back(GateBothVector{ type, inputSockets, outputSockets });
    return gates.size() - 1;
}