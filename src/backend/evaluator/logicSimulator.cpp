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
	rollingAvgLength(8),
	rollingAvgIndex(0),
	targetTickrate(40 * 60) {
	rollingAvg.resize(rollingAvgLength, 0);
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
	for (auto& gate : gates) {
		if (gate.isValid()) {
			std::fill(gate.statesA.begin(), gate.statesA.end(), logic_state_t::LOW);
			std::fill(gate.statesB.begin(), gate.statesB.end(), logic_state_t::LOW);
		}
	}
}

simulator_gate_id_t LogicSimulator::addGate(const GateType& gateType, bool allowSubstituteDecomissioned) {
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

void LogicSimulator::connectGates(simulator_gate_id_t sourceGate, size_t outputGroup,
								 simulator_gate_id_t targetGate, size_t inputGroup) {
	if (sourceGate < 0 || sourceGate >= gates.size()) {
		logError("connectGates: sourceGate index out of range", "Simulator");
		return;
	}
	if (!gates[sourceGate].isValid()) {
		logError("connectGates: sourceGate is invalid", "Simulator");
		return;
	}
	if (targetGate < 0 || targetGate >= gates.size()) {
		logError("connectGates: targetGate index out of range", "Simulator");
		return;
	}
	if (!gates[targetGate].isValid()) {
		logError("connectGates: targetGate is invalid", "Simulator");
		return;
	}

	if (outputGroup >= gates[sourceGate].getOutputGroupCount()) {
		logError("connectGates: outputGroup index out of range", "Simulator");
		return;
	}
	if (inputGroup >= gates[targetGate].getInputGroupCount()) {
		logError("connectGates: inputGroup index out of range", "Simulator");
		return;
	}

	GateConnection connection(sourceGate, outputGroup);

	// for (const auto& existingConn : gates[targetGate].inputGroups[inputGroup]) {
	// 	if (existingConn == connection) {
	// 		return;
	// 	}
	// }

	// allowing multiple copies of the same connection for junction purposes
	// ex. in eval, if two junctions part of the same pool are connected to the same gate

	gates[targetGate].inputGroups[inputGroup].push_back(connection);
	gates[sourceGate].outputGroups[outputGroup].emplace_back(targetGate, inputGroup);
}

void LogicSimulator::disconnectGates(simulator_gate_id_t sourceGate, size_t outputGroup,
	simulator_gate_id_t targetGate, size_t inputGroup) {
	if (sourceGate < 0 || sourceGate >= gates.size() || !gates[sourceGate].isValid()) {
		logError("disconnectGates: sourceGate index out of range or invalid", "Simulator");
		return;
	}
	if (targetGate < 0 || targetGate >= gates.size() || !gates[targetGate].isValid()) {
		logError("disconnectGates: targetGate index out of range or invalid", "Simulator");
		return;
	}

	if (outputGroup >= gates[sourceGate].getOutputGroupCount() ||
		inputGroup >= gates[targetGate].getInputGroupCount()) {
		logWarning("disconnectGates: outputGroup or inputGroup index out of range", "Simulator");
		return;
	}

	GateConnection connection(sourceGate, outputGroup);

	// auto& inputs = gates[targetGate].inputGroups[inputGroup];
	// inputs.erase(
	// 	std::remove(inputs.begin(), inputs.end(), connection),
	// 	inputs.end()
	// );

	// auto& outputs = gates[sourceGate].outputGroups[outputGroup];
	// outputs.erase(
	// 	std::remove_if(outputs.begin(), outputs.end(),
	// 		[targetGate, inputGroup](const std::pair<simulator_gate_id_t, size_t>& conn) {
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
		[targetGate, inputGroup](const std::pair<simulator_gate_id_t, size_t>& conn) {
			return conn.first == targetGate && conn.second == inputGroup;
		});
	if (it2 != outputs.end()) {
		outputs.erase(it2);
	}
}

