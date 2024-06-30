#include "physical_device.hpp"

#include <limits>
#include <sstream>

#include "instance.hpp"

namespace sat
{
	/////////////////////////
	//// Physical Device ////
	/////////////////////////

	namespace device
	{
		PhysicalDevice query(VkPhysicalDevice handle) noexcept
		{
			PhysicalDevice device{};
			device.handle = handle;

			vkGetPhysicalDeviceProperties(handle, &device.properties);
			vkGetPhysicalDeviceFeatures(handle, &device.features);

			// Get queue family properties
			uint32_t count;
			vkGetPhysicalDeviceQueueFamilyProperties(handle, &count, nullptr);
			device.queueFamilies.resize(count);
			vkGetPhysicalDeviceQueueFamilyProperties(
			    handle, &count, device.queueFamilies.data());

			return device;
		}

		std::optional<uint32_t> find_graphics_queue(
		    const PhysicalDevice& device) noexcept
		{
			for (uint32_t index = 0; index < device.queueFamilies.size();
			     ++index)
			{
				if (device.queueFamilies[index].queueFlags &
				    VK_QUEUE_GRAPHICS_BIT)
				{
					return index;
				}
			}

			return std::nullopt;
		}

		std::optional<uint32_t> find_surface_queue(
		    VkSurfaceKHR surface, const PhysicalDevice& device) noexcept
		{
			VkBool32 result;

			for (uint32_t index = 0; index < device.queueFamilies.size();
			     ++index)
			{
				vkGetPhysicalDeviceSurfaceSupportKHR(
				    device.handle, index, surface, &result);

				if (result == VK_TRUE)
				{
					return index;
				}
			}

			return std::nullopt;
		}
	} // namespace device

	//////////////////////////////////
	//// Physical Device Selector ////
	//////////////////////////////////

	PhysicalDeviceSelector::PhysicalDeviceSelector(
	    rn<Instance> instance) noexcept
	    : instance_(std::move(instance))
	{
		std::vector<PhysicalDevice> devices = instance_->devices();

		if (devices.empty())
		{
			// If no devices were found

			instance_->logger().log(LogLevel::Error,
			                        "Found no available physical devices");
		}
		else
		{
			// If devices were found

			std::ostringstream oss;
			oss << "Found physical devices:";

			devices_.resize(devices.size());

			for (int i = 0; i < devices.size(); ++i)
			{
				oss << "\n - " << devices[i].properties.deviceName;
				devices_[i] = std::make_pair(devices[i], 0);
			}

			instance_->logger().log(LogLevel::Info, oss.str());
		}
	}

	PhysicalDeviceSelector& PhysicalDeviceSelector::require(
	    const PhysicalDeviceCriterion& criterion) noexcept
	{
		auto it = devices_.begin();
		while (it != devices_.end())
		{
			if (std::optional<int> result = criterion(it->first))
			{
				// If device passes condition

				it->second += result.value();
				++it;
			}
			else
			{
				// If device fails condition and it is required

				instance_->logger().log(
				    LogLevel::Trace,
				    "Physical device `{}` failed to meet criterion",
				    it->first.properties.deviceName);

				it = devices_.erase(it);
			}
		}

		return *this;
	}

	PhysicalDeviceSelector& PhysicalDeviceSelector::prefer(
	    const PhysicalDeviceCriterion& criterion) noexcept
	{
		for (auto& [device, score] : devices_)
		{
			if (std::optional<int> result = criterion(device))
			{
				score += result.value();
			}
		}

		return *this;
	}

	std::optional<PhysicalDevice> PhysicalDeviceSelector::select()
	    const noexcept
	{
		if (devices_.empty())
		{
			instance_->logger().log(LogLevel::Warn,
			                        "Failed to find suitable physical device");
			return std::nullopt;
		}

		// Don't need to handle because the list is guarenteed not to be empty
		int highscore = std::numeric_limits<int>::lowest();
		const PhysicalDevice* pDevice;

		for (const auto& [device, score] : devices_)
		{
			if (score > highscore)
			{
				highscore = score;
				pDevice   = &device;
			}
		}

		instance_->logger().log(LogLevel::Info,
		                        "Selected device: {}",
		                        pDevice->properties.deviceName);

		return *pDevice;
	}

	///////////////////////////////////
	//// Physical Device Criterion ////
	///////////////////////////////////

	namespace criterion
	{
		PhysicalDeviceCriterion device(VkPhysicalDeviceType type,
		                               int bias) noexcept
		{
			return [=](const PhysicalDevice& device) -> std::optional<int> {
				return device.properties.deviceType == type
				           ? std::optional(bias)
				           : std::nullopt;
			};
		}

		PhysicalDeviceCriterion weigh_devices() noexcept
		{
			return [=](const PhysicalDevice& device) -> std::optional<int> {
				switch (device.properties.deviceType)
				{
				default:
				case VK_PHYSICAL_DEVICE_TYPE_CPU:            return 0;
				case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: return 500;
				case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:   return 1000;
				}
			};
		}

		PhysicalDeviceCriterion graphics_queue_family(int bias) noexcept
		{
			return [=](const PhysicalDevice& device) -> std::optional<int> {
				return device::find_graphics_queue(device).has_value()
				           ? std::optional(bias)
				           : std::nullopt;
			};
		}

		PhysicalDeviceCriterion surface_support(VkSurfaceKHR surface,
		                                        int bias) noexcept
		{
			return [=](const PhysicalDevice& device) -> std::optional<int> {
				return device::find_surface_queue(surface, device).has_value()
				           ? std::optional(bias)
				           : std::nullopt;
			};
		}
	} // namespace criterion
} // namespace sat
