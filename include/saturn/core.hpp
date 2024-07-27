#ifndef SATURN_CORE_HPP
#define SATURN_CORE_HPP

#include <vulkan/vulkan.h>

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
	template <typename This, typename T>
	class Builder;

	/////////////////
	//// Wranger ////
	/////////////////

	template <typename T>
	class Wranger
	{
	public:
		explicit Wranger(T* ptr) noexcept;

		template <typename U>
		operator U() const noexcept;

		T* operator->() noexcept { return item_.get(); }

		const T* operator->() const noexcept { return item_.get(); }

		void reset() noexcept;

		T* get() noexcept { return item_.get(); }

		const T* get() const noexcept { return item_.get(); }

	private:
		std::shared_ptr<T> item_;
	};

	template <typename T>
	template <typename U>
	inline Wranger<T>::operator U() const noexcept
	{
		return *item_;
	}

	template <typename T>
	inline Wranger<T>::Wranger(T* ptr) noexcept
	    : item_(ptr)
	{}

	template <typename T>
	inline void Wranger<T>::reset() noexcept
	{
		item_.reset();
	}

	////////////
	//// rn ////
	////////////

	template <typename T>
	using rn = Wranger<T>;

	///////////////////
	//// Container ////
	///////////////////

	template <typename T>
	class Container
	{
	public:
		Container() noexcept;
		Container(T handle) noexcept;

		operator T() const noexcept { return handle_; }

		operator const T*() const noexcept { return &handle_; }

		T handle() const noexcept { return handle_; }

	protected:
		T handle_;
	};

	template <typename T>
	inline Container<T>::Container() noexcept
	    : handle_(VK_NULL_HANDLE)
	{}

	template <typename T>
	inline Container<T>::Container(T handle) noexcept
	    : handle_(handle)
	{}

	/////////////////
	//// Builder ////
	/////////////////

	template <typename This, typename T>
	class Builder
	{
	public:
		Wranger<T> build() const;
	};

	template <typename This, typename T>
	inline Wranger<T> Builder<This, T>::build() const
	{
		return Wranger<T>(new T(*static_cast<const This*>(this)));
	}
} // namespace sat

#endif
