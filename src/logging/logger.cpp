#include "logger.h"

Logger::Logger(const std::filesystem::path& outputFile) : outputFile(outputFile) {
	outputFileStream.open(outputFile);
	outputFileStream.close();
}

void Logger::log(LogType type, const std::string& message) {
	switch (type) {
	case LogType::Info:
		fileBuffer << "[Info] " + message + "\n";
		std::cout << "[\033[1;37mInfo\033[0m] " << message << "\n";
		break;
	case LogType::Warning:
		fileBuffer << "[Warning] " + message + "\n";
		std::cout << "[\033[1;33mWarning\033[0m] " << message << "\n";
		break;
	case LogType::Error:
		fileBuffer << "[ERROR] " << message << "\n";
		std::cerr << "[\033[1;31mERROR\033[0m] " << message << "\n";;
		break;
	case LogType::Fatal:
		fileBuffer << "[FATAL] " << message << "\n";
		std::cerr << "[\033[1;4;41;97mFATAL\033[0m] " + message + "\n";
		break;
	}

	flushToFile();
}

void Logger::flushToFile() {
	outputFileStream.open(outputFile, std::ios::app);
	outputFileStream << fileBuffer.str();
	outputFileStream.close();
	fileBuffer.str(""); // clears the buffer
}
