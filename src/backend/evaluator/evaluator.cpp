#include <functional>

#include "evaluator.h"

Evaluator::Evaluator(std::shared_ptr<BlockContainerWrapper> blockContainerWrapper)
    :paused(false),
    targetTickrate(0),
    logicSimulator(),
    addressTree() {
    const auto blockContainer = blockContainerWrapper->getBlockContainer();
    const Difference difference = blockContainer->getCreationDifference();

    makeEdit(std::make_shared<Difference>(difference), blockContainerWrapper->getContainerId());

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
    const auto modifications = difference->getModifications();
    for (const auto& modification : modifications) {
        const auto& [modificationType, modificationData] = modification;
        switch (modificationType) {
        case Difference::REMOVED_BLOCK: {
            const auto& [position, rotation, blockType] = std::get<Difference::block_modification_t>(modificationData);
            const auto address = Address(position);
            const block_id_t blockId = addressTree.getValue(address);
            logicSimulator.decomissionGate(blockId);
            addressTree.removeValue(address);
            break;
        }
        case Difference::PLACE_BLOCK: {
            const auto& [position, rotation, blockType] = std::get<Difference::block_modification_t>(modificationData);
            const auto address = Address(position);
            const GateType gateType = blockContainerToEvaluatorGatetype(blockType);
            const block_id_t blockId = logicSimulator.addGate(gateType, true);
            addressTree.addValue(address, blockId);
            break;
        }
        case Difference::REMOVED_CONNECTION: {
            const auto& [outputPosition, inputPosition] = std::get<Difference::connection_modification_t>(modificationData);
            const auto outputAddress = Address(outputPosition);
            const auto inputAddress = Address(inputPosition);
            const block_id_t outputBlockId = addressTree.getValue(outputAddress);
            const block_id_t inputBlockId = addressTree.getValue(inputAddress);
            logicSimulator.disconnectGates(outputBlockId, inputBlockId);
            break;
        }
        case Difference::CREATED_CONNECTION: {
            const auto& [outputPosition, inputPosition] = std::get<Difference::connection_modification_t>(modificationData);
            const auto outputAddress = Address(outputPosition);
            const auto inputAddress = Address(inputPosition);
            const block_id_t outputBlockId = addressTree.getValue(outputAddress);
            const block_id_t inputBlockId = addressTree.getValue(inputAddress);
            logicSimulator.connectGates(outputBlockId, inputBlockId);
            break;
        }
        default:
            throw std::invalid_argument("makeEdit: invalid modificationType");
        }
    }
    logicSimulator.debugPrint();
}

GateType blockContainerToEvaluatorGatetype(BlockType blockType) {
    switch (blockType) {
    case BlockType::AND:
        return GateType::AND;
    case BlockType::OR:
        return GateType::OR;
    case BlockType::XOR:
        return GateType::XOR;
    case BlockType::NAND:
        return GateType::NAND;
    case BlockType::NOR:
        return GateType::NOR;
    case BlockType::XNOR:
        return GateType::XNOR;
    default:
        throw std::invalid_argument("blockContainerToEvaluatorGatetype: invalid blockType");
    }
}