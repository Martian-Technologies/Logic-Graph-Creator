#include "logging.h"
#include "logger.h"

Logger mainLogger("connection_machine.log");

void logInfo(const std::string& message, const std::string& subcategory) {
	mainLogger.log(LogType::Info, message, subcategory);
}

void logWarning(const std::string& message, const std::string& subcategory) {
	mainLogger.log(LogType::Warning, message, subcategory);
}

void logError(const std::string& message, const std::string& subcategory) {
	mainLogger.log(LogType::Error, message, subcategory);
}

void logFatalError(const std::string& message, const std::string& subcategory) {
	mainLogger.log(LogType::Fatal, message, subcategory);
}
