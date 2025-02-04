#include "logger.h"

#define INFO_HEAD "[\033[1;37m"
#define WARNING_HEAD "[\033[1;33m"
#define ERROR_HEAD "[\033[1;31m"
#define FATAL_HEAD "[\033[1;4;41;97m"
#define ANSI_TAIL "\033[0m] "

Logger::Logger(const std::filesystem::path& outputFile) : asyncStream(std::cout), outputFile(outputFile) {
	outputFileStream.open(outputFile);
	outputFileStream.close();
}

void Logger::log(LogType type, const std::string& message, const std::string& subcategory) {
	std::string thingThatGoesBetweenTheBrackets;
	if (subcategory != "") {
		thingThatGoesBetweenTheBrackets = " - " + subcategory;
	}
	switch (type) {
	case LogType::Info:
		thingThatGoesBetweenTheBrackets = "Info" + thingThatGoesBetweenTheBrackets;
		fileBuffer << "[" + thingThatGoesBetweenTheBrackets + "] " + message + "\n";
		asyncStream << INFO_HEAD + thingThatGoesBetweenTheBrackets + ANSI_TAIL << message << "\n";
		break;
	case LogType::Warning:
		thingThatGoesBetweenTheBrackets = "Warning" + thingThatGoesBetweenTheBrackets;
		fileBuffer << "[" + thingThatGoesBetweenTheBrackets + "] " + message + "\n";
		asyncStream << WARNING_HEAD + thingThatGoesBetweenTheBrackets + ANSI_TAIL << message << "\n";
		break;
	case LogType::Error:
		thingThatGoesBetweenTheBrackets = "ERROR" + thingThatGoesBetweenTheBrackets;
		fileBuffer << "[" + thingThatGoesBetweenTheBrackets + "] " + message + "\n";
		asyncStream << ERROR_HEAD + thingThatGoesBetweenTheBrackets + ANSI_TAIL << message << "\n";
		break;
	case LogType::Fatal:
		thingThatGoesBetweenTheBrackets = "FATAL" + thingThatGoesBetweenTheBrackets;
		fileBuffer << "[" + thingThatGoesBetweenTheBrackets + "] " + message + "\n";
		asyncStream << FATAL_HEAD + thingThatGoesBetweenTheBrackets + ANSI_TAIL + message + "\n";
		break;
	}
	asyncStream.emit();
	flushToFile();
}

void Logger::flushToFile() {
	outputFileStream.open(outputFile, std::ios::app);
	outputFileStream << fileBuffer.str();
	outputFileStream.close();
	fileBuffer.str(""); // clears the buffer
}
