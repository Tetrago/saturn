#include "device.hpp"

#include "instance.hpp"
#include "local.hpp"

namespace sat
{
	////////////////////////
	//// Device Builder ////
	////////////////////////

	DeviceBuilder::DeviceBuilder(rn<Instance> instance,
	                             PhysicalDevice device) noexcept
	    : instance_(std::move(instance)), device_(device)
	{}

	DeviceBuilder& DeviceBuilder::addQueue(uint32_t index,
	                                       float priority) noexcept
	{
		queues_[index].push_back(priority);
		return *this;
	}

	rn<Device> DeviceBuilder::build() const
	{
		return rn<Device>(new Device(*this));
	}

	////////////////
	//// Device ////
	////////////////

	Device::Device(const DeviceBuilder& builder)
	    : instance_(builder.instance_), device_(builder.device_)
	{
		std::vector<VkDeviceQueueCreateInfo> queueInfos;

		for (const auto& [index, priorities] : builder.queues_)
		{
			VkDeviceQueueCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			createInfo.queueFamilyIndex = index;
			createInfo.queueCount       = priorities.size();
			createInfo.pQueuePriorities = priorities.data();

			queueInfos.push_back(createInfo);
		}

		VkDeviceCreateInfo createInfo{};
		createInfo.sType                = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = queueInfos.size();
		createInfo.pQueueCreateInfos    = queueInfos.data();
		createInfo.pEnabledFeatures     = &device_.features;

		VK_CALL(vkCreateDevice(device_.handle, &createInfo, nullptr, &handle_),
		        instance_->logger(),
		        "Failed to create device");

		instance_->logger().log(LogLevel::Trace,
		                        "Created device with {} queues",
		                        builder.queues_.size());
	}

	Device::~Device() noexcept
	{
		vkDestroyDevice(handle_, nullptr);
	}

	VkQueue Device::queue(uint32_t family, uint32_t index) const noexcept
	{
		VkQueue handle;
		vkGetDeviceQueue(handle_, family, index, &handle);

		instance_->logger().log(LogLevel::Trace,
		                        "Retrieving queue family {} index {}",
		                        family,
		                        index);

		return handle;
	}
} // namespace sat
