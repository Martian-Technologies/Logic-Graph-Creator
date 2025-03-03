#ifndef blockData_h
#define blockData_h

#include "backend/position/position.h"
#include "connectionEnd.h"
#include "blockDefs.h"

class BlockData {
	friend class BlockDataManager;
public:
	// allows the reader to spend less time if we are looking at 1x1 blocks with one input and one output
	inline block_size_t getWidth() const noexcept { return width; }
	inline block_size_t getHeight() const noexcept { return height; }
	inline std::pair<connection_end_id_t, bool> getInputConnectionId(const Vector& vector) const noexcept {
		if (defaultData) return { 0, vector.dx == 0 && vector.dy == 0 };
		for (auto pair : inputs) if (pair.first == vector) return {pair.second, true};
		return {0, false};
	}
	inline std::pair<connection_end_id_t, bool> getOutputConnectionId(const Vector& vector) const noexcept {
		if (defaultData) return { 1, vector.dx == 0 && vector.dy == 0 };
		for (auto pair : outputs) if (pair.first == vector) return {pair.second, true};
		return {0, false};
	}
	inline std::pair<Vector, bool> getConnectionVector(connection_end_id_t connectionId) const noexcept {
		if (defaultData) return { Vector(0, 0), connectionId < 2 };
		for (auto pair : inputs) if (pair.second == connectionId) return {pair.first, true};
		for (auto pair : outputs) if (pair.second == connectionId) return {pair.first, true};
		return {Vector(), false};
	}
	inline connection_end_id_t getMaxConnectionId() const noexcept {
		if (defaultData) return 1;
		return inputs.size() + outputs.size() - 1;
	}
	inline bool isConnectionInput(connection_end_id_t connectionId) const noexcept {
		if (defaultData) return connectionId == 0;
		for (auto pair : inputs) if (pair.second == connectionId) return true;
		return false;
	}
	inline bool isPlaceable() const noexcept { return placeable; }
	inline const std::string& getName() const noexcept { return name; }
	inline const std::string& getPath() const noexcept { return path; }

private:
	bool defaultData = true;
	bool placeable = true;
	std::string name = "Unnamed Block";
	std::string path = "Basic";
	block_size_t width = 1;
	block_size_t height = 1;
	std::vector<std::pair<Vector, connection_end_id_t>> inputs;
	std::vector<std::pair<Vector, connection_end_id_t>> outputs;
};

#endif /* blockData_h */
