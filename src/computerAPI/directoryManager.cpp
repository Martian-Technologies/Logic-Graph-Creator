#include "directoryManager.h"

const std::vector<std::filesystem::path> possibleResourceDirectories = {
	"resources"
};

std::filesystem::path DirectoryManager::resourceDirectory("");
std::filesystem::path DirectoryManager::workingDirectory("");
std::filesystem::path DirectoryManager::configDirectory("");

void DirectoryManager::findDirectories() {
	// only logic for finding resource directory right now, other will be set somehow
	
	for (const auto& path : possibleResourceDirectories) {
		if (std::filesystem::exists(path)) {
			resourceDirectory = path;
			logInfo("Found resource directory at (" + resourceDirectory.string() + ")");
			return;
		}
	}
	logFatalError("Could not find resource directory. Make sure you are executing the program from the top of the source tree.");
}
