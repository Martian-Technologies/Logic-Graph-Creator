#ifndef directoryManager_h
#define directoryManager_h

#include <filesystem>

class DirectoryManager {
public:
	static void findDirectories();

	// The resource data holds immutable data shipped with the application (images, shaders, etc)
	inline static const std::filesystem::path& getResourceDirectory() { return resourceDirectory; }
	// The working directory is the location of the open "project"
	inline static const std::filesystem::path& getWorkingDirectory() { return workingDirectory; }
	// The config directory is the location that config files should placed (shared between "projects")
	inline static const std::filesystem::path& getConfigDirectory() { return configDirectory; }

private:
	static std::filesystem::path resourceDirectory;
	static std::filesystem::path workingDirectory;
	static std::filesystem::path configDirectory;
};

#endif
