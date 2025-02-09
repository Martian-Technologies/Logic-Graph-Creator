#ifndef logger_h
#define logger_h

#include <syncstream>
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

	void log(LogType type, const std::string& message, const std::string& subcategory = "");

private:
	std::filesystem::path outputFile;
	std::ofstream outputFileStream;
	
	std::osyncstream outStream;
	std::osyncstream errStream;
	std::osyncstream fileStream;
};

#endif /* logger_h */
