#pragma once

#include <string>

namespace SplitEngine
{
	class ErrorHandler
	{
		public:
			static void ThrowRuntimeError(const std::string& message);
	};

}
