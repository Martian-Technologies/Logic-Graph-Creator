#include <stdexcept>
#include <chrono>
#include <iostream>
#include <algorithm>
#include <functional>

#include "logicSimulator.h"

LogicSimulator::LogicSimulator()
	: gates(),
	numDecomissioned(0),
	ticksRun(0),
	realTickrate(0),
	running(true),
	proceedFlag(false),
	isWaiting(false),
	nextTick_us(std::chrono::duration_cast<std::chrono::microseconds>(
		std::chrono::system_clock::now().time_since_epoch()).count()),
	targetTickrate(40*60) {
	simulationThread = std::thread(&LogicSimulator::simulationLoop, this);
	tickrateMonitorThread = std::thread(&LogicSimulator::tickrateMonitor, this);
}

LogicSimulator::~LogicSimulator() {
	running.store(false, std::memory_order_release);
	signalToProceed();
	{
		std::unique_lock<std::mutex> lock(killThreadsMux);
		killThreadsCv.notify_all();
	}
	if (simulationThread.joinable()) {
		simulationThread.join();
	}
	if (tickrateMonitorThread.joinable()) {
		tickrateMonitorThread.join();
	}
}

void LogicSimulator::initialize() {
	std::unique_lock<std::shared_mutex> lock(simulationMutex);
	for (auto& gate : gates) {
		if (gate.isValid()) {
			std::fill(gate.statesA.begin(), gate.statesA.end(), logic_state_t::LOW);
			std::fill(gate.statesB.begin(), gate.statesB.end(), logic_state_t::LOW);
		}
	}
}

block_id_t LogicSimulator::addGate(const GateType& gateType, bool allowSubstituteDecomissioned) {
	std::unique_lock<std::shared_mutex> lock(simulationMutex);

	if (allowSubstituteDecomissioned && numDecomissioned > 0) {
		for (size_t i = 0; i < gates.size(); ++i) {
			if (gates[i].type == GateType::NONE) {
				gates[i] = Gate(gateType);
				--numDecomissioned;
				return i;
			}
		}
	}

	gates.emplace_back(gateType);
	return gates.size() - 1;
}

void LogicSimulator::connectGates(block_id_t sourceGate, size_t outputGroup,
								 block_id_t targetGate, size_t inputGroup) {
	std::unique_lock<std::shared_mutex> lock(simulationMutex);

	if (sourceGate < 0 || sourceGate >= gates.size() || !gates[sourceGate].isValid())
		throw std::out_of_range("connectGates: sourceGate index out of range or invalid");
	if (targetGate < 0 || targetGate >= gates.size() || !gates[targetGate].isValid())
		throw std::out_of_range("connectGates: targetGate index out of range or invalid");

	if (outputGroup >= gates[sourceGate].getOutputGroupCount())
		throw std::out_of_range("connectGates: outputGroup index out of range");
	if (inputGroup >= gates[targetGate].getInputGroupCount())
		throw std::out_of_range("connectGates: inputGroup index out of range");

	GateConnection connection(sourceGate, outputGroup);

	// for (const auto& existingConn : gates[targetGate].inputGroups[inputGroup]) {
	// 	if (existingConn == connection) {
	// 		return;
	// 	}
	// }

	// allowing multiple copies of the same connection for buffer purposes
	// ex. in eval, if two buffers part of the same pool are connected to the same gate

	gates[targetGate].inputGroups[inputGroup].push_back(connection);
	gates[sourceGate].outputGroups[outputGroup].emplace_back(targetGate, inputGroup);
}

void LogicSimulator::connectGates(block_id_t sourceGate, block_id_t targetGate, size_t inputGroup) {
	connectGates(sourceGate, 0, targetGate, inputGroup);
}

