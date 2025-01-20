#include "logicSimulator.h"
#include "util.h"

std::vector<input_socket_id_t> Gate::getInputSockets() {
	// switch using variant index
	switch (sockets.index()) {
	case 0:
		return std::get<SocketsBothVector>(sockets).inputSockets;
	case 1:
		return { std::get<SocketsSingle>(sockets).inputSocket };
	case 2:
		return std::get<SocketsInputVector>(sockets).inputSockets;
	case 3:
		return { std::get<SocketsOutputVector>(sockets).inputSocket };
	default:
		UNREACHABLE();
	}
}
std::vector<output_socket_id_t> Gate::getOutputSockets() {
	switch (sockets.index()) {
	case 0:
		return std::get<SocketsBothVector>(sockets).outputSockets;
	case 1:
		return { std::get<SocketsSingle>(sockets).outputSocket };
	case 2:
		return { std::get<SocketsInputVector>(sockets).outputSocket };
	case 3:
		return std::get<SocketsOutputVector>(sockets).outputSockets;
	default:
		UNREACHABLE();
	}
}

LogicSimulator::LogicSimulator()
	: running(true),
	pause(false),
	paused(false),
	tickrate(40),
	sprint(true),
	nextTick_us(0),
	ticksRun(0),
	tickrateConveyer{0, 0, 0, 0, 0, 0, 0, 0},
	realTickrate(0),
	calculationThread(std::thread(&LogicSimulator::simulationLoop, this)),
	monitorThread(std::thread(&LogicSimulator::monitorLoop, this)) {
}

LogicSimulator::~LogicSimulator() {
	running.store(false, std::memory_order_release);
	calculationThread.join();
	monitorThread.join();
}

output_socket_id_t LogicSimulator::registerOutputSocket() {
	output_socket_id_t id = currentOutputSockets.size();
	currentOutputSockets.push_back(false);
	nextOutputSockets.push_back(false);
	connectionDestinations.push_back(std::vector<input_socket_id_t>());
	return id;
}

std::vector<output_socket_id_t> LogicSimulator::registerOutputSockets(unsigned int count) {
	std::vector<output_socket_id_t> ids;
	for (unsigned int i = 0; i < count; i++) {
		ids.push_back(registerOutputSocket());
	}
	return ids;
}

input_socket_id_t LogicSimulator::registerInputSocket() {
	input_socket_id_t id = inputCountSockets.size();
	inputCountSockets.push_back(0);
	inputTotalCountSockets.push_back(0);
	return id;
}

std::vector<input_socket_id_t> LogicSimulator::registerInputSockets(unsigned int count) {
	std::vector<input_socket_id_t> ids;
	for (unsigned int i = 0; i < count; i++) {
		ids.push_back(registerInputSocket());
	}
	return ids;
}

eval_gate_id_t LogicSimulator::registerGate(input_socket_id_t inputSocket, output_socket_id_t outputSocket, GateType type) {
	gates.push_back(Gate{
		type,
		SocketsSingle{
			inputSocket,
			outputSocket
		}
	});
	return gates.size() - 1;
}

eval_gate_id_t LogicSimulator::registerGate(input_socket_id_t inputSocket, std::vector<output_socket_id_t> outputSockets, GateType type) {
	gates.push_back(Gate{
		type,
		SocketsOutputVector{
			inputSocket,
			outputSockets
		}
	});
	return gates.size() - 1;
}

eval_gate_id_t LogicSimulator::registerGate(std::vector<input_socket_id_t> inputSockets, output_socket_id_t outputSocket, GateType type) {
	gates.push_back(Gate{
		type,
		SocketsInputVector{
			inputSockets,
			outputSocket
		}
	});
	return gates.size() - 1;
}

eval_gate_id_t LogicSimulator::registerGate(std::vector<input_socket_id_t> inputSockets, std::vector<output_socket_id_t> outputSockets, GateType type) {
	gates.push_back(Gate{
		type,
		SocketsBothVector{
			inputSockets,
			outputSockets
		}
	});
	return gates.size() - 1;
}

void LogicSimulator::decomissionGate(eval_gate_id_t gateId) {
	gates[gateId].type = GateType::NONE;
}

void LogicSimulator::connect(output_socket_id_t sourceSocket, input_socket_id_t destinationSocket) {
	connectionDestinations[sourceSocket].push_back(destinationSocket);
	inputTotalCountSockets[destinationSocket] += 1;
}

void LogicSimulator::disconnect(output_socket_id_t sourceSocket, input_socket_id_t destinationSocket) {
	connectionDestinations[sourceSocket].erase(std::remove(connectionDestinations[sourceSocket].begin(), connectionDestinations[sourceSocket].end(), destinationSocket), connectionDestinations[sourceSocket].end());
	inputTotalCountSockets[destinationSocket] -= 1;
}

