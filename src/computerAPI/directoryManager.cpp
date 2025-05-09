#include "directoryManager.h"

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
		logInfo("Found resource directory relative to executable at ({})", "", resourceDirectory.string());
		return;
	}

	// check relative to macOS bundle
	std::filesystem::path relativeToBundle = getBundlePath().parent_path() / "resources";
	if (std::filesystem::exists(relativeToBundle)) {
		resourceDirectory = relativeToBundle;
		logInfo("Found resource directory relative to MacOS bundle at ({})", "", resourceDirectory.string());
		return;
	}

	// check for resources directory relative to executable's parent
	std::filesystem::path relativeToExecutableParent = getExecutablePath().parent_path().parent_path() / "resources";
	if (std::filesystem::exists(relativeToExecutableParent)) {
		resourceDirectory = relativeToExecutableParent;
		logInfo("Found resource directory relative to executable's parent at ({})", "", resourceDirectory.string());
		return;
	}

	// check for resources directory relative to working directory
	std::filesystem::path relativeToWorkingDirectory = "resources";
	if (std::filesystem::exists(relativeToWorkingDirectory)) {
		resourceDirectory = relativeToWorkingDirectory;
		logWarning("Found resource directory strictly relative to working directory, this is probably not intended.", "", resourceDirectory.string());
		return;
	}


	logFatalError("Could not find resource directory. Make sure you are executing the program from the top of the source tree.");
}

// thank you for this snippet Jacob Tate - https://gist.github.com/Jacob-Tate/7b326a086cf3f9d46e32315841101109
std::filesystem::path DirectoryManager::getExecutablePath() {
	return cpplocate::getExecutablePath();
}

std::filesystem::path DirectoryManager::getBundlePath() {
	return cpplocate::getBundlePath();
}
