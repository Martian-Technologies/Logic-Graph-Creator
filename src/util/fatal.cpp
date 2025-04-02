#include "fatal.h"

void throwFatalError(const std::string& message) {
	logFatalError("Throwing fatal error with message: '{}'", "", message);
	throw std::runtime_error(message);
}
