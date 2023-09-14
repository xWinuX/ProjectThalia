#pragma once

#include <string>

namespace ProjectThalia::Debug
{
	enum LogLevel
	{
		Info    = 0, // Just info
		Warning = 1, // fixable or not relevant
		Error   = 2, // recoverable error
		Fatal   = 3  // not recoverable error, application crashes
	};

	class Log
	{
		public:
			static void Info(const std::string& message);
			static void Warning(const std::string& message);
			static void Error(const std::string& message);
			static void Fatal(const std::string& message);

		private:
			static void        Print(const std::string& message, LogLevel logLevel);
			static std::string LogLevelToString(LogLevel logLevel);
			static std::string GetDateTime();
	};

}
