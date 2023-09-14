#include "ProjectThalia/Debug/Log.hpp"

#include <chrono>
#include <format>
#include <iostream>

namespace ProjectThalia::Debug
{
	void Log::Info(const std::string& message) { Print(message, LogLevel::Info); }

	void Log::Warning(const std::string& message) { Print(message, LogLevel::Warning); }

	void Log::Error(const std::string& message) { Print(message, LogLevel::Error); }

	void Log::Fatal(const std::string& message) { Print(message, LogLevel::Fatal); }

	void Log::Print(const std::string& message, LogLevel logLevel)
	{
		std::string formattedMessage = std::format("{} [{}] {}", GetDateTime(), LogLevelToString(logLevel), message);
		if (logLevel < 2) { std::cout << formattedMessage << std::endl; }
		else { std::cerr << formattedMessage << std::endl; }
	}

	std::string Log::LogLevelToString(LogLevel logLevel)
	{
		switch (logLevel)
		{
			case Debug::Info: return "Info";
			case Debug::Warning: return "Warning";
			case Debug::Error: return "Error";
			case Debug::Fatal: return "Fatal";
		}
	}

	std::string Log::GetDateTime()
	{
		auto time = std::chrono::system_clock::now();

		return std::format("{:%H:%M:%S}", time);
	}
}
