#ifndef logger_h
#define logger_h

#include <filesystem>

enum LogType {
	Info = 1,
	Warning = 2,
	Error = 4,
	Fatal = 8
};

class Logger {
public:
	Logger(const std::filesystem::path& outputFile);

	void log(LogType type, const std::string& message);

private:
	void flushToFile();

	std::filesystem::path outputFile;
	std::ofstream outputFileStream;

	std::stringstream fileBuffer;
};

#endif /* logger_h */
