#pragma once

#include <string>

namespace ProjectThalia
{
	class ErrorHandler
	{
		public:
			static void ThrowRuntimeError(const std::string& message);
	};

}