void LogicSimulator::decomissionGate(simulator_gate_id_t gate) {
	if (gate < 0 || gate >= gates.size() || !gates[gate].isValid()) {
		logError("decomissionGate: gate index out of range or already decommissioned", "Simulator");
		return;
	}

	for (size_t groupIdx = 0; groupIdx < gates[gate].inputGroups.size(); ++groupIdx) {
		auto& inputGroup = gates[gate].inputGroups[groupIdx];

		for (auto& inputConn : inputGroup) {
			simulator_gate_id_t inputGate = inputConn.gateId;
			size_t outputGroup = inputConn.outputGroup;

			if (inputGate < 0 || inputGate >= gates.size() || !gates[inputGate].isValid())
				continue;

			if (outputGroup < gates[inputGate].outputGroups.size()) {
				auto& outputList = gates[inputGate].outputGroups[outputGroup];

				outputList.erase(
					std::remove_if(outputList.begin(), outputList.end(),
						[gate, groupIdx](const std::pair<simulator_gate_id_t, size_t>& conn) {
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
			simulator_gate_id_t outputGate = connection.first;
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

std::unordered_map<simulator_gate_id_t, simulator_gate_id_t> LogicSimulator::compressGates() {
	std::unordered_map<simulator_gate_id_t, simulator_gate_id_t> gateMap;
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

void LogicSimulator::computeJunctionStates(Gate& gate) {
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
		if (gate.inputGroups[0].size() == 0) {
			gate.statesB[0] = logic_state_t::LOW;
			return;
		}
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
		if (gate.inputGroups[0].size() == 0) {
			gate.statesB[0] = logic_state_t::LOW;
			return;
		}
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
		if (gate.inputGroups[0].size() == 0) {
			gate.statesB[0] = logic_state_t::LOW;
			return;
		}
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
	case GateType::COPYINPUT:
	{
		if (gate.inputGroups[0].size() == 0) {
			gate.statesB[0] = logic_state_t::UNDEFINED;
			return;
		}
		gate.statesB[0] = gates[gate.inputGroups[0][0].gateId].statesA[gate.inputGroups[0][0].outputGroup];
		return;
	}
	case GateType::TRISTATE_BUFFER:
	{
		const auto& dataGroup = gate.inputGroups[0];
		const auto& controlGroup = gate.inputGroups[1];
		if (controlGroup.size() == 0) {
			gate.statesB[0] = logic_state_t::UNDEFINED;
			return;
		}
		logic_state_t controlState = gates[controlGroup[0].gateId].statesA[controlGroup[0].outputGroup];
		if (controlState == logic_state_t::HIGH) {
			if (dataGroup.size() == 0) {
				gate.statesB[0] = logic_state_t::UNDEFINED;
				return;
			}
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
		if (controlGroup.size() == 0) {
			gate.statesB[0] = logic_state_t::UNDEFINED;
			return;
		}
		logic_state_t controlState = gates[controlGroup[0].gateId].statesA[controlGroup[0].outputGroup];
		if (controlState == logic_state_t::LOW) {
			if (dataGroup.size() == 0) {
				gate.statesB[0] = logic_state_t::UNDEFINED;
				return;
			}
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
	case GateType::JUNCTION:
	{
		logError("Gate type \"Junction\" exists in simulator, cannot be simulated.", "Simulator");
		return;
	}
	case GateType::NONE:
	{
		logError("Gate type \"None\" exists in simulator, cannot be simulated.", "Simulator");
		return;
	}
	}
	logError("computeGateStates: Unknown gate type {}", "", static_cast<int>(gate.type));
}

void LogicSimulator::setState(simulator_gate_id_t gate, size_t outputGroup, logic_state_t state) {
	if (gate < 0 || gate >= gates.size() || !gates[gate].isValid()) {
		logError("setState: gate index out of range or invalid", "Simulator");
		return;
	}

	if (outputGroup >= gates[gate].getOutputGroupCount()) {
		logError("setState: outputGroup index out of range", "Simulator");
		return;
	}

	gates[gate].statesA[outputGroup] = state;
	gates[gate].statesB[outputGroup] = state;
}

void LogicSimulator::clearGates() {
	gates.clear();
	numDecomissioned = 0;
}

logic_state_t LogicSimulator::getState(simulator_gate_id_t gate, size_t outputGroup) const {
	if (gate < 0 || gate >= gates.size() || !gates[gate].isValid()) {
		logError("getState: gate index out of range or invalid", "Simulator");
		return logic_state_t::UNDEFINED;
	}

	if (outputGroup >= gates[gate].getOutputGroupCount()) {
		logError("getState: outputGroup index out of range", "Simulator");
		return logic_state_t::UNDEFINED;
	}

	return gates[gate].statesA[outputGroup];
}

std::vector<std::vector<logic_state_t>> LogicSimulator::getCurrentState() const {
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

void LogicSimulator::simulationLoop() {
	while (running.load(std::memory_order_acquire)) {
		if (proceedFlag.load(std::memory_order_acquire)) {
			{
				std::unique_lock<std::shared_mutex> lock(simulationMutex);
				for (auto& gate : gates) {
					if (gate.type == GateType::JUNCTION) {
						computeJunctionStates(gate);
					}
				}
				for (auto& gate : gates) {
					if (gate.isValid() && gate.type != GateType::JUNCTION) {
						computeGateStates(gate);
					}
				}
				for (auto& gate : gates) {
					if (gate.isValid()) {
						std::swap(gate.statesA, gate.statesB);
					}
				}
				lock.unlock();
			}

			++ticksRun;

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
	if (proceedFlag.load(std::memory_order_acquire)) {
		return realTickrate.load(std::memory_order_acquire);
	}
	return 0;
}

void LogicSimulator::tickrateMonitor() {
	while (running.load(std::memory_order_acquire)) {
		if (proceedFlag.load(std::memory_order_acquire)) {
			const long long int ticks = ticksRun.exchange(0, std::memory_order_relaxed);
			updateRollingAverage(ticks);
		} else {
			updateRollingAverage(0);
		}

		std::unique_lock<std::mutex> lk(killThreadsMux);
		killThreadsCv.wait_for(lk, std::chrono::microseconds(1000000 / rollingAvgLength));
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

void LogicSimulator::updateRollingAverage(int reading) {
	long long int localRealTickrate = realTickrate.load(std::memory_order_acquire);
	localRealTickrate -= rollingAvg[rollingAvgIndex];
	rollingAvg[rollingAvgIndex] = reading;
	localRealTickrate += reading;
	rollingAvgIndex = (rollingAvgIndex + 1) % rollingAvgLength;
	realTickrate.store(localRealTickrate, std::memory_order_release);
}