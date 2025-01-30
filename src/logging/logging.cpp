#include "logging.h"
#include "logger.h"

Logger mainLogger("gatality.log");

void logInfo(const std::string& message) {
	mainLogger.log(LogType::Info, message);
}

void logWarning(const std::string& message) {
	mainLogger.log(LogType::Warning, message);
}

void logError(const std::string& message) {
	mainLogger.log(LogType::Error, message);
}

void logFatalError(const std::string& message) {
	mainLogger.log(LogType::Fatal, message);
}
