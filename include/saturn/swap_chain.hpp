#ifndef SATURN_SWAP_CHAIN_HPP
#define SATURN_SWAP_CHAIN_HPP

#include <vulkan/vulkan.h>

#include <span>
#include <vector>

#include "core.hpp"

namespace sat
{
	////////////////////////////
	//// Swap Chain Details ////
	////////////////////////////

	struct PhysicalDevice;

	struct SwapChainDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	namespace swap_chain
	{
		SATURN_API SwapChainDetails query(const PhysicalDevice& device,
		                                  VkSurfaceKHR surface) noexcept;
	}

	////////////////////////////
	//// Swap Chain Builder ////
	////////////////////////////

	class Device;
	class SwapChain;

	class SATURN_API SwapChainBuilder
	{
	public:
		SwapChainBuilder(rn<Device> device, VkSurfaceKHR surface) noexcept;

		SwapChainBuilder& selectSurfaceFormat(
		    VkFormat format, VkColorSpaceKHR colorSpace) noexcept;

		SwapChainBuilder& selectPresentMode(
		    VkPresentModeKHR presentMode) noexcept;

		/**
		 * \brief Sets the swap chain's extent in pixels when necessary.
		 */
		SwapChainBuilder& extent(int width, int height) noexcept;

		SwapChainBuilder& imageCount(uint32_t count) noexcept;

		/**
		 * \brief Shares swap chain between queue families
		 */
		SwapChainBuilder& usage(VkImageUsageFlags usage) noexcept;

		SwapChainBuilder& share(
		    std::span<uint32_t const> queueFamilies) noexcept;

		rn<SwapChain> build();

	private:
		friend class SwapChain;

		rn<Device> device_;
		VkSurfaceKHR surface_;
		SwapChainDetails details_;
		VkSurfaceFormatKHR surfaceFormat_;
		VkPresentModeKHR presentMode_;
		VkExtent2D extent_{};
		uint32_t imageCount_;
		VkImageUsageFlags usage_;
		std::vector<uint32_t> queueFamilies_{};
	};

	////////////////////
	//// Swap Chain ////
	////////////////////

	class SATURN_API SwapChain
	{
	public:
		~SwapChain() noexcept;

		SwapChain(const SwapChain&)            = delete;
		SwapChain& operator=(const SwapChain&) = delete;

		VkSwapchainKHR handle() const noexcept { return handle_; }

	private:
		friend class SwapChainBuilder;

		SwapChain(const SwapChainBuilder& builder);

		sat::rn<Device> device_;
		VkSwapchainKHR handle_;
		std::vector<VkImage> images_;
		VkFormat format_;
		VkExtent2D extent_;
		std::vector<VkImageView> views_;
	};
} // namespace sat

#endif
