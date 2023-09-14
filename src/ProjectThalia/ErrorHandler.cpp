#include "ProjectThalia/ErrorHandler.hpp"
#include "ProjectThalia/Debug/Log.hpp"

#include <format>

namespace ProjectThalia
{
	void ErrorHandler::ThrowRuntimeError(const std::string& message) {
		Debug::Log::Fatal(message);
		throw std::runtime_error(message);
	}
}
