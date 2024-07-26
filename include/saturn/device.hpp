#ifndef SATURN_DEVICE_HPP
#define SATURN_DEVICE_HPP

#include <vulkan/vulkan.h>

#include <unordered_map>
#include <vector>

#include "core.hpp"
#include "instance.hpp"
#include "physical_device.hpp"

namespace sat
{
	class Device;

	////////////////////////
	//// Device Builder ////
	////////////////////////

	class SATURN_API DeviceBuilder
	{
	public:
		DeviceBuilder(rn<Instance> instance, PhysicalDevice device) noexcept;

		/**
		 * \brief Adds a new queue to the specified index with the given
		 * priority.
		 */
		DeviceBuilder& addQueue(uint32_t index, float priority = 1) noexcept;

		DeviceBuilder& addExtension(const char* pExtensionName) noexcept;

		rn<Device> build() const;

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

	class Logger;

	class SATURN_API Device
	{
	public:
		~Device() noexcept;

		Device(const Device&)            = delete;
		Device& operator=(const Device&) = delete;

		VkQueue queue(uint32_t family, uint32_t index = 0) const noexcept;

		VkDevice handle() const noexcept { return handle_; }

		Instance& instance() const noexcept { return *instance_; }

		const PhysicalDevice& device() const noexcept { return device_; }

	private:
		friend class DeviceBuilder;

		explicit Device(const DeviceBuilder& builder);

		Logger& logger() const noexcept { return instance_->logger(); }

		VkDevice handle_;
		rn<Instance> instance_;
		PhysicalDevice device_;
	};
} // namespace sat

#endif
