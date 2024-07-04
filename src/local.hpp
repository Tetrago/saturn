#ifndef SATURN_LOCAL_HPP
#define SATURN_LOCAL_HPP

#include <algorithm>
#include <span>
#include <sstream>
#include <stdexcept>
#include <unordered_set>

#include "instance.hpp"
#include "logger.hpp"
#include "util.hpp"

#define VK_CALL(call, logger, expection)                                  \
	{                                                                     \
		VkResult res = (call);                                            \
		if (res != VK_SUCCESS)                                            \
		{                                                                 \
			(logger).log(LogLevel::Error, "{}: {}", #call, name_of(res)); \
			throw std::runtime_error(expection);                          \
		}                                                                 \
	}

namespace sat
{
	template <typename T, typename R>
	inline bool evaluate_required_items(Logger& logger,
	                                    std::span<const char* const> required,
	                                    std::span<T const> available,
	                                    R T::*ref,
	                                    std::string_view label)
	{
		// Get a set of all required extensions
		std::unordered_set<std::string> remaining;
		std::ranges::transform(
		    required,
		    std::inserter(remaining, remaining.begin()),
		    [](const char* name) { return std::string(name); });

		std::ostringstream oss;
		oss << "Found " << label << ':';

		// Evaluate all instance extensions available on system
		for (const T& props : available)
		{
			auto res = remaining.find(props.*ref);
			if (res == remaining.end())
			{
				// Extension is not needed
				oss << "\n [ ] " << props.*ref;
			}
			else
			{
				// Extension is required
				oss << "\n [X] " << props.*ref;
				remaining.erase(res);
			}
		}

		logger.log(LogLevel::Trace, oss.str());

		if (remaining.empty())
		{
			// If all extensions have been found, return
			return true;
		}
		else
		{
			// Otherwise, log all missing extensions

			oss.str("");
			oss << "Missing " << label << ':';

			for (const std::string& name : remaining)
			{
				oss << "\n [-] " << name;
			}

			logger.log(LogLevel::Error, oss.str());

			return false;
		}
	}
} // namespace sat

#define S_TRACE(instance, ...) \
	(instance)->logger().log(LogLevel::Trace, __VA_ARGS__)

#define S_INFO(instance, ...) \
	(instance)->logger().log(LogLevel::Info, __VA_ARGS__)

#define S_WARN(instance, ...) \
	(instance)->logger().log(LogLevel::Warn, __VA_ARGS__)

#define S_ERROR(instance, ...) \
	(instance)->logger().log(LogLevel::Error, __VA_ARGS__)

#endif
