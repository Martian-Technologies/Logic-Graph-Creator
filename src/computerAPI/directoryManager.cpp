#include "directoryManager.h"
#include <unistd.h>

#include <cpplocate/cpplocate.h>

std::filesystem::path DirectoryManager::resourceDirectory("");
std::filesystem::path DirectoryManager::projectDirectory("");
std::filesystem::path DirectoryManager::configDirectory("");

void DirectoryManager::findDirectories() {
	// only logic for finding resource directory right now, other will be set later somehow

	// check for resources directory relative to executable
	std::filesystem::path relativeToExecutable = getExecutablePath().parent_path() / "resources";
	if (std::filesystem::exists(relativeToExecutable)) {
		resourceDirectory = relativeToExecutable;
		logInfo("Found resource directory at ({})", "", resourceDirectory.string());
		return;
	}

	// check for resources directory relative to working directory
	std::filesystem::path relativeToWorkingDirectory = "resources";
	if (std::filesystem::exists(relativeToWorkingDirectory)) {
		resourceDirectory = relativeToWorkingDirectory;
		logInfo("Found resource directory at ({})", "", resourceDirectory.string());
		return;
	}
	
	
	logFatalError("Could not find resource directory. Make sure you are executing the program from the top of the source tree.");
}

// thank you for this snippet Jacob Tate - https://gist.github.com/Jacob-Tate/7b326a086cf3f9d46e32315841101109
std::filesystem::path DirectoryManager::getExecutablePath() {
	return cpplocate::getExecutablePath();
}