void LogicSimulator::disconnectGates(block_id_t sourceGate, size_t outputGroup,
	block_id_t targetGate, size_t inputGroup) {
	std::unique_lock<std::shared_mutex> lock(simulationMutex);

	if (sourceGate < 0 || sourceGate >= gates.size() || !gates[sourceGate].isValid())
		throw std::out_of_range("disconnectGates: sourceGate index out of range or invalid");
	if (targetGate < 0 || targetGate >= gates.size() || !gates[targetGate].isValid())
		throw std::out_of_range("disconnectGates: targetGate index out of range or invalid");

	if (outputGroup >= gates[sourceGate].getOutputGroupCount() ||
		inputGroup >= gates[targetGate].getInputGroupCount())
		return;

	GateConnection connection(sourceGate, outputGroup);

	// auto& inputs = gates[targetGate].inputGroups[inputGroup];
	// inputs.erase(
	// 	std::remove(inputs.begin(), inputs.end(), connection),
	// 	inputs.end()
	// );

	// auto& outputs = gates[sourceGate].outputGroups[outputGroup];
	// outputs.erase(
	// 	std::remove_if(outputs.begin(), outputs.end(),
	// 		[targetGate, inputGroup](const std::pair<block_id_t, size_t>& conn) {
	// 			return conn.first == targetGate && conn.second == inputGroup;
	// 		}),
	// 	outputs.end()
	// );

	// only delete the first instance of the connection if it exists
	auto& inputs = gates[targetGate].inputGroups[inputGroup];
	auto it = std::find_if(inputs.begin(), inputs.end(),
		[&connection](const GateConnection& conn) {
			return conn.gateId == connection.gateId && conn.outputGroup == connection.outputGroup;
		});
	if (it != inputs.end()) {
		inputs.erase(it);
	}
	auto& outputs = gates[sourceGate].outputGroups[outputGroup];
	auto it2 = std::find_if(outputs.begin(), outputs.end(),
		[targetGate, inputGroup](const std::pair<block_id_t, size_t>& conn) {
			return conn.first == targetGate && conn.second == inputGroup;
		});
	if (it2 != outputs.end()) {
		outputs.erase(it2);
	}
}

void LogicSimulator::disconnectGates(block_id_t sourceGate, block_id_t targetGate, size_t inputGroup) {
	disconnectGates(sourceGate, 0, targetGate, inputGroup);
}

void LogicSimulator::decomissionGate(block_id_t gate) {
	std::unique_lock<std::shared_mutex> lock(simulationMutex);
	if (gate < 0 || gate >= gates.size() || !gates[gate].isValid())
		throw std::out_of_range("decomissionGate: gate index out of range or already decommissioned");

	for (size_t groupIdx = 0; groupIdx < gates[gate].inputGroups.size(); ++groupIdx) {
		auto& inputGroup = gates[gate].inputGroups[groupIdx];

		for (auto& inputConn : inputGroup) {
			block_id_t inputGate = inputConn.gateId;
			size_t outputGroup = inputConn.outputGroup;

			if (inputGate < 0 || inputGate >= gates.size() || !gates[inputGate].isValid())
				continue;

			if (outputGroup < gates[inputGate].outputGroups.size()) {
				auto& outputList = gates[inputGate].outputGroups[outputGroup];

				outputList.erase(
					std::remove_if(outputList.begin(), outputList.end(),
						[gate, groupIdx](const std::pair<block_id_t, size_t>& conn) {
							return conn.first == gate && conn.second == groupIdx;
						}),
					outputList.end()
				);
			}
		}
		inputGroup.clear();
	}

	for (size_t outGroupIdx = 0; outGroupIdx < gates[gate].outputGroups.size(); ++outGroupIdx) {
		auto& outputGroup = gates[gate].outputGroups[outGroupIdx];

		for (const auto& connection : outputGroup) {
			block_id_t outputGate = connection.first;
			size_t inputGroupIdx = connection.second;

			if (outputGate < 0 || outputGate >= gates.size() || !gates[outputGate].isValid())
				continue;

			if (inputGroupIdx < gates[outputGate].inputGroups.size()) {
				auto& targetInputs = gates[outputGate].inputGroups[inputGroupIdx];

				targetInputs.erase(
					std::remove_if(targetInputs.begin(), targetInputs.end(),
						[gate](const GateConnection& conn) {
							return conn.gateId == gate;
						}),
					targetInputs.end()
				);
			}
		}
		outputGroup.clear();
	}

	gates[gate].type = GateType::NONE;
	gates[gate].statesA.clear();
	gates[gate].statesB.clear();
	gates[gate].inputGroups.clear();
	gates[gate].outputGroups.clear();
	++numDecomissioned;
}

