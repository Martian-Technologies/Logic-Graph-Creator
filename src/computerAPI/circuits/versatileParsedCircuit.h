#ifndef versatileParsedCircuit_h
#define versatileParsedCircuit_h

#include "backend/container/block/connectionEnd.h"
#include "backend/position/position.h"

class VersatileParsedCircuit {
	friend class CircuitValidator;
public:
	struct BlockData {
		BlockData(FPosition position, Rotation rotation, const std::string& type) : position(position), rotation(rotation), type(type) { }
		BlockData(const std::string& type) : type(type) { }

		FPosition position = FPosition::getInvalid(); // will be validated into integer values
		Rotation rotation = Rotation::ZERO; // todo: make into integer value to generalize the rotation
		std::string type;
		std::vector<>
	};

	struct ConnectionData {
		ConnectionData(const std::string& outputPortIdentifier, const std::string& inputPortIdentifier) :
			outputPortIdentifier(outputPortIdentifier), inputPortIdentifier(inputPortIdentifier) { }
		bool operator==(const ConnectionData& other) const {
			return outputPortIdentifier == other.outputPortIdentifier && inputPortIdentifier == other.inputPortIdentifier;
		}
		std::string outputPortIdentifier;
		std::string inputPortIdentifier;
	};

	struct ConnectionPort {
		ConnectionPort(bool isInput, const std::string& portName, Vector positionOnBlock, const std::string& internalPort = "") :
			isInput(isInput), portName(portName), positionOnBlock(positionOnBlock), internalPort(internalPort) { }
		bool isInput;
		std::string portIdentifier = "";
		Vector positionOnBlock;
		std::string internalPortIdentifier = "";
	};

	void addConnectionPort(bool isInput, const std::string& portName, Vector positionOnBlock, const std::string& internalPort = "");
	const std::vector<ConnectionPort>& getConnectionPorts() const { return ports; }

	void addBlock(const std::string& blockIdentifier, FPosition position, Rotation rotation, const std::string& type);
	void addBlock(const std::string& blockIdentifier, const std::string& type);
	void addConnection(const std::string& outputPortIdentifier, const std::string& inputPortIdentifier);

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

typedef std::shared_ptr<VersatileParsedCircuit> SharedVersatileParsedCircuit;

#endif /* versatileParsedCircuit_h */
