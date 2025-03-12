#include "resourceManager.h"

const std::vector<std::filesystem::path> possibleResourceDirectories = {
	"resources"
};

std::filesystem::path ResourceManager::resourceDirectory;

void ResourceManager::initializeResourceDirectory() {
	for (const auto& path : possibleResourceDirectories) {
		if (std::filesystem::exists(path)) {
			resourceDirectory = path;
			logInfo("Found resource directory at (" + resourceDirectory.string() + ")");
			return;
		}
	}
	logFatalError("Could not find resource directory. Make sure you are executing the program from the top of the source tree.");
}
