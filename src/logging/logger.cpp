#include "logger.h"

Logger::Logger(const std::filesystem::path& outputFile) : outputFile(outputFile) {
	outputFileStream.open(outputFile);
	outputFileStream.close();
}

void Logger::log(LogType type, const std::string& message) {
	std::lock_guard<std::mutex> lock(mtx);
	std::string output;
	switch (type) {
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
}

void Logger::flushToFile() {
	outputFileStream.open(outputFile, std::ios::app);
	outputFileStream << fileBuffer.str();
	outputFileStream.close();
	fileBuffer.str(""); // clears the buffer
}
