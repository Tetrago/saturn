#ifndef SATURN_LOG_HPP
#define SATURN_LOG_HPP

#include <format>
#include <string_view>

#include "core.hpp"

namespace sat
{
	enum class LogLevel
	{
		Trace = 0,
		Info,
		Warn,
		Error
	};

	////////////////
	//// Logger ////
	////////////////

	class SATURN_API Logger
	{
	public:
		Logger(LogLevel level = LogLevel::Error) noexcept;
		virtual ~Logger() noexcept = default;

		Logger(const Logger&)            = delete;
		Logger& operator=(const Logger&) = delete;

		void log(LogLevel level, std::string_view message) noexcept;

		template <typename... Args>
		void log(LogLevel level,
		         std::format_string<Args...> fmt,
		         Args&&... args) noexcept
		{
			log(level, std::format(fmt, std::forward<Args>(args)...));
		}

	protected:
		virtual void logMessage(LogLevel level,
		                        std::string_view message) noexcept
		{}

	private:
		LogLevel level_;
	};

	////////////////////////
	//// Console Logger ////
	////////////////////////

	class SATURN_API ConsoleLogger : public Logger
	{
	public:
		explicit ConsoleLogger(const std::string& name,
		                       LogLevel level = LogLevel::Info) noexcept;

	protected:
		void logMessage(LogLevel level,
		                std::string_view message) noexcept override;

	private:
		std::string name_;
	};
} // namespace sat

#endif
