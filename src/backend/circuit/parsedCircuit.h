#ifndef parsedCircuit_h
#define parsedCircuit_h

#include "backend/blockData/blockDataManager.h"
#include "backend/container/block/connectionEnd.h"
#include "backend/position/position.h"

class CircuitManager;

class ParsedCircuit {
	friend class CircuitValidator;
public:
	struct BlockData {
		BlockData() { } // nothing bad block
		BlockData(FPosition pos, Rotation rotation, BlockType type) : pos(pos), rotation(rotation), type(type) { }
		BlockData(BlockType type) : type(type) { }

		FPosition pos = FPosition(std::numeric_limits<float>::max(), std::numeric_limits<float>::max()); // will be validated into integer values
		Rotation rotation = Rotation::ZERO; // todo: make into integer value to generalize the rotation
		BlockType type = BlockType::NONE;
	};

	struct ConnectionData {
		ConnectionData(block_id_t outputBlockId, connection_end_id_t outputId, block_id_t inputBlockId, connection_end_id_t inputId) :
			outputBlockId(outputBlockId), outputId(outputId), inputBlockId(inputBlockId), inputId(inputId) { }
		block_id_t outputBlockId;
		connection_end_id_t outputId;
		block_id_t inputBlockId;
		connection_end_id_t inputId;

		bool operator==(const ConnectionData& other) const {
			return outputId == other.outputId && inputId == other.inputId &&
				outputBlockId == other.outputBlockId && inputBlockId == other.inputBlockId;
		}
	};

	struct ConnectionPort {
		ConnectionPort(bool isInput, connection_end_id_t connectionEndId, Vector positionOnBlock, block_id_t block, const std::string& portName) :
			isInput(isInput), connectionEndId(connectionEndId), positionOnBlock(positionOnBlock), block(block), portName(portName) { }
		ConnectionPort(bool isInput, connection_end_id_t connectionEndId, Vector positionOnBlock, block_id_t block, connection_end_id_t internalBlockConnectionEndId, const std::string& portName) :
			isInput(isInput), connectionEndId(connectionEndId), positionOnBlock(positionOnBlock), block(block), internalBlockConnectionEndId(internalBlockConnectionEndId), portName(portName) { }
		bool isInput;
		connection_end_id_t connectionEndId;
		Vector positionOnBlock;
		block_id_t block;
		connection_end_id_t internalBlockConnectionEndId = 0; // for switch and light this is fine
		std::string portName;
	};

	void addConnectionPort(bool isInput, connection_end_id_t connectionEndId, Vector positionOnBlock, block_id_t id, const std::string& portName);
	const std::vector<ConnectionPort>& getConnectionPorts() const { return ports; }


	void addBlock(block_id_t id, const BlockData& block);
	void addConnection(const ConnectionData& conn);

	const BlockData* getBlock(block_id_t id) const {
		auto itr = blocks.find(id);
		if (itr != blocks.end()) return &itr->second;
		return nullptr;
	}
	const std::unordered_map<block_id_t, BlockData>& getBlocks() const { return blocks; }
	const std::vector<ConnectionData>& getConns() const { return connections; }

	void makePositionsRelative();

	void setAbsoluteFilePath(const std::string& fpath) { absoluteFilePath = fpath; }
	const std::string& getAbsoluteFilePath() const { return absoluteFilePath; }

	void setName(const std::string& name) { this->name = name; }
	const std::string& getName() const { return name; }

	void setUUID(const std::string& uuid) { this->uuid = uuid; }
	const std::string& getUUID() const { return uuid; }

	Vector getSize() const { return size; }
	void setSize(Vector size) { this->size = size; valid = false; }

	void markAsCustom() { isCustomBlock = true; }
	bool isCustom() const { return isCustomBlock; }
	bool isValid() const { return valid; }
	Vector getMinPos() const { return minPos; }

private:
	Vector minPos = Vector(std::numeric_limits<int>::max(), std::numeric_limits<int>::max());
	Vector maxPos = Vector(std::numeric_limits<int>::min(), std::numeric_limits<int>::min()); // TODO: delete this because I think it is unused

	std::string absoluteFilePath;
	std::string uuid;
	std::string name;

	// If this represents a custom block:
	bool isCustomBlock;
	Vector size;

	std::vector<ConnectionPort> ports; // connection id is the index in the vector

	std::unordered_map<block_id_t, BlockData> blocks;
	std::vector<ConnectionData> connections;

	bool valid = true;
};

typedef std::shared_ptr<ParsedCircuit> SharedParsedCircuit;

class CircuitValidator {
public:
	CircuitValidator(ParsedCircuit& parsedCircuit, BlockDataManager* blockDataManager) : parsedCircuit(parsedCircuit), blockDataManager(blockDataManager) { validate(); }
private:
	struct ConnectionHash {
		size_t operator()(const ParsedCircuit::ConnectionData& p) const {
			return std::hash<block_id_t>()(p.outputId) ^ std::hash<block_id_t>()(p.inputId) ^
				std::hash<connection_end_id_t>()(p.outputBlockId) ^ std::hash<connection_end_id_t>()(p.inputBlockId);
		}
	};

	void validate();
	bool validateBlockData();
	bool validateBlockTypes();
	bool setBlockPositionsInt();
	bool handleInvalidConnections();
	bool setOverlapsUnpositioned();

	bool handleUnpositionedBlocks();

	bool isIntegerPosition(const FPosition& pos) const {
		return pos.x == std::floor(pos.x) && pos.y == std::floor(pos.y);
	}
	block_id_t generateNewBlockId() const {
		block_id_t id = 0;
		// slow
		while (parsedCircuit.blocks.find(id) != parsedCircuit.blocks.end()) {
			++id;
		}
		return id;
	}

	BlockDataManager* blockDataManager;
	ParsedCircuit& parsedCircuit;
	std::unordered_set<Position> occupiedPositions;
};

#endif /* parsedCircuit_h */
