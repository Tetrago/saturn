#ifndef SATURN_UTIL_HPP
#define SATURN_UTIL_HPP

#include <vulkan/vulkan.h>

#include <exception>
#include <string>
#include <string_view>

#include "core.hpp"

#define SATURN_THROW(type, ...) \
	throw ::sat::type(__FILE__, __LINE__, __VA_ARGS__)

#ifndef NDEBUG
	#define SATURN_CALL(call)                             \
		([&]() {                                          \
			VkResult res = (call);                        \
			if (res != VK_SUCCESS)                        \
			{                                             \
				throw ::sat::UnsuccessfulResultException( \
					__FILE__, __LINE__, #call, res);      \
			}                                             \
		}())

	#define SATURN_GET(instance, function)                     \
		([&]() {                                               \
			auto fn = reinterpret_cast<PFN_##function>(        \
				vkGetInstanceProcAddr((instance), #function)); \
			if (fn != nullptr)                                 \
			{                                                  \
				return fn;                                     \
			}                                                  \
			else                                               \
			{                                                  \
				throw ::sat::MissingFeatureException(          \
					__FILE__, __LINE__, #function);            \
			}                                                  \
		}())
#else
	#include <stdexcept>

	#define SATURN_CALL(call)     \
		if ((call) != VK_SUCCESS) \
		throw std::runtime_error("Failed to call Vulkan function")

	#define SATURN_GET(instance, function)                                     \
		([&]() {                                                               \
			auto fn = reinterpret_cast<PFN_##function>((instance), #function); \
			if (fn != nullptr)                                                 \
			{                                                                  \
				return fn;                                                     \
			}                                                                  \
			else                                                               \
			{                                                                  \
				throw std::runtime_error(                                      \
					"Failed to retreived vulkan function");                    \
			}                                                                  \
		}())
#endif

namespace sat
{
	SATURN_API std::string_view name_of(VkResult result) noexcept;

	class SATURN_API UnsuccessfulResultException : public std::exception
	{
	public:
		UnsuccessfulResultException(std::string_view file,
		                            int line,
		                            std::string_view call,
		                            VkResult result) noexcept
		    : file_(file), line_(line), call_(call), result_(result)
		{}

		const char* what() const noexcept override;

		std::string_view file() const noexcept { return file_; }

		int line() const noexcept { return line_; }

		std::string_view call() const noexcept { return call_; }

		VkResult result() const noexcept { return result_; }

	private:
		std::string_view file_;
		int line_;
		std::string_view call_;
		VkResult result_;

		mutable std::string what_;
	};

	class SATURN_API MissingFeatureException : public std::exception
	{
	public:
		MissingFeatureException(std::string_view file,
		                        int line,
		                        std::string_view feature) noexcept
		    : file_(file), line_(line), feature_(feature)
		{}

		const char* what() const noexcept override;

	private:
		std::string_view file_;
		int line_;
		std::string_view feature_;

		mutable std::string what_;
	};
} // namespace sat

#endif
