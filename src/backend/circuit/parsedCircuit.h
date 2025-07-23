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
		BlockData(FPosition position, Rotation rotation, BlockType type) : position(position), rotation(rotation), type(type) { }
		BlockData(BlockType type) : type(type) { }

		FPosition position = FPosition::getInvalid(); // will be validated into integer values
		Rotation rotation = Rotation::ZERO; // todo: make into integer value to generalize the rotation
		BlockType type = BlockType::NONE;
	};

	struct ConnectionData {
		ConnectionData(block_id_t outputBlockId, connection_end_id_t outputEndId, block_id_t inputBlockId, connection_end_id_t inputEndId) :
			outputBlockId(outputBlockId), outputEndId(outputEndId), inputBlockId(inputBlockId), inputEndId(inputEndId) { }
		block_id_t outputBlockId;
		connection_end_id_t outputEndId;
		block_id_t inputBlockId;
		connection_end_id_t inputEndId;

		bool operator==(const ConnectionData& other) const {
			return outputEndId == other.outputEndId && inputEndId == other.inputEndId &&
				outputBlockId == other.outputBlockId && inputBlockId == other.inputBlockId;
		}
	};

	struct ConnectionPort {
		ConnectionPort(
			bool isInput,
			connection_end_id_t connectionEndId,
			Vector positionOnBlock,
			block_id_t internalBlockId,
			connection_end_id_t internalBlockConnectionEndId,
			const std::string& portName = ""
		) : isInput(isInput), connectionEndId(connectionEndId), positionOnBlock(positionOnBlock),
			internalBlockId(internalBlockId), internalBlockConnectionEndId(internalBlockConnectionEndId), portName(portName) { }
		ConnectionPort(
			bool isInput,
			connection_end_id_t connectionEndId,
			Vector positionOnBlock,
			const std::string& portName = ""
		) : isInput(isInput), connectionEndId(connectionEndId), positionOnBlock(positionOnBlock), portName(portName) { }
		bool isInput;
		connection_end_id_t connectionEndId;
		Vector positionOnBlock;
		block_id_t internalBlockId = 0;
		connection_end_id_t internalBlockConnectionEndId = 0;
		std::string portName = "";
	};

	void addConnectionPort(
		bool isInput,
		connection_end_id_t connectionEndId,
		Vector positionOnBlock,
		block_id_t internalBlockId,
		connection_end_id_t internalBlockConnectionEndId,
		const std::string& portName = ""
	);
	void addConnectionPort(
		bool isInput,
		connection_end_id_t connectionEndId,
		Vector positionOnBlock,
		const std::string& portName = ""
	);
	const std::vector<ConnectionPort>& getConnectionPorts() const { return ports; }

	void addBlock(block_id_t id, FPosition pos, Rotation rotation, BlockType type);
	void addBlock(block_id_t id, BlockType type);
	void addConnection(block_id_t outputBlockId, connection_end_id_t outputEndId, block_id_t inputBlockId, connection_end_id_t inputEndId);

	const BlockData* getBlock(block_id_t id) const {
		auto itr = blocks.find(id);
		if (itr != blocks.end()) return &itr->second;
		return nullptr;
	}
	const std::unordered_map<block_id_t, BlockData>& getBlocks() const { return blocks; }
	const std::vector<ConnectionData>& getConns() const { return connections; }

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

private:
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
			return std::hash<block_id_t>()(p.outputEndId) ^ std::hash<block_id_t>()(p.outputEndId) ^
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