std::unordered_map<block_id_t, block_id_t> LogicSimulator::compressGates() {
	std::unique_lock<std::shared_mutex> lock(simulationMutex);
	std::unordered_map<block_id_t, block_id_t> gateMap;
	int newGateIndex = 0;

	for (size_t i = 0; i < gates.size(); ++i) {
		if (gates[i].isValid()) {
			gateMap[i] = newGateIndex++;
		}
	}

	std::vector<Gate> newGates;
	newGates.reserve(newGateIndex);

	for (size_t i = 0; i < gates.size(); ++i) {
		if (gates[i].isValid()) {
			newGates.push_back(gates[i]);
		}
	}

	for (auto& gate : newGates) {
		for (auto& group : gate.inputGroups) {
			for (auto& inputConn : group) {
				inputConn.gateId = gateMap[inputConn.gateId];
			}
		}

		for (auto& group : gate.outputGroups) {
			for (auto& outputConn : group) {
				outputConn.first = gateMap[outputConn.first];
			}
		}
	}

	gates = std::move(newGates);
	numDecomissioned = 0;

	return gateMap;
}

void LogicSimulator::computeBufferStates(Gate& gate) {
	logic_state_t state = logic_state_t::FLOATING;
	for (const auto& conn : gate.inputGroups[0]) {
		logic_state_t inputState = gates[conn.gateId].statesA[conn.outputGroup];
		if (inputState == logic_state_t::FLOATING) {
			continue;
		}
		if (inputState == logic_state_t::UNDEFINED) {
			state = logic_state_t::UNDEFINED;
			break;
		}
		if (state == logic_state_t::FLOATING) {
			state = inputState;
		}
		else if (state != inputState) {
			state = logic_state_t::UNDEFINED;
			break;
		}
	}
	gate.statesA[0] = state;
	gate.statesB[0] = state;
}

