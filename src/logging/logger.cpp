#include "logger.h"

Logger mainLogger("gatality.log", LogType::Info);

void logVerbose(const std::string& message) { 
#ifdef VERBOSE_LOG
	mainLogger.log(LogType::Verbose, message);
#endif
}

void logInfo(const std::string& message) {
	mainLogger.log(LogType::Info, message);
}

void logWarning(const std::string& message) {
	mainLogger.log(LogType::Warning, message);
}

void logError(const std::string& message) {
	mainLogger.log(LogType::Error, message);
}

void fatalError(const std::string& message) {
	mainLogger.log(LogType::Fatal, message);
}

Logger::Logger(const std::filesystem::path& outputFile, LogType logIntensity)
	: outputFile(outputFile), logIntensity(logIntensity) {

	outputFileStream.open(outputFile);
	outputFileStream.close();
}

void Logger::log(LogType type, const std::string& message) {
	if (type >= logIntensity) {
		std::string output;
		switch (type) {
		case LogType::Verbose:
			output = "[Verbose] " + message + "\n";
			fileBuffer << output;
			std::cout << output;
			break;
		case LogType::Info:
			output = "[Info] " + message + "\n";
			fileBuffer << output;
			std::cout << output;
			break;
		case LogType::Warning:
			output = "[Warning] " + message + "\n";
			fileBuffer << output;
			std::cout << output;
			break;
		case LogType::Error:
			output = "[ERROR] " + message + "\n";
			fileBuffer << output;
			std::cerr << output;
			break;
		case LogType::Fatal:
			output = "[FATAL] " + message + "\n";
			fileBuffer << output;
			std::cerr << output;
			break;
		}

		flushToFile();

		if (type == LogType::Fatal) {
			throw std::runtime_error(message);
		}
	}
}

void Logger::flushToFile() {
	outputFileStream.open(outputFile, std::ios::app);
	outputFileStream << fileBuffer.str();
	outputFileStream.close();
	fileBuffer.clear();
}
