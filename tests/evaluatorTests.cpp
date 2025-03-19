#include "evaluatorTests.h"
#include "util/uuid.h"

// Note that logic simulator is tested separately
void EvaluatorTest::SetUp() {
    circuit_id_t circuitId = circuitManager.createNewCircuit("Circuit", generate_uuid_v4());
	circuit = circuitManager.getCircuit(circuitId);
    evaluator = std::make_shared<Evaluator>(1, circuitManager, circuitId);
    i = 0;
}

void EvaluatorTest::TearDown() {
    // remove ptr references
    circuit.reset();
    evaluator.reset();
}
/*
TEST_F(EvaluatorTest, InitTest) {
    ASSERT_EQ(evaluator->getEvaluatorId(), 1);

    // 0 when paused, set to be paused on evaluator's constructor
    ASSERT_EQ(evaluator->getRealTickrate(), 0);
}

TEST_F(EvaluatorTest, PauseUnpauseTest) {
	evaluator->setPause(false);
    evaluator->setUseTickrate(false);
    // set to 1000000000 tick/min
    // tickrate should be ~16666666.7 ?
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
    ASSERT_GT(evaluator->getRealTickrate(), 0);
	
	evaluator->setPause(true);
	std::this_thread::sleep_for(std::chrono::seconds(2));
	ASSERT_EQ(evaluator->getRealTickrate(), 0);
}

TEST_F(EvaluatorTest, TickrateTest) {
    int new_tickrate = 1000;
	evaluator->setTickrate(new_tickrate);
	evaluator->setUseTickrate(true);
	evaluator->setPause(false);
	
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	
	ASSERT_GT(evaluator->getRealTickrate(), 0);
	//ASSERT_EQ(evaluator->getRealTickrate(), new_tickrate/60); // have to sleep for a couple seconds

    evaluator->setUseTickrate(false);
	//std::this_thread::sleep_for(std::chrono::seconds(1));
    // tickrate should grow
    //ASSERT_GT(evaluator->getRealTickrate(), new_tickrate);

    evaluator->setPause(true);
}


TEST_F(EvaluatorTest, BasicStateManagement) {
	Position pos(i, i); ++i;
	Rotation rot = Rotation::ZERO;
	
    // insert switch
	circuit->tryInsertBlock(pos, rot, BlockType::SWITCH);
	
	Address addr(pos);
	ASSERT_EQ(evaluator->getState(addr), false);
	
	// set state
	evaluator->setState(addr, true);
	ASSERT_EQ(evaluator->getState(addr), true);
}

TEST_F(EvaluatorTest, BulkStateOperations) {
	std::vector<Position> positions = {
		Position(i, i),
		Position(i+1, i),
		Position(i+2, i)
	};
    i+=3;
	
	// place switches
	for (const Position& pos : positions) {
		circuit->tryInsertBlock(pos, Rotation::ZERO, BlockType::SWITCH);
	}
	
	std::vector<Address> addresses;
	for (const Position& pos : positions) {
		addresses.push_back(Address(pos));
	}
	
    std::vector<logic_state_t> states = evaluator->getBulkStates(addresses);
	ASSERT_EQ(states.size(), addresses.size());
	for (logic_state_t state : states) {
		ASSERT_EQ(state, false);
	}
}

TEST_F(EvaluatorTest, LogicGateEvaluation) {
	// AND gate with two inputs
	Position andPos(i, i); ++i;
	Position in1(i, i); ++i;
	Position in2(i, i); ++i;
	
	circuit->tryInsertBlock(andPos, Rotation::ZERO, BlockType::AND);
	circuit->tryInsertBlock(in1, Rotation::ZERO, BlockType::SWITCH);
	circuit->tryInsertBlock(in2, Rotation::ZERO, BlockType::SWITCH);
	
	circuit->tryCreateConnection(in1, andPos);
	circuit->tryCreateConnection(in2, andPos);
	
	// set input states
	evaluator->setState(Address(in1), true);
	evaluator->setState(Address(in2), true);
	
	// run simulation
	evaluator->setPause(false);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	
	// check AND gate output
	ASSERT_EQ(evaluator->getState(Address(andPos)), true);

	// change one input
	evaluator->setState(Address(in1), false);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	ASSERT_EQ(evaluator->getState(Address(andPos)), false);

    evaluator->setPause(true);
}

TEST_F(EvaluatorTest, EvaluatingCircuitModifications) {
	Position pos1(i, i); ++i;
	Position pos2(i, i); ++i;
	
	circuit->tryInsertBlock(pos1, Rotation::ZERO, BlockType::AND);
	circuit->tryInsertBlock(pos2, Rotation::ZERO, BlockType::OR);
	circuit->tryCreateConnection(pos1, pos2);
	
    // removal
	circuit->tryRemoveBlock(pos1);
	
	// retrieving state from address should have a check if it doesn't exist
    // As of now we don't have a check and we attempt to retrieve a key that doesn't exist in unordered map
	//Address removedAddr(pos1);
	//ASSERT_EQ(evaluator->getState(removedAddr), false);

	// block movement
    evaluator->setState(pos2, true);
	Position newPos(i, i); ++i;
	circuit->tryMoveBlock(pos2, newPos);

	// reaccess addresses
	//ASSERT_EQ(evaluator->getState(Address(pos2)), false); // This old address will cause an error as it is not found in address tree
	ASSERT_EQ(evaluator->getState(Address(newPos)), true);
}

TEST_F(EvaluatorTest, GateTypeConversion) {
    // test block type conversions
	struct TestCase {
		BlockType blockType;
		GateType expectedGateType;
	};
	
	std::vector<TestCase> testCases = {
		{BlockType::AND, GateType::AND},
		{BlockType::OR, GateType::OR},
		{BlockType::XOR, GateType::XOR},
		{BlockType::NAND, GateType::NAND},
		{BlockType::NOR, GateType::NOR},
		{BlockType::XNOR, GateType::XNOR},
		{BlockType::SWITCH, GateType::DEFAULT_RETURN_CURRENTSTATE},
		{BlockType::BUTTON, GateType::DEFAULT_RETURN_CURRENTSTATE},
		{BlockType::TICK_BUTTON, GateType::TICK_INPUT},
		{BlockType::LIGHT, GateType::OR}
	};
	
	for (const TestCase& testCase : testCases) {
		ASSERT_EQ(circuitToEvaluatorGatetype(testCase.blockType), testCase.expectedGateType);
	}
	
	ASSERT_THROW(circuitToEvaluatorGatetype(static_cast<BlockType>(-1)), std::invalid_argument);
}

TEST_F(EvaluatorTest, ThreadSafetyAndPausing) {
	Position pos(i, i); ++i;
	circuit->tryInsertBlock(pos, Rotation::ZERO, BlockType::SWITCH);
	Address addr(pos);
	
	// test rapid state changes
	evaluator->setPause(false);
	
    std::thread stateChanger([this, addr]() { changeState(addr); });
    std::thread stateReader([this, addr]() { readState(addr); });
	
	stateChanger.join();
	stateReader.join();
	
    bool finalState = evaluator->getState(addr);
    // the change state is ran 100 times, ending on 99%2==0
    ASSERT_TRUE(finalState==false) << "Not synced threads";

    evaluator->setPause(true);
}

void EvaluatorTest::changeState(const Address& addr) {
    for (int j = 0; j < 100; j++) {
        //evaluator->setState(addr, j % 2 == 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void EvaluatorTest::readState(const Address& addr) {
    for (int j=0; j<100; ++j) {
        //evaluator->getState(addr);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

TEST_F(EvaluatorTest, FastCircuitModifications) {
	std::vector<Position> positions;
	for (int j=0; j<10; ++j) {
		positions.push_back(Position(i, i)); ++i;
		circuit->tryInsertBlock(positions.back(), Rotation::ZERO, BlockType::AND);
	}
	
	for (int j=0; j<(int)positions.size()-1; ++j) {
		circuit->tryCreateConnection(positions[j], positions[j+1]);
	}
	
	for (int j=0; j<(int)positions.size(); j+=2) {
		circuit->tryRemoveBlock(positions[j]);
	}
	
	// check that blocks still exist
	for (int j=1; j<(int)positions.size(); j+=2) {
		Address addr(positions[j]);
		ASSERT_NO_THROW(evaluator->getState(addr));
	}
}
*/
