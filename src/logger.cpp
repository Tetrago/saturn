#include "logger.hpp"

#include <iostream>

namespace sat
{
	namespace
	{
		constexpr std::string_view TRACE   = "\033[0;90m";
		constexpr std::string_view INFO    = "\033[0;37m";
		constexpr std::string_view WARN    = "\033[0;33m";
		constexpr std::string_view ERROR   = "\033[0;31m";
		constexpr std::string_view UNKNOWN = "\033[0;36m";
		constexpr std::string_view RESET   = "\033[0m";

		constexpr std::string_view value_of(LogLevel level) noexcept
		{
			switch (level)
			{
			case LogLevel::Trace: return TRACE;
			case LogLevel::Info:  return INFO;
			case LogLevel::Warn:  return WARN;
			case LogLevel::Error: return ERROR;
			default:              return UNKNOWN;
			}
		}

		constexpr std::string_view string_of(LogLevel level) noexcept
		{
			switch (level)
			{
			case LogLevel::Trace: return "(trace)";
			case LogLevel::Info:  return "(info) ";
			case LogLevel::Warn:  return "(warn) ";
			case LogLevel::Error: return "(error)";
			default:              return "(unkwn)";
			}
		}
	} // namespace

	////////////////
	//// Logger ////
	////////////////

	Logger::Logger(LogLevel level) noexcept
	    : level_(level)
	{}

	void Logger::log(LogLevel level, std::string_view message) noexcept
	{
		if (static_cast<int>(level) >= static_cast<int>(level_))
		{
			logMessage(level, message);
		}
	}

	////////////////////////
	//// Console Logger ////
	////////////////////////

	ConsoleLogger::ConsoleLogger(const std::string& name,
	                             LogLevel level) noexcept
	    : Logger(level), name_(name)
	{}

	void ConsoleLogger::logMessage(LogLevel level,
	                               std::string_view message) noexcept
	{
		std::cout << value_of(level) << string_of(level) << " [" << name_
		          << "] " << message << RESET << std::endl;
	}
} // namespace sat