void LogicSimulator::computeGateStates(Gate& gate) {
	switch (gate.type) {
	case GateType::AND:
	{
		bool hasBadInput = false;
		for (const auto& conn : gate.inputGroups[0]) {
			logic_state_t inputState = gates[conn.gateId].statesA[conn.outputGroup];
			if (inputState == logic_state_t::LOW) {
				gate.statesB[0] = logic_state_t::LOW;
				return;
			}
			if (inputState == logic_state_t::UNDEFINED) {
				hasBadInput = true;
			}
			if (inputState == logic_state_t::FLOATING) {
				hasBadInput = true;
			}
		}
		if (hasBadInput) {
			gate.statesB[0] = logic_state_t::UNDEFINED;
		}
		else {
			gate.statesB[0] = logic_state_t::HIGH;
		}
		return;
	}

	case GateType::OR:
	{
		bool hasBadInput = false;
		for (const auto& conn : gate.inputGroups[0]) {
			logic_state_t inputState = gates[conn.gateId].statesA[conn.outputGroup];
			if (inputState == logic_state_t::HIGH) {
				gate.statesB[0] = logic_state_t::HIGH;
				return;
			}
			if (inputState == logic_state_t::UNDEFINED) {
				hasBadInput = true;
			}
			if (inputState == logic_state_t::FLOATING) {
				hasBadInput = true;
			}
		}
		if (hasBadInput) {
			gate.statesB[0] = logic_state_t::UNDEFINED;
		}
		else {
			gate.statesB[0] = logic_state_t::LOW;
		}
		return;
	}
	case GateType::XOR:
	{
		bool hasHighInput = false;
		for (const auto& conn : gate.inputGroups[0]) {
			logic_state_t inputState = gates[conn.gateId].statesA[conn.outputGroup];
			if (inputState == logic_state_t::HIGH) {
				hasHighInput = !hasHighInput;
			}
			if (inputState == logic_state_t::UNDEFINED) {
				gate.statesB[0] = logic_state_t::UNDEFINED;
				return;
			}
			if (inputState == logic_state_t::FLOATING) {
				gate.statesB[0] = logic_state_t::UNDEFINED;
				return;
			}
		}
		gate.statesB[0] = fromBool(hasHighInput);
		return;
	}
	case GateType::NAND:
	{
		bool hasBadInput = false;
		for (const auto& conn : gate.inputGroups[0]) {
			logic_state_t inputState = gates[conn.gateId].statesA[conn.outputGroup];
			if (inputState == logic_state_t::LOW) {
				gate.statesB[0] = logic_state_t::HIGH;
				return;
			}
			if (inputState == logic_state_t::UNDEFINED) {
				hasBadInput = true;
			}
			if (inputState == logic_state_t::FLOATING) {
				hasBadInput = true;
			}
		}
		if (hasBadInput) {
			gate.statesB[0] = logic_state_t::UNDEFINED;
		}
		else {
			gate.statesB[0] = logic_state_t::LOW;
		}
		return;
	}
	case GateType::NOR:
	{
		bool hasBadInput = false;
		for (const auto& conn : gate.inputGroups[0]) {
			logic_state_t inputState = gates[conn.gateId].statesA[conn.outputGroup];
			if (inputState == logic_state_t::HIGH) {
				gate.statesB[0] = logic_state_t::LOW;
				return;
			}
			if (inputState == logic_state_t::UNDEFINED) {
				hasBadInput = true;
			}
			if (inputState == logic_state_t::FLOATING) {
				hasBadInput = true;
			}
		}
		if (hasBadInput) {
			gate.statesB[0] = logic_state_t::UNDEFINED;
		}
		else {
			gate.statesB[0] = logic_state_t::HIGH;
		}
		return;
	}
	case GateType::XNOR:
	{
		bool hasHighInput = false;
		for (const auto& conn : gate.inputGroups[0]) {
			logic_state_t inputState = gates[conn.gateId].statesA[conn.outputGroup];
			if (inputState == logic_state_t::HIGH) {
				hasHighInput = !hasHighInput;
			}
			if (inputState == logic_state_t::UNDEFINED) {
				gate.statesB[0] = logic_state_t::UNDEFINED;
				return;
			}
			if (inputState == logic_state_t::FLOATING) {
				gate.statesB[0] = logic_state_t::UNDEFINED;
				return;
			}
		}
		gate.statesB[0] = fromBool(!hasHighInput);
		return;
	}
	case GateType::DEFAULT_RETURN_CURRENTSTATE:
	{
		gate.statesB[0] = gate.statesA[0];
		return;
	}
	case GateType::TICK_INPUT:
	{
		gate.statesB[0] = logic_state_t::LOW;
		return;
	}
	case GateType::CONSTANT_ON:
	{
		gate.statesB[0] = logic_state_t::HIGH;
		return;
	}
	case GateType::TRISTATE_BUFFER:
	{
		const auto& dataGroup = gate.inputGroups[0];
		const auto& controlGroup = gate.inputGroups[1];
		logic_state_t controlState = gates[controlGroup[0].gateId].statesA[controlGroup[0].outputGroup];
		if (controlState == logic_state_t::HIGH) {
			gate.statesB[0] = gates[dataGroup[0].gateId].statesA[dataGroup[0].outputGroup];
		}
		else if (controlState == logic_state_t::LOW) {
			gate.statesB[0] = logic_state_t::FLOATING;
		}
		else {
			gate.statesB[0] = logic_state_t::UNDEFINED;
		}
		return;
	}
	case GateType::TRISTATE_BUFFER_INVERTED:
	{
		const auto& dataGroup = gate.inputGroups[0];
		const auto& controlGroup = gate.inputGroups[1];
		logic_state_t controlState = gates[controlGroup[0].gateId].statesA[controlGroup[0].outputGroup];
		if (controlState == logic_state_t::LOW) {
			gate.statesB[0] = gates[dataGroup[0].gateId].statesA[dataGroup[0].outputGroup];
		}
		else if (controlState == logic_state_t::HIGH) {
			gate.statesB[0] = logic_state_t::FLOATING;
		}
		else {
			gate.statesB[0] = logic_state_t::UNDEFINED;
		}
		return;
	}
	}
	logError("computeGateStates: Unknown gate type {}", "", static_cast<int>(gate.type));
}

void LogicSimulator::computeNextState() {
	std::shared_lock<std::shared_mutex> lock(simulationMutex);

	for (auto& gate : gates) {
		if (gate.type == GateType::BUFFER) {
			computeBufferStates(gate);
		}
	}
	for (auto& gate : gates) {
		if (gate.isValid() && gate.type != GateType::BUFFER) {
			computeGateStates(gate);
		}
	}
}

void LogicSimulator::swapStates() {
	std::unique_lock<std::shared_mutex> lock(simulationMutex);

	for (auto& gate : gates) {
		if (gate.isValid()) {
			std::swap(gate.statesA, gate.statesB);
		}
	}
}

void LogicSimulator::setState(block_id_t gate, size_t outputGroup, logic_state_t state) {
	std::unique_lock<std::shared_mutex> lock(simulationMutex);
	if (gate < 0 || gate >= gates.size() || !gates[gate].isValid())
		throw std::out_of_range("setState: gate index out of range or invalid");

	if (outputGroup >= gates[gate].getOutputGroupCount())
		throw std::out_of_range("setState: outputGroup index out of range");

	gates[gate].statesA[outputGroup] = state;
	gates[gate].statesB[outputGroup] = state;
}

void LogicSimulator::setState(block_id_t gate, logic_state_t state) {
	setState(gate, 0, state);
}

