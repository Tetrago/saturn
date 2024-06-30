#ifndef SATURN_UTIL_HPP
#define SATURN_UTIL_HPP

#include <vulkan/vulkan.h>

#include <string_view>

namespace sat
{
	std::string_view name_of(VkResult result) noexcept;
}

#endif
