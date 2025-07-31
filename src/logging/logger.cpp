#include "logger.h"

#ifdef TRACY_PROFILER
	#include <tracy/Tracy.hpp>
#endif

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
	//output logs to cout
	case LogType::Info:
		categoryText = "Info" + categoryText;
		std::cout << "[" << ANSI_INFO << categoryText << ANSI_TAIL << "] " << message << "\n";
		std::cout.flush();
		break;
	case LogType::Warning:
		categoryText = "Warning" + categoryText;
		std::cout << "[" << ANSI_WARNING << categoryText << ANSI_TAIL << "] " << message << "\n";
		std::cout.flush();
		break;
	case LogType::Error:
		categoryText = "ERROR" + categoryText;
		std::cerr << "[" << ANSI_ERROR << categoryText << ANSI_TAIL << "] " << message << "\n";
		break;
	case LogType::Fatal:
		categoryText = "FATAL" + categoryText;
		std::cerr << "[" << ANSI_FATAL << categoryText << ANSI_TAIL << "] " << message << "\n";
		break;
	}

	// and output to the log file
	#ifdef TRACY_PROFILER
		std::string msg = "[" + categoryText + "] " + message;
		TracyMessage(msg.c_str(), msg.size());
	#endif
	outputFileStream << "[" << categoryText << "] " << message << "\n";
	outputFileStream.flush();
}
