#ifndef resourceManager_h
#define resourceManager_h

#include <filesystem>

class ResourceManager {
public:
	static void initializeResourceDirectory();
	inline static const std::filesystem::path& getResourceDirectory() { return resourceDirectory; }

private:
	static std::filesystem::path resourceDirectory; 
};

#endif
