#ifndef SATURN_CORE_HPP
#define SATURN_CORE_HPP

#include <memory>

#ifdef _MSC_VER
	#define SATURN_API_EXPORT __declspec(dllexport)
	#define SATURN_API_IMPORT __declspec(dllimport)
#else
	#define SATURN_API_EXPORT
	#define SATURN_API_IMPORT
#endif

#ifdef SATURN_SHARED
	#ifdef SATURN_BUILD
		#define SATURN_API SATURN_API_EXPORT
	#else
		#define SATURN_API SATURN_API_IMPORT
	#endif
#else
	#define SATURN_API
#endif

#ifndef SATURN_ENABLE_VALIDATION
	#ifndef NDEBUG
		#define SATURN_ENABLE_VALIDATION 1
	#else
		#define SATURN_ENABLE_VALIDATION 0
	#endif
#endif

namespace sat
{
	template <typename T>
	using rn = std::shared_ptr<T>;

	template <typename T, typename... Args>
	inline rn<T> make_rn(Args&&... args) noexcept
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}
} // namespace sat

#endif