void LogicSimulator::clearGates() {
	std::unique_lock<std::shared_mutex> lock(simulationMutex);
	gates.clear();
	numDecomissioned = 0;
}

void LogicSimulator::reserveGates(unsigned int numGates) {
	std::unique_lock<std::shared_mutex> lock(simulationMutex);
	gates.reserve(numGates);
}

logic_state_t LogicSimulator::getState(block_id_t gate, size_t outputGroup) const {
	std::shared_lock<std::shared_mutex> lock(simulationMutex);
	if (gate < 0 || gate >= gates.size() || !gates[gate].isValid())
		throw std::out_of_range("getState: gate index out of range or invalid");

	if (outputGroup >= gates[gate].getOutputGroupCount())
		throw std::out_of_range("getState: outputGroup index out of range");

	return gates[gate].statesA[outputGroup];
}

logic_state_t LogicSimulator::getState(block_id_t gate) const {
	return getState(gate, 0);
}

std::vector<std::vector<logic_state_t>> LogicSimulator::getCurrentState() const {
	std::shared_lock<std::shared_mutex> lock(simulationMutex);
	std::vector<std::vector<logic_state_t>> states;
	states.reserve(gates.size());

	for (const auto& gate : gates) {
		if (gate.isValid()) {
			states.push_back(gate.statesA);
		} else {
			states.push_back(std::vector<logic_state_t>());
		}
	}

	return states;
}

void LogicSimulator::debugPrint() {
	std::shared_lock<std::shared_mutex> lock(simulationMutex);
	std::cout << "ID:        ";
	for (size_t i = 0; i < gates.size(); ++i) {
		std::cout << i << " ";
	}
	std::cout << "\nGate type: ";
	for (const auto& gate : gates) {
		std::cout << static_cast<int>(gate.type) << " ";
	}

	std::cout << "\nInput Groups:" << std::endl;

	size_t maxInputGroups = 0;
	for (const auto& gate : gates) {
		maxInputGroups = std::max(maxInputGroups, gate.getInputGroupCount());
	}

	for (size_t groupIdx = 0; groupIdx < maxInputGroups; ++groupIdx) {
		std::cout << "Group " << groupIdx << ":  ";
		for (size_t gateIdx = 0; gateIdx < gates.size(); ++gateIdx) {
			const auto& gate = gates[gateIdx];
			if (groupIdx < gate.getInputGroupCount()) {
				std::cout << "[";
				for (size_t inputIdx = 0; inputIdx < gate.inputGroups[groupIdx].size(); ++inputIdx) {
					const auto& conn = gate.inputGroups[groupIdx][inputIdx];
					std::cout << conn.gateId << ":" << conn.outputGroup;
					if (inputIdx < gate.inputGroups[groupIdx].size() - 1) {
						std::cout << ",";
					}
				}
				std::cout << "] ";
			} else {
				std::cout << "[] ";
			}
		}
		std::cout << std::endl;
	}

	std::cout << "\nOutput Groups:" << std::endl;

	size_t maxOutputGroups = 0;
	for (const auto& gate : gates) {
		maxOutputGroups = std::max(maxOutputGroups, gate.getOutputGroupCount());
	}

	for (size_t groupIdx = 0; groupIdx < maxOutputGroups; ++groupIdx) {
		std::cout << "Group " << groupIdx << ":  ";
		for (size_t gateIdx = 0; gateIdx < gates.size(); ++gateIdx) {
			const auto& gate = gates[gateIdx];
			if (groupIdx < gate.getOutputGroupCount()) {
				std::cout << "[";
				for (size_t outputIdx = 0; outputIdx < gate.outputGroups[groupIdx].size(); ++outputIdx) {
					const auto& conn = gate.outputGroups[groupIdx][outputIdx];
					std::cout << conn.first << ":" << conn.second;
					if (outputIdx < gate.outputGroups[groupIdx].size() - 1) {
						std::cout << ",";
					}
				}
				std::cout << "] ";
			} else {
				std::cout << "[] ";
			}
		}
		std::cout << std::endl;
	}

	std::cout << "\nStates:" << std::endl;

	for (size_t groupIdx = 0; groupIdx < maxOutputGroups; ++groupIdx) {
		std::cout << "Group " << groupIdx << " StateA: ";
		for (size_t gateIdx = 0; gateIdx < gates.size(); ++gateIdx) {
			const auto& gate = gates[gateIdx];
			if (gate.isValid() && groupIdx < gate.getOutputGroupCount()) {
				const char* stateStr;
				switch (gate.statesA[groupIdx]) {
					case logic_state_t::LOW: stateStr = "L"; break;
					case logic_state_t::HIGH: stateStr = "H"; break;
					case logic_state_t::FLOATING: stateStr = "Z"; break;
					case logic_state_t::UNDEFINED: stateStr = "X"; break;
					default: stateStr = "?";
				}
				std::cout << stateStr << " ";
			} else {
				std::cout << "- ";
			}
		}
		std::cout << std::endl;

		std::cout << "Group " << groupIdx << " StateB: ";
		for (size_t gateIdx = 0; gateIdx < gates.size(); ++gateIdx) {
			const auto& gate = gates[gateIdx];
			if (gate.isValid() && groupIdx < gate.getOutputGroupCount()) {
				const char* stateStr;
				switch (gate.statesB[groupIdx]) {
					case logic_state_t::LOW: stateStr = "L"; break;
					case logic_state_t::HIGH: stateStr = "H"; break;
					case logic_state_t::FLOATING: stateStr = "Z"; break;
					case logic_state_t::UNDEFINED: stateStr = "X"; break;
					default: stateStr = "?";
				}
				std::cout << stateStr << " ";
			} else {
				std::cout << "- ";
			}
		}
		std::cout << std::endl;
	}

	std::cout << std::endl;
}

