#ifndef blockDataManager_h
#define blockDataManager_h

#include "backend/dataUpdateEventManager.h"
#include "blockData.h"

class BlockDataManager {
public:
	BlockDataManager(DataUpdateEventManager* dataUpdateEventManager) : dataUpdateEventManager(dataUpdateEventManager) {
		// load default data
		for (unsigned int i = 0; i < 13; i++) addBlock();
		getBlockData(BlockType::AND)->setName("And");
		getBlockData(BlockType::OR)->setName("Or");
		getBlockData(BlockType::XOR)->setName("Xor");
		getBlockData(BlockType::NAND)->setName("Nand");
		getBlockData(BlockType::NOR)->setName("Nor");
		getBlockData(BlockType::XNOR)->setName("Xnor");
		getBlockData(BlockType::JUNCTION)->setName("Junction");
		// TRISTATE_BUFFER
		getBlockData(BlockType::TRISTATE_BUFFER)->setName("Tristate Buffer");
		getBlockData(BlockType::TRISTATE_BUFFER)->setDefaultData(false);
		getBlockData(BlockType::TRISTATE_BUFFER)->setConnectionInput(Vector(0, 1), 0);
		getBlockData(BlockType::TRISTATE_BUFFER)->setConnectionInput(Vector(0, 0), 1);
		getBlockData(BlockType::TRISTATE_BUFFER)->setConnectionOutput(Vector(0, 1), 2);
		getBlockData(BlockType::TRISTATE_BUFFER)->setSize(Vector(1, 2));
		// BUTTON
		getBlockData(BlockType::BUTTON)->setName("Button");
		getBlockData(BlockType::BUTTON)->setDefaultData(false);
		getBlockData(BlockType::BUTTON)->setConnectionOutput(Vector(0), 0);
		// TICK_BUTTON
		getBlockData(BlockType::TICK_BUTTON)->setName("Tick Button");
		getBlockData(BlockType::TICK_BUTTON)->setDefaultData(false);
		getBlockData(BlockType::TICK_BUTTON)->setConnectionOutput(Vector(0), 0);
		// SWITCH
		getBlockData(BlockType::SWITCH)->setName("Switch");
		getBlockData(BlockType::SWITCH)->setDefaultData(false);
		getBlockData(BlockType::SWITCH)->setConnectionOutput(Vector(0), 0);
		// CONSTANT
		getBlockData(BlockType::CONSTANT)->setName("Constant");
		getBlockData(BlockType::CONSTANT)->setDefaultData(false);
		getBlockData(BlockType::CONSTANT)->setIsPlaceable(false);
		getBlockData(BlockType::CONSTANT)->setConnectionOutput(Vector(0), 0);
		// LIGHT
		getBlockData(BlockType::LIGHT)->setName("Light");
		getBlockData(BlockType::LIGHT)->setDefaultData(false);
		getBlockData(BlockType::LIGHT)->setConnectionInput(Vector(0), 0);
	}

	inline BlockType addBlock() noexcept {
		blockData.emplace_back((BlockType)(blockData.size()+1), dataUpdateEventManager);
		sendBlockDataUpdate();
		return (BlockType) blockData.size();
	}

	inline BlockType getBlockType(const std::string& blockPath) const {
		for (unsigned int i = 0; i < blockData.size(); i++) {
			if (blockData[i].getPath() + "/" + blockData[i].getName() == blockPath) {
				return (BlockType)(i + 1);
			}
		}
		return BlockType::NONE;
	}

	inline void sendBlockDataUpdate() { dataUpdateEventManager->sendEvent("blockDataUpdate"); }

	inline const BlockData* getBlockData(BlockType type) const noexcept { if (!blockExists(type)) return nullptr; return &blockData[type-1]; }
	inline BlockData* getBlockData(BlockType type) noexcept { if (!blockExists(type)) return nullptr; return &blockData[type-1]; }

	inline unsigned int maxBlockId() const noexcept { return blockData.size(); }
	inline bool blockExists(BlockType type) const noexcept { return type != BlockType::NONE && type <= blockData.size(); }
	inline bool isPlaceable(BlockType type) const noexcept {
		if (!blockExists(type)) return false;
		return blockData[type-1].isPlaceable();
	}
	
	inline std::string getName(BlockType type) const noexcept {
		if (!blockExists(type)) return "None" + std::to_string(type);
		return blockData[type-1].getName();
	}
	inline std::string getPath(BlockType type) const noexcept {
		if (!blockExists(type)) return "Path To None";
		return blockData[type-1].getPath();
	}

	inline Vector getBlockSize(BlockType type) const noexcept {
		if (!blockExists(type)) return Vector();
		return blockData[type-1].getSize();
	}
	inline Vector getBlockSize(BlockType type, Rotation rotation) const noexcept {
		if (!blockExists(type)) return Vector();
		return blockData[type-1].getSize(rotation);
	}

	inline std::pair<connection_end_id_t, bool> getInputConnectionId(BlockType type, const Vector& vector) const noexcept {
		if (!blockExists(type)) return {0, false};
		return blockData[type-1].getInputConnectionId(vector);
	}
	inline std::pair<connection_end_id_t, bool> getOutputConnectionId(BlockType type, const Vector& vector) const noexcept {
		if (!blockExists(type)) return {0, false};
		return blockData[type-1].getOutputConnectionId(vector);
	}
	inline std::pair<connection_end_id_t, bool> getInputConnectionId(BlockType type, Rotation rotation, const Vector& vector) const noexcept {
		if (!blockExists(type)) return {0, false};
		return blockData[type-1].getInputConnectionId(vector, rotation);
	}
	inline std::pair<connection_end_id_t, bool> getOutputConnectionId(BlockType type, Rotation rotation, const Vector& vector) const noexcept {
		if (!blockExists(type)) return {0, false};
		return blockData[type-1].getOutputConnectionId(vector, rotation);
	}

	inline std::pair<Vector, bool> getConnectionVector(BlockType type, connection_end_id_t connectionId) const noexcept {
		if (!blockExists(type)) return {Vector(), false};
		return blockData[type-1].getConnectionVector(connectionId);
	}
	inline std::pair<Vector, bool> getConnectionVector(BlockType type, Rotation rotation, connection_end_id_t connectionId) const noexcept {
		if (!blockExists(type)) return {Vector(), false};
		return blockData[type-1].getConnectionVector(connectionId, rotation);
	}

	inline connection_end_id_t getConnectionCount(BlockType type) const noexcept {
		if (!blockExists(type)) return 0;
		return blockData[type-1].getConnectionCount();
	}
	inline bool isConnectionInput(BlockType type, connection_end_id_t connectionId) const noexcept {
		if (!blockExists(type)) return false;
		return blockData[type-1].isConnectionInput(connectionId);
	}

private:
	std::vector<BlockData> blockData;
	DataUpdateEventManager* dataUpdateEventManager;
};

#endif /* blockDataManager_h */
