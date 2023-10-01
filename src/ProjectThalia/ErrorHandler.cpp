#include "ProjectThalia/ErrorHandler.hpp"
#include "ProjectThalia/Debug/Log.hpp"

namespace ProjectThalia
{
	void ErrorHandler::ThrowRuntimeError(const std::string& message) {
		LOG_FATAL(message);
		throw std::runtime_error(message);
	}
}
