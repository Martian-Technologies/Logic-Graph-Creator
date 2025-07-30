#ifndef renderManager_h
#define renderManager_h

#include <functional>

#include "backend/address.h"
#include "backend/container/block/blockDefs.h"
#include "backend/container/difference.h"

class Circuit;

struct RenderConnectionEnd {
	Position otherBlock;
	bool isInput;
};

struct WireHash {
    std::size_t operator () (const std::pair<Position,Position> &p) const {
        auto h1 = std::hash<Position>{}(p.first);
        auto h2 = std::hash<Position>{}(p.second);

		// temp hash
        return h1 ^ h2;
    }
};

// probably should have just used a messaging system for this, but I want a thread safe queue dangit
class CircuitRenderer {
public:
	virtual void startMakingEdits() = 0;
	virtual void stopMakingEdits() = 0;
	virtual void addBlock(BlockType type, Position position, Vector size, Rotation rotation) = 0;
	virtual void removeBlock(Position position) = 0;
	virtual void moveBlock(Position curPos, Position newPos, Rotation newRotation, Vector size) = 0; // moves JUST the block
	virtual void addWire(std::pair<Position, Position> points, std::pair<FVector, FVector> socketOffsets, Address address) = 0;
	virtual void removeWire(std::pair<Position, Position> points) = 0;
};

// Currently, everything in this class is meant to run on the event loop thread, meaning synchronization is not needed. This might be prone to change.
class CircuitRenderManager {
public:
	CircuitRenderManager(Circuit* circuit);
	void addDifference(DifferenceSharedPtr diff);
	void getMeUpToSpeed(CircuitRenderer* renderer);
	void connect(CircuitRenderer* circuitRenderer);
	void disconnect(CircuitRenderer* circuitRenderer);

private:
	void addDifference(DifferenceSharedPtr diff, const std::set<CircuitRenderer*>& renderers);
	Address getAddressOfConnection(std::pair<Position, Position> connection);

private:
	Circuit* circuit;
	std::set<CircuitRenderer*> connectedRenderers;

	std::unordered_map<Position, std::unordered_map<std::pair<Position, Position>, RenderConnectionEnd, WireHash>> blockToConnections;
	std::unordered_map<Position, Rotation> blockToRotation;
	std::unordered_map<Position, BlockType> blockToType;
};

#endif
