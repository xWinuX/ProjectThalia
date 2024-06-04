#include "SplitEngine/Debug/Log.hpp"

#include <chrono>
#include <iostream>

namespace SplitEngine::Debug
{
	void Log::Info(const std::string& message) { Print(message, LogLevel::Info); }

	void Log::Warning(const std::string& message) { Print(message, LogLevel::Warning); }

	void Log::Error(const std::string& message) { Print(message, LogLevel::Error); }

	void Log::Fatal(const std::string& message) { Print(message, LogLevel::Fatal); }

	void Log::Print(const std::string& message, const LogLevel logLevel)
	{
		const std::string formattedMessage = std::format("{0} [{1}] {2}", GetDateTime(), LogLevelToString(logLevel), message);
		if (logLevel < 2) { std::cout << formattedMessage << std::endl; }
		else { std::cerr << formattedMessage << std::endl; }
	}

	std::string Log::LogLevelToString(const LogLevel logLevel)
	{
		switch (logLevel)
		{
			case Debug::Info:
				return "Info";
			case Debug::Warning:
				return "Warning";
			case Debug::Error:
				return "Error";
			case Debug::Fatal:
				return "Fatal";
		}
		return {};
	}

	std::string Log::GetDateTime()
	{
		auto time = std::chrono::system_clock::now();

		return std::format("{:%H:%M:%S}", time);
	}

	void Log::Info(const std::string& message, std::string file, const int lineNumber) { Print(message, LogLevel::Info, file, lineNumber); }

	void Log::Warning(const std::string& message, std::string file, const int lineNumber) { Print(message, LogLevel::Warning, file, lineNumber); }

	void Log::Error(const std::string& message, std::string file, const int lineNumber) { Print(message, LogLevel::Error, file, lineNumber); }

	void Log::Fatal(const std::string& message, std::string file, const int lineNumber) { Print(message, LogLevel::Fatal, file, lineNumber); }

	void Log::Print(const std::string& message, LogLevel logLevel, std::string& file, int lineNumber)
	{
		// Get file name from path
		file = file.substr(file.find_last_of("/\\") + 1);

		Print(std::format("[{0}:{1}] ", file, lineNumber) + message, logLevel);
	}
}
