#include "sync.hpp"

#include <limits>

#include "device.hpp"
#include "error.hpp"

namespace sat
{
	//////////////
	//// Sync ////
	//////////////

	namespace sync
	{
		rn<Fence> fence(rn<Device> device, bool signaled)
		{
			return rn<Fence>(new Fence(std::move(device), signaled));
		}

		rn<Semaphore> semaphore(rn<Device> device)
		{
			return rn<Semaphore>(new Semaphore(std::move(device)));
		}
	} // namespace sync

	///////////////
	//// Fence ////
	///////////////

	Fence::Fence(rn<Device> device, bool signaled) noexcept
	    : device_(std::move(device))
	{
		VkFenceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

		if (signaled)
		{
			createInfo.flags |= VK_FENCE_CREATE_SIGNALED_BIT;
		}

		SATURN_CALL(vkCreateFence(device_, &createInfo, nullptr, &handle_));
	}

	Fence::~Fence() noexcept
	{
		vkDestroyFence(device_, handle_, nullptr);
	}

	void Fence::wait() const noexcept
	{
		vkWaitForFences(device_,
		                1,
		                &handle_,
		                VK_TRUE,
		                std::numeric_limits<uint64_t>::max());
	}

	void Fence::reset() noexcept
	{
		vkResetFences(device_, 1, &handle_);
	}

	///////////////////
	//// Semaphore ////
	///////////////////

	Semaphore::Semaphore(rn<Device> device) noexcept
	    : device_(std::move(device))
	{
		VkSemaphoreCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		SATURN_CALL(vkCreateSemaphore(device_, &createInfo, nullptr, &handle_));
	}

	Semaphore::~Semaphore() noexcept
	{
		vkDestroySemaphore(device_, handle_, nullptr);
	}
} // namespace sat
