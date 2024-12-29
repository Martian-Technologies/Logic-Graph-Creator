#include "../logicSimulator.h"
#include <iostream>
#include "tests.h"

bool testTwoGates(LogicSimulator& ls) {
    ls.clearGates();
    ls.reserveGates(2);
    auto gate1 = ls.addGate(GateType::AND);
    auto gate2 = ls.addGate(GateType::NOR);
    ls.connectGates(gate1, gate2);
    ls.initialize();
    ls.simulateNTicks(10);
    // print the state of the gates
    std::vector<logic_state_t> states = ls.getCurrentState();
    return states[gate2] && !states[gate1];
}

bool testThreeGates(LogicSimulator& ls) {
    ls.clearGates();
    ls.reserveGates(3);
    auto gate1 = ls.addGate(GateType::AND);
    auto gate2 = ls.addGate(GateType::NOR);
    auto gate3 = ls.addGate(GateType::AND);
    ls.connectGates(gate1, gate2);
    ls.connectGates(gate2, gate3);
    ls.connectGates(gate3, gate1);
    ls.initialize();
    ls.simulateNTicks(10);
    // print the state of the gates
    std::vector<logic_state_t> states = ls.getCurrentState();
    return states[gate1] && !states[gate2] && states[gate3];
}

bool testFullAdder(LogicSimulator& ls) {
    ls.clearGates();
    ls.reserveGates(8);
    auto gateXOR = ls.addGate(GateType::XOR); // sum
    auto gateAND1 = ls.addGate(GateType::AND);
    auto gateAND2 = ls.addGate(GateType::AND);
    auto gateAND3 = ls.addGate(GateType::AND);
    auto gateOR = ls.addGate(GateType::OR); // carry
    auto gateInput1 = ls.addGate(GateType::INPUT);
    auto gateInput2 = ls.addGate(GateType::INPUT);
    auto gateInput3 = ls.addGate(GateType::INPUT);
    ls.connectGates(gateInput1, gateXOR);
    ls.connectGates(gateInput2, gateXOR);
    ls.connectGates(gateInput3, gateXOR);
    ls.connectGates(gateInput1, gateAND1);
    ls.connectGates(gateInput2, gateAND1);
    ls.connectGates(gateInput2, gateAND2);
    ls.connectGates(gateInput3, gateAND2);
    ls.connectGates(gateInput1, gateAND3);
    ls.connectGates(gateInput3, gateAND3);
    ls.connectGates(gateAND1, gateOR);
    ls.connectGates(gateAND2, gateOR);
    ls.connectGates(gateAND3, gateOR);
    ls.initialize();
    ls.simulateNTicks(10);
    // output should be 0, 0


    bool fail = false;
    std::vector<logic_state_t> states = ls.getCurrentState();
    if (states[gateXOR] != false) {
        std::cout << "XOR gate failed pt. 1" << std::endl;
        fail = true;
    }
    if (states[gateOR] != false) {
        std::cout << "OR gate failed pt. 1" << std::endl;
        fail = true;
    }

    ls.simulateNTicks(1);
    ls.setState(gateInput1, true);
    ls.swapStates();

    ls.simulateNTicks(10);

    // output should be 1, 0
    states = ls.getCurrentState();
    if (states[gateXOR] != true) {
        std::cout << "XOR gate failed pt. 2" << std::endl;
        fail = true;
    }
    if (states[gateOR] != false) {
        std::cout << "OR gate failed pt. 2" << std::endl;
        fail = true;
    }

    ls.computeNextState(ls.allGates());
    ls.setState(gateInput2, true);
    ls.swapStates();

    ls.simulateNTicks(10);

    // output should be 0, 1
    states = ls.getCurrentState();
    if (states[gateXOR] != false) {
        std::cout << "XOR gate failed pt. 3" << std::endl;
        fail = true;
    }
    if (states[gateOR] != true) {
        std::cout << "OR gate failed pt. 3" << std::endl;
        fail = true;
    }

    ls.computeNextState(ls.allGates());
    ls.setState(gateInput3, true);
    ls.swapStates();

    ls.simulateNTicks(10);

    // output should be 1, 1
    states = ls.getCurrentState();
    if (states[gateXOR] != true) {
        std::cout << "XOR gate failed pt. 4" << std::endl;
        fail = true;
    }
    if (states[gateOR] != true) {
        std::cout << "OR gate failed pt. 4" << std::endl;
        fail = true;
    }

    ls.computeNextState(ls.allGates());
    ls.setState(gateInput1, false);
    ls.setState(gateInput2, false);
    ls.setState(gateInput3, false);
    ls.swapStates();

    ls.simulateNTicks(10);

    // output should be 0, 0
    states = ls.getCurrentState();
    if (states[gateXOR] != false) {
        std::cout << "XOR gate failed pt. 5" << std::endl;
        fail = true;
    }
    if (states[gateOR] != false) {
        std::cout << "OR gate failed pt. 5" << std::endl;
        fail = true;
    }

    return !fail;
}

bool testDecomissioning(LogicSimulator& ls) {
    ls.clearGates();
    ls.reserveGates(3);
    auto gate1 = ls.addGate(GateType::AND);
    auto gate2 = ls.addGate(GateType::NOR);
    auto gate3 = ls.addGate(GateType::NOR);
    ls.connectGates(gate1, gate2);
    ls.connectGates(gate2, gate3);
    ls.connectGates(gate1, gate3);
    // this should result in a stable state: 0, 1, 0
    ls.initialize();
    ls.simulateNTicks(10);
    if (ls.getState(gate1) || !ls.getState(gate2) || ls.getState(gate3)) {
        std::cout << "Initial state failed" << std::endl;
        return false;
    }
    ls.computeNextState(ls.allGates());
    ls.decomissionGate(gate2);
    ls.swapStates();
    // this should result in a stable state: 0, _, 1
    ls.simulateNTicks(10);
    if (ls.getState(gate1) || !ls.getState(gate3)) {
        std::cout << "Decomissioning failed" << std::endl;
        return false;
    }
    std::unordered_map<block_id_t, block_id_t> mapping = ls.compressGates();
    if (mapping.size() != 2) {
        std::cout << "Compression failed" << std::endl;
        return false;
    }
    if (ls.getState(mapping[gate1]) || !ls.getState(mapping[gate3])) {
        return false;
    }
    return true;
}

bool runSimulatorTests() {

    LogicSimulator ls = LogicSimulator();
    bool fail = false;
    if (!testTwoGates(ls)) {
        std::cout << "Test \"two gates\" failed" << std::endl;
        fail = true;
    }
    if (!testThreeGates(ls)) {
        std::cout << "Test \"three gates\" failed" << std::endl;
        fail = true;
    }
    if (!testFullAdder(ls)) {
        std::cout << "Test \"full adder\" failed" << std::endl;
        fail = true;
    }
    if (!testDecomissioning(ls)) {
        std::cout << "Test \"decomissioning\" failed" << std::endl;
        fail = true;
    }

    if (!fail) {
        std::cout << "All tests passed" << std::endl;
    }
    return !fail;
}