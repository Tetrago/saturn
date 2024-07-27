#ifndef SATURN_SYNC_HPP
#define SATURN_SYNC_HPP

#include <vulkan/vulkan.h>

#include "core.hpp"

namespace sat
{
	class Device;
	class Fence;
	class Semaphore;

	//////////////
	//// Sync ////
	//////////////

	namespace sync
	{
		SATURN_API rn<Fence> fence(rn<Device> device, bool signaled = false);
		SATURN_API rn<Semaphore> semaphore(rn<Device> device);
	} // namespace sync

	///////////////
	//// Fence ////
	///////////////

	class SATURN_API Fence : public Container<VkFence>
	{
	public:
		~Fence() noexcept;

		Fence(const Fence&)            = delete;
		Fence& operator=(const Fence&) = delete;

		void wait() const noexcept;
		void reset() noexcept;

	private:
		friend rn<Fence> sync::fence(rn<Device>, bool);

		Fence(rn<Device> device, bool signaled) noexcept;

		rn<Device> device_;
	};

	///////////////////
	//// Semaphore ////
	///////////////////

	class SATURN_API Semaphore : public Container<VkSemaphore>
	{
	public:
		~Semaphore() noexcept;

		Semaphore(const Semaphore&)            = delete;
		Semaphore& operator=(const Semaphore&) = delete;

	private:
		friend rn<Semaphore> sync::semaphore(rn<Device>);

		Semaphore(rn<Device> device) noexcept;

		rn<Device> device_;
	};
} // namespace sat

#endif
