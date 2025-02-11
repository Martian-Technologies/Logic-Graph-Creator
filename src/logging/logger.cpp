#include "logger.h"

#define ANSI_INFO "\033[1;37m"
#define ANSI_WARNING "\033[1;33m"
#define ANSI_ERROR "\033[1;31m"
#define ANSI_FATAL "\033[1;4;41;97m"
#define ANSI_TAIL "\033[0m"

Logger::Logger(const std::filesystem::path& outputFile)
	: outputFile(outputFile), outputFileStream(outputFile) {
}

void Logger::log(LogType type, const std::string& message, const std::string& subcategory) {
	std::lock_guard<std::mutex> guard(loggingMutex);
	
	std::string categoryText; 
	if (subcategory != "") {
		categoryText = " - " + subcategory;
	}
	
	switch (type) {
	case LogType::Info:
		categoryText = "Info" + categoryText;
		outputFileStream << "[" << categoryText << "] " << message << "\n";
		std::cout << "[" << ANSI_INFO << categoryText << ANSI_TAIL << "] " << message << "\n";
		break;
	case LogType::Warning:
		categoryText = "Warning" + categoryText;
		outputFileStream << "[" << categoryText << "] " << message << "\n";
		std::cout << "[" << ANSI_WARNING << categoryText << ANSI_TAIL << "] " << message << "\n";
		break;
	case LogType::Error:
		categoryText = "ERROR" + categoryText;
		outputFileStream << "[" << categoryText << "] " << message << "\n";
		std::cerr << "[" << ANSI_ERROR << categoryText << ANSI_TAIL << "] " << message << "\n";
		break;
	case LogType::Fatal:
		categoryText = "FATAL" + categoryText;
		outputFileStream << "[" << categoryText << "] " << message << "\n";
		std::cerr << "[" << ANSI_FATAL << categoryText << ANSI_TAIL << "] " << message << "\n";
		break;
	}
}
