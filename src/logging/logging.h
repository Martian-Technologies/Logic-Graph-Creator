#ifndef logging_h
#define logging_h

#include <format>

// basic string logging
void logInfo(const std::string& message, const std::string& subcategory = "");
void logWarning(const std::string& message, const std::string& subcategory = "");
void logError(const std::string& message, const std::string& subcategory = "");
void logFatalError(const std::string& message, const std::string& subcategory = "");

// fancy formatted logging
template<typename ...Args>
void logInfo(const std::format_string<Args...>& fmt, const std::string& subcategory, Args&&...args) {
	std::ostringstream message;
	message << std::format(fmt, std::forward<Args>(args)...);
	logInfo(message.str(), subcategory);
}

template<typename ...Args>
void logWarning(const std::format_string<Args...>& fmt, const std::string& subcategory, Args&&...args) {
	std::ostringstream message;
	message << std::format(fmt, std::forward<Args>(args)...);
	logWarning(message.str(), subcategory);
}

template<typename ...Args>
void logError(const std::format_string<Args...>& fmt, const std::string& subcategory, Args&&...args) {
	std::ostringstream message;
	message << std::format(fmt, std::forward<Args>(args)...);
	logError(message.str(), subcategory);
}

template<typename ...Args>
void logFatalError(const std::format_string<Args...>& fmt, const std::string& subcategory, Args&&...args) {
	std::ostringstream message;
	message << std::format(fmt, std::forward<Args>(args)...);
	logFatalError(message.str(), subcategory);
}


#endif /* logging_h */
