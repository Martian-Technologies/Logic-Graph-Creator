#ifndef logging_h
#define logging_h

void logInfo(const std::string& message, const std::string& subcategory = "");
void logWarning(const std::string& message, const std::string& subcategory = "");
void logError(const std::string& message, const std::string& subcategory = "");
void logFatalError(const std::string& message, const std::string& subcategory = "");

#endif /* logging_h */
