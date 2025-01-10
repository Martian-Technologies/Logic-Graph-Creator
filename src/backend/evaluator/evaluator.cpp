#include "evaluator.h"

Evaluator::Evaluator(std::shared_ptr<Circuit> circuit)
	:paused(true),
	targetTickrate(0),
	logicSimulator(),
	addressTree(circuit->getContainerId()),
	usingTickrate(false) {
	setTickrate(40 * 60); // 40 Hz
	const auto blockContainer = circuit->getBlockContainer();
	const Difference difference = blockContainer->getCreationDifference();
	makeEdit(std::make_shared<Difference>(difference), circuit->getContainerId());
	circuit->connectListener(this, std::bind(&Evaluator::makeEdit, this, std::placeholders::_1, std::placeholders::_2)); // this should actually be circuitManager aggregating all the circuits' diffs
}

logic_state_t Evaluator::getState(const Address& address) {
	return false;
}

std::vector<logic_state_t> Evaluator::getBulkStates(const std::vector<Address>& addresses) {
	std::vector<logic_state_t> states;
	for (const Address& address : addresses) {
		states.push_back(false);
	}
	return states;
}

void Evaluator::setTickrate(unsigned long long tickrate) {}
void Evaluator::setUseTickrate(bool useTickrate) {}
long long int Evaluator::getRealTickrate() const { return 0; }
void Evaluator::makeEdit(DifferenceSharedPtr difference, circuit_id_t containerId) {

}
void Evaluator::setPause(bool pause) {}
void Evaluator::setState(const Address& address, logic_state_t state) {}