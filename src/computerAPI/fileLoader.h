#ifndef fileLoader_h
#define fileLoader_h

#include <filesystem>

// TODO - not sure if this should be an optional, and be the responsibility of the caller to throw
std::vector<char> readFileAsBytes(const std::filesystem::path& path);

#endif
