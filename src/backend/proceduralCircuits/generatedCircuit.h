#ifndef generatedCircuit_h
#define generatedCircuit_h

#include "backend/container/block/connectionEnd.h"
#include "backend/position/sparse2d.h"

class GeneratedCircuit {
	friend class GeneratedCircuitValidator;
public:
	struct GeneratedCircuitBlockData {
		GeneratedCircuitBlockData(Position position, Rotation rotation, BlockType type) : position(position), rotation(rotation), type(type) { }
		Position position;
		Rotation rotation;
		BlockType type;
	};

	struct ConnectionData {
		block_id_t outputBlockId;
		connection_end_id_t outputId;
		block_id_t inputBlockId;
		connection_end_id_t inputId;

		bool operator==(const ConnectionData& other) const {
			return (
				outputBlockId == other.outputBlockId &&
				outputId == other.outputId &&
				inputBlockId == other.inputBlockId &&
				inputId == other.inputId
			);
		}
	};

	struct ConnectionPort {
		ConnectionPort(
			bool isInput,
			connection_end_id_t connectionEndId,
			Vector positionOnBlock,
			block_id_t internalBlockId,
			connection_end_id_t internalBlockConnectionEndId,
			const std::string& portName
		) : isInput(isInput), connectionEndId(connectionEndId), positionOnBlock(positionOnBlock), internalBlockId(internalBlockId), internalBlockConnectionEndId(internalBlockConnectionEndId), portName(portName) { }
		bool isInput;
		connection_end_id_t connectionEndId;
		Vector positionOnBlock;
		block_id_t internalBlockId;
		connection_end_id_t internalBlockConnectionEndId;
		std::string portName;
	};

	void addConnectionPort(
		bool isInput,
		connection_end_id_t connectionEndId,
		Vector positionOnBlock,
		block_id_t internalBlockId,
		connection_end_id_t internalBlockConnectionEndId,
		const std::string& portName
	);
	const std::vector<ConnectionPort>& getConnectionPorts() const { return ports; }


	block_id_t addBlock(Position pos, Rotation rotation, BlockType type);
	block_id_t addBlock(BlockType type);
	void addConnection(block_id_t outputBlockId, connection_end_id_t outputId, block_id_t inputBlockId, connection_end_id_t inputId);

	block_id_t getBlockId(Position position) const {
		const block_id_t* id = positionMap.get(position);
		return id ? *id : 0;
	}
	const GeneratedCircuitBlockData* getBlock(block_id_t id) const {
		auto itr = blocks.find(id);
		if (itr != blocks.end()) return &itr->second;
		return nullptr;
	}
	const GeneratedCircuitBlockData* getBlock(Position position) const {
		const block_id_t* id = positionMap.get(position);
		return id ? getBlock(*id) : nullptr;
	}
	const std::unordered_map<block_id_t, GeneratedCircuitBlockData>& getBlocks() const { return blocks; }
	const std::vector<ConnectionData>& getConns() const { return connections; }

	void setName(const std::string& name) { this->name = name; }
	const std::string& getName() const { return name; }

	Size getSize() const { return size; }
	void setSize(Size size) { this->size = size; valid = false; }

	void markAsCustom() { isCustomBlock = true; }
	bool isCustom() const { return isCustomBlock; }
	bool isValid() const { return valid; }

private:
	block_id_t blockIdCounter = 0;
	block_id_t getNewBlockId() { return ++blockIdCounter; }

	std::string uuid;
	std::string name;

	bool isCustomBlock;
	Size size;
	std::vector<ConnectionPort> ports;

	std::unordered_map<block_id_t, GeneratedCircuitBlockData> blocks;
	Sparse2dArray<block_id_t> positionMap;
	std::vector<ConnectionData> connections;

	bool valid = true;
};

typedef std::shared_ptr<GeneratedCircuit> SharedGeneratedCircuit;

#endif /* generatedCircuit_h */
