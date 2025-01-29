#ifndef logger_h
#define logger_h

#include <filesystem>

enum LogType { Verbose, Info, Warning, Error, Fatal };

void logVerbose(const std::string& message);
void logInfo(const std::string& message);
void logWarning(const std::string& message);
void logError(const std::string& message);
void fatalError(const std::string& message);

class Logger {
public:
	Logger(const std::filesystem::path& outputFile, LogType logIntensity);

	void log(LogType type, const std::string& message);

private:
	std::filesystem::path outputFile;
	std::ofstream outputFileStream;

	std::stringstream fileBuffer;

	LogType logIntensity;

private:
	void flushToFile();
};

#endif
