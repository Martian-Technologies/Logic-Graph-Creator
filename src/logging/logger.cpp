#include "logger.h"

#define ANSI_INFO "\033[1;37m"
#define ANSI_WARNING "\033[1;33m"
#define ANSI_ERROR "\033[1;31m"
#define ANSI_FATAL "\033[1;4;41;97m"
#define ANSI_TAIL "\033[0m"

Logger::Logger(const std::filesystem::path& outputFile)
	: outputFile(outputFile), outputFileStream(outputFile), outStream(std::cout), errStream(std::cerr), fileStream(outputFileStream) {
}

void Logger::log(LogType type, const std::string& message, const std::string& subcategory) {
	std::string categoryText; 
	if (subcategory != "") {
		categoryText = " - " + subcategory;
	}
	
	switch (type) {
	case LogType::Info:
		categoryText = "Info" + categoryText;
		fileStream << "[" << categoryText << "] " << message << "\n";
		outStream << "[" << ANSI_INFO << categoryText << ANSI_TAIL << "] " << message << "\n";
		break;
	case LogType::Warning:
		categoryText = "Warning" + categoryText;
		fileStream << "[" << categoryText << "] " << message << "\n";
		outStream << "[" << ANSI_WARNING << categoryText << ANSI_TAIL << "] " << message << "\n";
		break;
	case LogType::Error:
		categoryText = "ERROR" + categoryText;
		fileStream << "[" << categoryText << "] " << message << "\n";
		errStream << "[" << ANSI_ERROR << categoryText << ANSI_TAIL << "] " << message << "\n";
		break;
	case LogType::Fatal:
		categoryText = "FATAL" + categoryText;
		fileStream << "[" << categoryText << "] " << message << "\n";
		errStream << "[" << ANSI_FATAL << categoryText << ANSI_TAIL << "] " << message << "\n";
		break;
	}
	
	outStream.emit();
	errStream.emit();
	fileStream.emit();
}
