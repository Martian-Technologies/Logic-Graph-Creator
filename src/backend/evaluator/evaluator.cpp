#include <functional>

#include "evaluator.h"

Evaluator::Evaluator(std::shared_ptr<BlockContainerWrapper> blockContainerWrapper)
    :running(false),
    paused(false),
    targetTickrate(0),
    logicSimulator(),
    addressTree() {
    // TODO: implement initializing from blockcontainer

    const auto& blockContainer = blockContainerWrapper->getBlockContainer();

    // connect makeEdit to blockContainerWrapper
    blockContainerWrapper->connectListener(this, std::bind(&Evaluator::makeEdit, this, std::placeholders::_1, std::placeholders::_2));
}

void Evaluator::pause() {
    paused = true;
}

void Evaluator::unpause() {
    paused = false;
}

void Evaluator::reset() {
    logicSimulator.initialize(); // wipes all the states
}

void Evaluator::setTickrate(unsigned long long tickrate) {
    assert(tickrate > 0);
    targetTickrate = tickrate;
}

void Evaluator::runNTicks(unsigned long long n) {
    // TODO: make this happen in the thread via leaky bucket
    logicSimulator.simulateNTicks(n);
}

void Evaluator::makeEdit(DifferenceSharedPtr difference, block_container_wrapper_id_t containerId) {
    assert(false); // TODO: implement accepting a diff and integrating it into the logic simulator
}