void LogicSimulator::simulationLoop() {
	while (true) {
		calculateStates();
		propagateStates();
		++ticksRun;

		bool waiting = false;
		bool sprinting = sprint.load(std::memory_order_acquire);
		// wait for pause to be false, if it is already false, don't wait
		while (
			running.load(std::memory_order_acquire)
			&& (
				pause.load(std::memory_order_acquire)
				|| (!sprinting)
				&& std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count() < nextTick_us.load(std::memory_order_acquire)
			)
		) {
			if (!waiting) {
				waiting = true;
				paused.store(true, std::memory_order_release);
			}
			std::this_thread::sleep_for(std::chrono::microseconds(100));
		}
		if (waiting) {
			paused.store(false, std::memory_order_release);
		}

		if (!sprinting) {
			const double target = tickrate.load(std::memory_order_acquire);
			nextTick_us.fetch_add(1000000 / target, std::memory_order_release);
		}

		if (!running.load(std::memory_order_acquire)) {
			break;
		}

		std::swap(currentOutputSockets, nextOutputSockets);
	}
}

inline void LogicSimulator::calculateStateBasic(unsigned int type, input_socket_id_t input, output_socket_id_t output) {
	unsigned int powered = inputCountSockets[input];
	if (type > 7) { // and + nand
		unsigned int gc = inputTotalCountSockets[input];
		nextOutputSockets[output] = ((type & 1) ^ (powered == gc)) && gc;
	} else if (type > 5) { // nor + xnor
		unsigned int gc = inputTotalCountSockets[input];
		nextOutputSockets[output] = (!((powered & 1) || (powered && (type & 1)))) && gc;
	} else if (type > 3) { // or + xor
		nextOutputSockets[output] = (powered & 1) || (powered && (type & 1));
	} else if (type > 1) { // tick_input + constant_on
		nextOutputSockets[output] = type & 1;
	} else { // stays the same
		nextOutputSockets[output] = currentOutputSockets[output];
	}
}

void LogicSimulator::calculateStates() {
	for (eval_gate_id_t i = 0; i < gates.size(); ++i) {
		const Gate gate = gates[i];
		const unsigned int type = static_cast<unsigned int>(gate.type);
		if (type < 10) {
			calculateStateBasic(type, std::get<SocketsSingle>(gate.sockets).inputSocket, std::get<SocketsSingle>(gate.sockets).outputSocket);
		}
		else {
		}
	}
}

void LogicSimulator::propagateStates() {
	for (output_socket_id_t i = 0; i < nextOutputSockets.size(); ++i) {
		const char difference = nextOutputSockets[i] - currentOutputSockets[i];
		if (difference != 0){
			for (input_socket_id_t destination : connectionDestinations[i]) {
				inputCountSockets[destination] += difference;
			}
		}
	}
}

void LogicSimulator::signalToPause() {
	pause.store(true, std::memory_order_release);
}

void LogicSimulator::signalToResume() {
	pause.store(false, std::memory_order_release);
}

void LogicSimulator::waitForPause() {
	while (!paused.load(std::memory_order_acquire)) {
		std::this_thread::sleep_for(std::chrono::microseconds(100));
	}
}

void LogicSimulator::setTickrate(double tickrate) {
	this->tickrate.store(tickrate, std::memory_order_release);
	nextTick_us.store(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count(), std::memory_order_release);

}

void LogicSimulator::setSprint(bool sprint) {
	this->sprint.store(sprint, std::memory_order_release);
	nextTick_us.store(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count(), std::memory_order_release);
}

void LogicSimulator::setOutputSocketState(output_socket_id_t outputSocket, logic_state_t state) {
	const char difference = state - nextOutputSockets[outputSocket];
	if (difference != 0) {
		for (input_socket_id_t destination : connectionDestinations[outputSocket]) {
			inputCountSockets[destination] += difference;
		}
	}
	nextOutputSockets[outputSocket] = state;
}

void LogicSimulator::monitorLoop() {
	int i = 0;
	long long int tickrate = 0;
	while (running.load(std::memory_order_acquire)) {
		const int ticks = ticksRun.exchange(0, std::memory_order_relaxed);
		tickrate += ticks - tickrateConveyer[i];
		tickrateConveyer[i] = ticks;
		i = (i + 1) % 8;
		realTickrate.store(tickrate, std::memory_order_release);
		std::this_thread::sleep_for(std::chrono::milliseconds(125));
	}
}