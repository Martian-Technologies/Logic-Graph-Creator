#include "simulatorOptimizer.h"

void SimulatorOptimizer::addGate(const GateType gateType, const middle_id_t gateId) {
    simulator_id_t simulatorId = simulatorIdProvider.getNewId();
    // if simulatorIds is too short, extend it
    if (simulatorIds.size() <= simulatorId) {
        simulatorIds.resize(simulatorId + 1);
    }
    simulatorIds[simulatorId] = gateId;
    switch (gateType) {
    case GateType::AND:
        simulator.andGates.push_back({ simulatorId, false, false, {} });
        break;
    case GateType::OR:
        simulator.andGates.push_back({ simulatorId, true, true, {} });
        break;
    case GateType::NAND:
        simulator.andGates.push_back({ simulatorId, false, true, {} });
        break;
    case GateType::NOR:
        simulator.andGates.push_back({ simulatorId, true, false, {} });
        break;
    case GateType::XOR:
        simulator.xorGates.push_back({ simulatorId, false, {} });
        break;
    case GateType::XNOR:
        simulator.xorGates.push_back({ simulatorId, true, {} });
        break;
    case GateType::JUNCTION:
        simulator.junctions.push_back({ simulatorId, {} });
        break;
    case GateType::TRISTATE_BUFFER:
        simulator.tristateBuffers.push_back({ simulatorId, false, {}, {} });
        break;
    case GateType::TRISTATE_BUFFER_INVERTED:
        simulator.tristateBuffers.push_back({ simulatorId, true, {}, {} });
        break;
    case GateType::CONSTANT_OFF:
        // simulator.constantGates.push_back({ simulatorId });
        // set the output state to OFF
        break;
    case GateType::CONSTANT_ON:
        // simulator.constantGates.push_back({ simulatorId });
        // set the output state to ON
        break;
    case GateType::DUMMY_INPUT:
        simulator.copySelfOutputGates.push_back({ simulatorId });
        break;
    case GateType::THROUGH:
        simulator.singleBuffers.push_back({ simulatorId, false, std::nullopt });
        break;
    case GateType::TICK_INPUT:
        simulator.constantResetGates.push_back({ simulatorId, logic_state_t::LOW });
        break;
    case GateType::NONE:
        logError("Cannot add gate of type NONE", "SimulatorOptimizer::addGate");
        break;
    }
    return;
}

void SimulatorOptimizer::removeGate(const middle_id_t gateId) {
    // Find the gate in the simulator and remove it
    auto it = std::find(simulatorIds.begin(), simulatorIds.end(), gateId);
    if (it != simulatorIds.end()) {
        simulator_id_t simulatorId = std::distance(simulatorIds.begin(), it);
        removeGateBySimId(simulatorId);
        simulatorIds.erase(it);
    }
}

void SimulatorOptimizer::removeGateBySimId(const simulator_id_t simulatorId) {
    // Remove the gate from the simulator based on its ID
    auto removeGateIf = [simulatorId](auto& gates) {
        gates.erase(std::remove_if(gates.begin(), gates.end(),
            [simulatorId](const auto& gate) { return gate.id == simulatorId; }), gates.end());
        };
    removeGateIf(simulator.andGates);
    removeGateIf(simulator.xorGates);
    removeGateIf(simulator.junctions);
    removeGateIf(simulator.buffers);
    removeGateIf(simulator.singleBuffers);
    removeGateIf(simulator.tristateBuffers);
    // removeGateIf(simulator.constantGates);
    removeGateIf(simulator.constantResetGates);
    removeGateIf(simulator.copySelfOutputGates);
}