#ifndef SATURN_DEVICE_HPP
#define SATURN_DEVICE_HPP

#include <vulkan/vulkan.h>

#include <unordered_map>
#include <vector>

#include "allocator.hpp"
#include "core.hpp"
#include "physical_device.hpp"

namespace sat
{
	class Device;
	class Fence;

	////////////////////////
	//// Device Builder ////
	////////////////////////

	class SATURN_API DeviceBuilder : public Builder<DeviceBuilder, Device>
	{
	public:
		DeviceBuilder(rn<Instance> instance, PhysicalDevice device) noexcept;

		/**
		 * \brief Adds a new queue to the specified index with the given
		 * priority.
		 */
		DeviceBuilder& addQueue(uint32_t index, float priority = 1) noexcept;

		DeviceBuilder& addExtension(const char* pExtensionName) noexcept;

	private:
		friend class Device;

		rn<Instance> instance_;
		PhysicalDevice device_;
		std::unordered_map<uint32_t, std::vector<float>> queues_;
		std::vector<const char*> extensions_;
	};

	////////////////
	//// Device ////
	////////////////

	class SATURN_API Device : public Container<VkDevice>
	{
	public:
		~Device() noexcept;

		Device(const Device&)            = delete;
		Device& operator=(const Device&) = delete;

		VkQueue queue(uint32_t family, uint32_t index = 0) const noexcept;

		void waitIdle() const;

		const PhysicalDevice& device() const noexcept { return device_; }

		VmaAllocator allocator() const noexcept { return allocator_; }

	private:
		friend class Builder<DeviceBuilder, Device>;

		explicit Device(const DeviceBuilder& builder);

		rn<Instance> instance_;
		PhysicalDevice device_;
		VmaAllocator allocator_;
	};
} // namespace sat

#endif
