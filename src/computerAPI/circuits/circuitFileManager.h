#ifndef circuitFileManager_h
#define circuitFileManager_h

#include <QString>

#include "backend/circuit/circuitManager.h"

// dont change the numbers next to enum values
// (app name)_(file version)
enum CircuitFileTypes {
	GATALITY_1 = 0
};

class CircuitFileManager {
public:
	inline CircuitFileManager(CircuitManager* circuitManager) : circuitManager(circuitManager) {}
	
	std::optional<circuit_id_t> load(const QString& path);
	bool loadInto(const QString& path, circuit_id_t circuit, const Position& position);

	bool save(const QString& path, circuit_id_t circuit);

private:
	struct saveInfo {
		circuit_update_count lastUpdateSaved;
		QString filePath;
	};

	CircuitManager* circuitManager;
	std::unordered_map<circuit_id_t, saveInfo> circuitSaveInfo;
};

#endif /* circuitFileManager_h */