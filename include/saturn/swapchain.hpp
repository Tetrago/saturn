#ifndef SATURN_SWAP_CHAIN_HPP
#define SATURN_SWAP_CHAIN_HPP

#include <vulkan/vulkan.h>

#include <span>
#include <vector>

#include "core.hpp"
#include "saturn/sync.hpp"

namespace sat
{
	class Device;
	struct PhysicalDevice;
	class Semaphore;

	////////////////////////////
	//// Swap Chain Details ////
	////////////////////////////

	struct SwapchainDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	namespace swap_chain
	{
		SATURN_API SwapchainDetails query(const PhysicalDevice& device,
		                                  VkSurfaceKHR surface) noexcept;
	}

	////////////////////////////
	//// Swap Chain Builder ////
	////////////////////////////

	class Device;
	class Swapchain;

	class SATURN_API SwapchainBuilder
	    : public Builder<SwapchainBuilder, Swapchain>
	{
	public:
		SwapchainBuilder(rn<Device> device, VkSurfaceKHR surface) noexcept;

		SwapchainBuilder& selectSurfaceFormat(
		    VkFormat format, VkColorSpaceKHR colorSpace) noexcept;

		SwapchainBuilder& selectPresentMode(
		    VkPresentModeKHR presentMode) noexcept;

		/**
		 * \brief Sets the swap chain's extent in pixels when necessary.
		 */
		SwapchainBuilder& extent(int width, int height) noexcept;

		SwapchainBuilder& imageCount(uint32_t count) noexcept;

		/**
		 * \brief Shares swap chain between queue families
		 */
		SwapchainBuilder& usage(VkImageUsageFlags usage) noexcept;

		SwapchainBuilder& share(uint32_t queueFamilyIndex) noexcept;

	private:
		friend class Swapchain;

		rn<Device> device_;
		VkSurfaceKHR surface_;
		SwapchainDetails details_;
		VkSurfaceFormatKHR surfaceFormat_;
		VkPresentModeKHR presentMode_;
		VkExtent2D extent_{};
		uint32_t imageCount_;
		VkImageUsageFlags usage_;
		std::vector<uint32_t> queueFamilies_;
	};

	////////////////////
	//// Swap Chain ////
	////////////////////

	class SATURN_API Swapchain : public Container<VkSwapchainKHR>
	{
	public:
		~Swapchain() noexcept;

		Swapchain(const Swapchain&)            = delete;
		Swapchain& operator=(const Swapchain&) = delete;

		/**
		 * \brief Acquires the next image in the swap chain.
		 *
		 * \return True on success, false when swap chain is out of date.
		 */
		bool acquireNextImage(const rn<Semaphore>& semaphore,
		                      uint32_t& imageIndex);

		const std::vector<VkImageView>& views() const noexcept
		{
			return views_;
		}

		VkFormat format() const noexcept { return format_; }

		const VkExtent2D& extent() const noexcept { return extent_; }

	private:
		friend class Builder<SwapchainBuilder, Swapchain>;

		explicit Swapchain(const SwapchainBuilder& builder);

		sat::rn<Device> device_;
		std::vector<VkImage> images_;
		VkFormat format_;
		VkExtent2D extent_;
		std::vector<VkImageView> views_;
	};
} // namespace sat

#endif