void LogicSimulator::simulationLoop() {
	while (running.load(std::memory_order_acquire)) {
		if (proceedFlag.load(std::memory_order_acquire)) {
			{
				computeNextState();
			}

			++ticksRun;

			swapStates(); // later we will just have two different computestate functions.

			const unsigned long long int target = targetTickrate.load(std::memory_order_acquire);
			nextTick_us.store(
				std::chrono::duration_cast<std::chrono::microseconds>(
					std::chrono::system_clock::now().time_since_epoch()
				).count() + (60000000 / target),
				std::memory_order_release
			);
		}

		auto now = std::chrono::duration_cast<std::chrono::microseconds>(
			std::chrono::system_clock::now().time_since_epoch()
		).count();

		auto nextTick = nextTick_us.load(std::memory_order_acquire);
		auto sleepDuration = std::max<int64_t>(0, nextTick - now);

		{
			isWaiting.store(true, std::memory_order_release);
			std::unique_lock<std::mutex> lock(simulationMutex2);

			if (proceedFlag.load(std::memory_order_acquire)) {
				simulationCv.wait_for(lock,
					std::chrono::microseconds(sleepDuration),
					[this]() { return !running.load(std::memory_order_acquire); }
				);
			} else {
				simulationCv.wait(lock,
					[this]() {
						return proceedFlag.load(std::memory_order_acquire) ||
							   !running.load(std::memory_order_acquire);
					}
				);
			}
			isWaiting.store(false, std::memory_order_release);
		}

		if (!running.load(std::memory_order_acquire)) {
			break;
		}
	}
}

void LogicSimulator::signalToPause() {
	proceedFlag.store(false, std::memory_order_release);
	// no need to notify bcs thread will just stop processing
}

void LogicSimulator::signalToProceed() {
	proceedFlag.store(true, std::memory_order_release);
	simulationCv.notify_one();
}

bool LogicSimulator::threadIsWaiting() const {
	return isWaiting.load(std::memory_order_acquire);
}

long long int LogicSimulator::getRealTickrate() const {
	return realTickrate.load(std::memory_order_acquire);
}

void LogicSimulator::tickrateMonitor() {
	while (running.load(std::memory_order_acquire)) {
		if (proceedFlag.load(std::memory_order_acquire)) {
			const long long int ticks = ticksRun.exchange(0, std::memory_order_relaxed);
			realTickrate.store(ticks, std::memory_order_release);
		} else {
			realTickrate.store(0, std::memory_order_release);
		}

		std::unique_lock<std::mutex> lk(killThreadsMux);
		killThreadsCv.wait_for(lk, std::chrono::seconds(1));
	}
}

void LogicSimulator::setTargetTickrate(unsigned long long tickrate) {
	targetTickrate.store(tickrate, std::memory_order_release);
	triggerNextTickReset();
}

void LogicSimulator::triggerNextTickReset() {
	nextTick_us.store(std::chrono::duration_cast<std::chrono::microseconds>(
		std::chrono::system_clock::now().time_since_epoch()).count(),
		std::memory_order_release);

	simulationCv.notify_one();
}