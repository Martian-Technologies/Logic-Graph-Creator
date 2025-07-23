#include "logicSimulator.h"

LogicSimulator::LogicSimulator() 
    : running(true) {
    simulationThread = std::thread(&LogicSimulator::simulationLoop, this);
}

LogicSimulator::~LogicSimulator() {
    {
        std::lock_guard<std::mutex> lk(cvMutex);
        running = false;
        cv.notify_all();
    }
    if (simulationThread.joinable())
        simulationThread.join();
}

void LogicSimulator::simulationLoop()
{
    using clock = std::chrono::steady_clock;
    auto nextTick = clock::now();

    while (true) {
        {
            std::unique_lock<std::mutex> lk(cvMutex);
            cv.wait_until(lk, nextTick, [this]{ return !running || !paused; });
            if (!running) break;
        }

        tickOnce();

        // schedule next tick
        nextTick += std::chrono::nanoseconds(60'000'000'000ULL / targetTickrate.load());
    }
}

void LogicSimulator::tickOnce() {
    for (auto& gate : andGates) gate.tick(statesA, statesB);
    for (auto& gate : xorGates) gate.tick(statesA, statesB);
    for (auto& gate : constantResetGates) statesB[gate.id] = gate.outputState;
    for (auto& gate : copySelfOutputGates) statesB[gate.id] = statesA[gate.id];

    // junctions are special because they need to act instantly, so they run at the end of the tick
    for (auto& gate : junctions) gate.tick(statesA, statesB);
}