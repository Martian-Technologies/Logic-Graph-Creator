#ifndef circuitFileManager_h
#define circuitFileManager_h

#include <QString>

#include "backend/circuit/circuitManager.h"

class CircuitFileManager {
public:
	CircuitFileManager(const CircuitManager* circuitManager);
	
    bool loadInto(const QString& path, circuit_id_t circuitId, const Position& cursorPosition);
    bool save(const QString& path, circuit_id_t circuitId);

private:
    // not implemented yet.
	struct saveInfo {
		circuit_update_count lastUpdateSaved;
		QString filePath;
	};

	const CircuitManager* circuitManager;
	std::unordered_map<circuit_id_t, saveInfo> circuitSaveInfo;
};

#endif /* circuitFileManager_h */
