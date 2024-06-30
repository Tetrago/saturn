#ifndef SATURN_PHYSICAL_DEVICE_HPP
#define SATURN_PHYSICAL_DEVICE_HPP

#include <vulkan/vulkan.h>

#include <functional>
#include <optional>
#include <vector>

#include "core.hpp"

namespace sat
{
	class Instance;

	/////////////////////////
	//// Physical Device ////
	/////////////////////////

	struct PhysicalDevice
	{
		VkPhysicalDevice handle;
		VkPhysicalDeviceProperties properties;
		VkPhysicalDeviceFeatures features;
		std::vector<VkQueueFamilyProperties> queueFamilies;
	};

	namespace device
	{
		SATURN_API PhysicalDevice query(VkPhysicalDevice handle) noexcept;

		SATURN_API std::optional<uint32_t> find_graphics_queue(
		    const PhysicalDevice& device) noexcept;

		SATURN_API std::optional<uint32_t> find_surface_queue(
		    VkSurfaceKHR surface, const PhysicalDevice& device) noexcept;
	} // namespace device

	//////////////////////////////////
	//// Physical Device Selector ////
	//////////////////////////////////

	/**
	 * Filters or biases specific device properties when selecting an
	 * appropriate physical device in a \ref PhysicalDeviceSelector.
	 *
	 * \return Score when biasing device, empty when filtering out device.
	 */
	using PhysicalDeviceCriterion =
	    std::function<std::optional<int>(const PhysicalDevice&)>;

	class SATURN_API PhysicalDeviceSelector
	{
	public:
		PhysicalDeviceSelector(rn<Instance> instance) noexcept;

		PhysicalDeviceSelector& require(
		    const PhysicalDeviceCriterion& criterion) noexcept;

		PhysicalDeviceSelector& prefer(
		    const PhysicalDeviceCriterion& criterion) noexcept;

		std::optional<PhysicalDevice> select() const noexcept;

	private:
		rn<Instance> instance_;
		std::vector<std::pair<PhysicalDevice, int>> devices_;
	};

	///////////////////////////////////
	//// Physical Device Criterion ////
	///////////////////////////////////

	namespace criterion
	{
		/**
		 * \brief Criterion that biases or filters a specific type of \ref
		 * VkPhysicalDeviceType.
		 *
		 * \return Criterion to use in \ref PhysicalDeviceSelector::filter(const
		 * PhysicalDeviceCriterion&).
		 */
		SATURN_API PhysicalDeviceCriterion device(VkPhysicalDeviceType type,
		                                          int bias = 1000) noexcept;

		/**
		 * \brief Automatically weighs graphics device types.
		 */
		SATURN_API PhysicalDeviceCriterion weigh_devices() noexcept;

		SATURN_API PhysicalDeviceCriterion
		graphics_queue_family(int bias = 1000) noexcept;

		SATURN_API PhysicalDeviceCriterion
		surface_support(VkSurfaceKHR surface, int bias = 1000) noexcept;
	} // namespace criterion
} // namespace sat

#endif
