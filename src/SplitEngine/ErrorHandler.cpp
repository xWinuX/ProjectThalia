#include "SplitEngine/ErrorHandler.hpp"
#include "SplitEngine/Debug/Log.hpp"

namespace SplitEngine
{
	void ErrorHandler::ThrowRuntimeError(const std::string& message)
	{
		LOG_FATAL(message);
		throw std::runtime_error(message);
	}
}
