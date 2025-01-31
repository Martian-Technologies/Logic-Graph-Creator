#include "simulatorTests.h"

void SimulatorTest::SetUp() {
    // start up as a reset evaluator
	simulator.initialize();
}

void SimulatorTest::TearDown() {
	simulator.clearGates();
}

TEST_F(SimulatorTest, BasicGateOperations) {
	block_id_t andGate = simulator.addGate(GateType::AND);
	ASSERT_EQ(simulator.getState(andGate), false);
	
	simulator.setState(andGate, true);
	ASSERT_EQ(simulator.getState(andGate), true);
}

TEST_F(SimulatorTest, GateConnections) {
	block_id_t input1 = simulator.addGate(GateType::DEFAULT_RETURN_CURRENTSTATE);
	block_id_t input2 = simulator.addGate(GateType::DEFAULT_RETURN_CURRENTSTATE);
	block_id_t andGate = simulator.addGate(GateType::AND);
	
	simulator.connectGates(input1, andGate);
	simulator.connectGates(input2, andGate);
	
	simulator.setState(input1, true);
	simulator.setState(input2, true);
	
	simulator.simulateNTicks(1);
	
	ASSERT_EQ(simulator.getState(andGate), true);
}

TEST_F(SimulatorTest, GateDisconnection) {
	block_id_t input = simulator.addGate(GateType::DEFAULT_RETURN_CURRENTSTATE);
	block_id_t orGate = simulator.addGate(GateType::OR);
	
	simulator.connectGates(input, orGate);
	simulator.setState(input, true);
	simulator.simulateNTicks(1);
	ASSERT_EQ(simulator.getState(orGate), true);
	
	simulator.disconnectGates(input, orGate);
	simulator.simulateNTicks(1);
	ASSERT_EQ(simulator.getState(orGate), false);
}

TEST_F(SimulatorTest, GateDecommission) {
	block_id_t gate = simulator.addGate(GateType::AND);
	simulator.setState(gate, true);
	ASSERT_EQ(simulator.getState(gate), true);
	
	simulator.decomissionGate(gate);
	ASSERT_EQ(simulator.getState(gate), false);
	
	block_id_t newGate = simulator.addGate(GateType::OR, true);
	ASSERT_EQ(gate, newGate);
}

TEST_F(SimulatorTest, GateCompression) {
	std::vector<block_id_t> gates;
	for(int i=0; i<5; ++i) {
		gates.push_back(simulator.addGate(GateType::AND));
	}
	
	simulator.decomissionGate(gates[1]);
	simulator.decomissionGate(gates[3]);
	
	auto gateMap = simulator.compressGates();
	
	ASSERT_EQ(gateMap[gates[0]], 0);
	ASSERT_EQ(gateMap[gates[2]], 1);
	ASSERT_EQ(gateMap[gates[4]], 2);
}

TEST_F(SimulatorTest, LogicGateTypes) {
	struct TestCase {
		GateType type;
		std::vector<bool> inputs;
		bool expectedOutput;
	};
	
	std::vector<TestCase> testCases = {
		{GateType::AND, {true, true}, true},
		{GateType::AND, {true, false}, false},
		{GateType::OR, {false, true}, true},
		{GateType::OR, {false, false}, false},
		{GateType::XOR, {true, true}, false},
		{GateType::XOR, {true, false}, true},
		{GateType::NAND, {true, true}, false},
		{GateType::NOR, {false, false}, true},
		{GateType::XNOR, {true, true}, true}
	};
	
	for(const auto& test : testCases) {
		block_id_t output = simulator.addGate(test.type);
		std::vector<block_id_t> inputs;
		
		for(bool input : test.inputs) {
			block_id_t inGate = simulator.addGate(GateType::DEFAULT_RETURN_CURRENTSTATE);
			simulator.connectGates(inGate, output);
			simulator.setState(inGate, input);
			inputs.push_back(inGate);
		}
		
		simulator.simulateNTicks(1);
		ASSERT_EQ(simulator.getState(output), test.expectedOutput);
	}
}

TEST_F(SimulatorTest, TickrateControl) {
	// low tickrate
	simulator.setTargetTickrate(60);
	simulator.signalToProceed();
	std::this_thread::sleep_for(std::chrono::seconds(1));
	ASSERT_GT(simulator.getRealTickrate(), 0);
	ASSERT_LE(simulator.getRealTickrate(), 70);
	
	// high tickrate
	simulator.setTargetTickrate(1000000);
	simulator.signalToProceed();
	std::this_thread::sleep_for(std::chrono::seconds(1));
	ASSERT_GT(simulator.getRealTickrate(), 0);
}

TEST_F(SimulatorTest, StateManagement) {
	simulator.reserveGates(10);
	
	// test state computation and propagation
	block_id_t true_input_one = simulator.addGate(GateType::DEFAULT_RETURN_CURRENTSTATE);
	block_id_t true_input_two = simulator.addGate(GateType::DEFAULT_RETURN_CURRENTSTATE);
	block_id_t false_input = simulator.addGate(GateType::DEFAULT_RETURN_CURRENTSTATE);
    
    simulator.setState(true_input_one, true);
    simulator.setState(true_input_two, true);
    simulator.setState(false_input, false);

	block_id_t and1 = simulator.addGate(GateType::AND);
	block_id_t and2 = simulator.addGate(GateType::AND);
	block_id_t and3 = simulator.addGate(GateType::AND);
	
	simulator.connectGates(true_input_one, and1); // true and true
	simulator.connectGates(true_input_two, and1);

	simulator.connectGates(true_input_one, and2); // true and false
	simulator.connectGates(false_input, and2);

	simulator.connectGates(true_input_one, and3); // single true
	
	simulator.computeNextState();
	simulator.propagatePowered();
	simulator.swapStates();
	
	ASSERT_TRUE(simulator.getState(and1));
	ASSERT_FALSE(simulator.getState(and2));
	ASSERT_TRUE(simulator.getState(and3));
	
	auto currentState = simulator.getCurrentState();
	ASSERT_EQ(currentState.size(), 6);
}
