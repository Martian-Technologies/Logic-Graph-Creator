#ifndef renderManager_h
#define renderManager_h

#include "backend/container/difference.h"
#include "backend/address.h"

class Circuit;

// probably should have just used a messaging system for this, but I want a thread safe queue dangit
class CircuitRenderer {
public:
	virtual ~CircuitRenderer() = default;
	virtual void startMakingEdits() = 0;
	virtual void stopMakingEdits() = 0;
	virtual void addBlock(BlockType type, Position position, Vector size, Rotation rotation, Position statePosition) = 0;
	virtual void removeBlock(Position position) = 0;
	virtual void moveBlock(Position curPos, Position newPos, Rotation newRotation, Vector size) = 0; // moves JUST the block
	virtual void addWire(std::pair<Position, Position> points, std::pair<FVector, FVector> socketOffsets) = 0;
	virtual void removeWire(std::pair<Position, Position> points) = 0;
	virtual void reset() = 0;
};

// Currently, everything in this class is meant to run on the event loop thread, meaning synchronization is not needed. This might be prone to change.
class CircuitRenderManager {
public:
	CircuitRenderManager(Circuit* circuit);
	~CircuitRenderManager();
	void addDifference(DifferenceSharedPtr diff);
	void connect(CircuitRenderer* circuitRenderer);
	void disconnect(CircuitRenderer* circuitRenderer);

private:
	void addDifference(DifferenceSharedPtr diff, const std::set<CircuitRenderer*>& renderers);

private:
	struct RenderedBlock {
		RenderedBlock(BlockType type, Rotation rotation) : type(type), rotation(rotation) {}
		std::unordered_map<std::pair<Position, Position>, Position> connectionsToOtherBlock;
		BlockType type;
		Rotation rotation;
	};

	Circuit* circuit;
	std::set<CircuitRenderer*> connectedRenderers;

	std::unordered_map<Position, RenderedBlock> renderedBlocks;
};

#endif /* renderManager_h */
