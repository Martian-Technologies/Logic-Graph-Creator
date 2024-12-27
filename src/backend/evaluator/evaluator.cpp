#include <functional>

#include "evaluator.h"

Evaluator::Evaluator(std::shared_ptr<BlockContainerWrapper> blockContainerWrapper)
    :running(false),
    paused(false),
    targetTickrate(0),
    logicSimulator() {
    // TODO: implement initializing from blockcontainer



    // connect makeEdit to blockContainerWrapper
    blockContainerWrapper->connectListener(this, std::bind(&Evaluator::makeEdit, this, std::placeholders::_1));
}

void Evaluator::stop() {
    running = false;
    paused = false;
}

void Evaluator::start() {
    running = true;
    paused = false;
}

void Evaluator::start(unsigned long long tickrate) {
    running = true;
    paused = false;
    targetTickrate = tickrate;
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
    targetTickrate = tickrate;
}

void Evaluator::runNTicks(unsigned long long n) {
    // TODO: make this happen in the thread via leaky bucket
    logicSimulator.simulateNTicks(n);
}

void Evaluator::makeEdit(DifferenceSharedPtr difference) {
    assert(false); // TODO: implement accepting a diff and integrating it into the logic simulator
}