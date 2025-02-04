#include "logger.h"

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
		asyncStream << "[\033[1;37m" + thingThatGoesBetweenTheBrackets + "\033[0m] " << message << "\n";
		break;
	case LogType::Warning:
		thingThatGoesBetweenTheBrackets = "Warning" + thingThatGoesBetweenTheBrackets;
		fileBuffer << "[" + thingThatGoesBetweenTheBrackets + "] " + message + "\n";
		asyncStream << "[\033[1;33m" + thingThatGoesBetweenTheBrackets + "\033[0m] " << message << "\n";
		break;
	case LogType::Error:
		thingThatGoesBetweenTheBrackets = "ERROR" + thingThatGoesBetweenTheBrackets;
		fileBuffer << "[" + thingThatGoesBetweenTheBrackets + "] " + message + "\n";
		asyncStream << "[\033[1;31m" + thingThatGoesBetweenTheBrackets + "\033[0m] " << message << "\n";;
		break;
	case LogType::Fatal:
		thingThatGoesBetweenTheBrackets = "FATAL" + thingThatGoesBetweenTheBrackets;
		fileBuffer << "[" + thingThatGoesBetweenTheBrackets + "] " + message + "\n";
		asyncStream << "[\033[1;4;41;97m" + thingThatGoesBetweenTheBrackets + "\033[0m] " + message + "\n";
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
