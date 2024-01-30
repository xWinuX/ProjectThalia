#pragma once

#include <format>
#include <string>

// This miserable hack is needed because the MSVC preprocessor doesn't seem to expand __VA_ARGS__ correctly
#define PRIVATE_LOG_EXPAND(x) x

#define PRIVATE_LOG_GET_MACRO(type, message, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, NAME, ...) NAME
#define PRIVATE_LOG_BOILERPLATE(type, ...)                       \
	PRIVATE_LOG_EXPAND(PRIVATE_LOG_GET_MACRO(__VA_ARGS__,        \
											 PRIVATE_LOG_FORMAT, \
											 PRIVATE_LOG_FORMAT, \
											 PRIVATE_LOG_FORMAT, \
											 PRIVATE_LOG_FORMAT, \
											 PRIVATE_LOG_FORMAT, \
											 PRIVATE_LOG_FORMAT, \
											 PRIVATE_LOG_FORMAT, \
											 PRIVATE_LOG_FORMAT, \
											 PRIVATE_LOG_FORMAT, \
											 PRIVATE_LOG_FORMAT, \
											 PRIVATE_LOG_FORMAT, \
											 PRIVATE_LOG)(type, __VA_ARGS__))

#define PRIVATE_LOG_FORMAT(type, message, ...) SplitEngine::Debug::Log::type(std::format(message, __VA_ARGS__), __FILE__, __LINE__)
#define PRIVATE_LOG(type, message) SplitEngine::Debug::Log::type(message, __FILE__, __LINE__)

#ifdef SE_DEBUG_LOG
	#define LOG(...) PRIVATE_LOG_BOILERPLATE(Info, __VA_ARGS__)
	#define LOG_WARNING(...) PRIVATE_LOG_BOILERPLATE(Warning, __VA_ARGS__)
	#define LOG_ERROR(...) PRIVATE_LOG_BOILERPLATE(Error, __VA_ARGS__)
	#define LOG_FATAL(...) PRIVATE_LOG_BOILERPLATE(Fatal, __VA_ARGS__)
#else
	#define LOG(...) \
		do {         \
		} while (0)
	#define LOG_WARNING(...) \
		do {                 \
		} while (0)
	#define LOG_ERROR(...) \
		do {               \
		} while (0)
	#define LOG_FATAL(...) \
		do {               \
		} while (0)
#endif

namespace SplitEngine::Debug
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

			static void Info(const std::string& message, std::string file, int lineNumber);
			static void Warning(const std::string& message, std::string file, int lineNumber);
			static void Error(const std::string& message, std::string file, int lineNumber);
			static void Fatal(const std::string& message, std::string file, int lineNumber);

		private:
			static void        Print(const std::string& message, LogLevel logLevel);
			static void        Print(const std::string& message, LogLevel logLevel, std::string& file, int lineNumber);
			static std::string LogLevelToString(LogLevel logLevel);
			static std::string GetDateTime();
	};
}
