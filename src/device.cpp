#include "device.hpp"

#include <cstring>
#include <ranges>

#include "error.hpp"
#include "instance.hpp"

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

	DeviceBuilder& DeviceBuilder::addExtension(
	    const char* pExtensionName) noexcept
	{
		extensions_.push_back(pExtensionName);
		return *this;
	}

	////////////////
	//// Device ////
	////////////////

	Device::Device(const DeviceBuilder& builder)
	    : instance_(builder.instance_), device_(builder.device_)
	{
		for (const char* pExtensionName : builder.extensions_)
		{
			for (const VkExtensionProperties& props : device_.extensions)
			{
				if (strcmp(props.extensionName, pExtensionName) == 0)
				{
					goto next;
				}
			}

			SATURN_THROW(MissingFeatureException, pExtensionName);
		next:;
		}

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
		createInfo.sType                 = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount  = queueInfos.size();
		createInfo.pQueueCreateInfos     = queueInfos.data();
		createInfo.pEnabledFeatures      = &device_.features;
		createInfo.enabledExtensionCount = builder.extensions_.size();
		createInfo.ppEnabledExtensionNames = builder.extensions_.data();

		SATURN_CALL(
		    vkCreateDevice(device_.handle, &createInfo, nullptr, &handle_));

		///////////////////
		//// Allocator ////
		///////////////////

		VmaVulkanFunctions functions{};
		functions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
		functions.vkGetDeviceProcAddr   = &vkGetDeviceProcAddr;

		VmaAllocatorCreateInfo allocatorInfo{};
		allocatorInfo.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
		allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_2;
		allocatorInfo.physicalDevice   = device_.handle;
		allocatorInfo.device           = handle_;
		allocatorInfo.instance         = instance_;
		allocatorInfo.pVulkanFunctions = &functions;

		SATURN_CALL_NO_THROW(vmaCreateAllocator(&allocatorInfo, &allocator_))
		{
			vkDestroyDevice(handle_, nullptr);
		}
	}

	Device::~Device() noexcept
	{
		vmaDestroyAllocator(allocator_);
		vkDestroyDevice(handle_, nullptr);
	}

	VkQueue Device::queue(uint32_t family, uint32_t index) const noexcept
	{
		VkQueue handle;
		vkGetDeviceQueue(handle_, family, index, &handle);

		return handle;
	}

	void Device::waitIdle() const
	{
		SATURN_CALL(vkDeviceWaitIdle(handle_));
	}
} // namespace sat
