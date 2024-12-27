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
    auto it = blockContainer->begin();
    const auto end = blockContainer->end();
    for (; it != end; ++it) {
        const Block& block = it->second;
        GateType gateType = GateType::NONE;
        switch (block.type()) {
        case BlockType::AND:
            gateType = GateType::AND;
            break;
        case BlockType::OR:
            gateType = GateType::OR;
            break;
        case BlockType::XOR:
            gateType = GateType::XOR;
            break;
        case BlockType::NAND:
            gateType = GateType::NAND;
            break;
        case BlockType::NOR:
            gateType = GateType::NOR;
            break;
        case BlockType::XNOR:
            gateType = GateType::XNOR;
            break;
        }
        addressTree.addValue(block.id(), logicSimulator.addGate(gateType));
    }

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