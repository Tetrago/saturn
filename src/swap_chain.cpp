#include "swap_chain.hpp"

#include <algorithm>
#include <limits>

#include "local.hpp"
#include "physical_device.hpp"

namespace sat
{
	////////////////////////////
	//// Swap Chain Details ////
	////////////////////////////

	SwapChainDetails swap_chain::query(const PhysicalDevice& device,
	                                   VkSurfaceKHR surface) noexcept
	{
		SwapChainDetails details{};

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
		    device.handle, surface, &details.capabilities);

		// Get surface formats
		uint32_t count;
		vkGetPhysicalDeviceSurfaceFormatsKHR(
		    device.handle, surface, &count, nullptr);

		if (count > 0)
		{
			details.formats.resize(count);
			vkGetPhysicalDeviceSurfaceFormatsKHR(
			    device.handle, surface, &count, details.formats.data());
		}

		// Get present modes
		vkGetPhysicalDeviceSurfacePresentModesKHR(
		    device.handle, surface, &count, nullptr);

		if (count > 0)
		{
			details.presentModes.resize(count);
			vkGetPhysicalDeviceSurfacePresentModesKHR(
			    device.handle, surface, &count, details.presentModes.data());
		}

		return details;
	}

	////////////////////////////
	//// Swap Chain Builder ////
	////////////////////////////

	SwapChainBuilder::SwapChainBuilder(rn<Device> device,
	                                   VkSurfaceKHR surface) noexcept
	    : device_(std::move(device)), surface_(surface)
	{
		details_       = swap_chain::query(device_->device(), surface);
		surfaceFormat_ = details_.formats[0];
		presentMode_   = details_.presentModes[0];
		imageCount_    = details_.capabilities.minImageCount + 1;
		usage_         = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		// Clamp image count if there exists a limit
		if (details_.capabilities.maxImageCount > 0 &&
		    imageCount_ > details_.capabilities.maxImageCount)
		{
			imageCount_ = details_.capabilities.maxImageCount;
		}
	}

	SwapChainBuilder& SwapChainBuilder::selectSurfaceFormat(
	    VkFormat format, VkColorSpaceKHR colorSpace) noexcept
	{
		for (const VkSurfaceFormatKHR& surfaceFormat : details_.formats)
		{
			if (surfaceFormat.format == format &&
			    surfaceFormat.colorSpace == colorSpace)
			{
				surfaceFormat_ = surfaceFormat;
				break;
			}
		}

		return *this;
	}

	SwapChainBuilder& SwapChainBuilder::selectPresentMode(
	    VkPresentModeKHR presentMode) noexcept
	{
		if (std::find(details_.presentModes.begin(),
		              details_.presentModes.end(),
		              presentMode) != details_.presentModes.end())
		{
			presentMode_ = presentMode;
		}

		return *this;
	}

	SwapChainBuilder& SwapChainBuilder::extent(int width, int height) noexcept
	{
		const VkSurfaceCapabilitiesKHR& caps = details_.capabilities;

		if (caps.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			extent_ = details_.capabilities.currentExtent;
		}
		else
		{
			extent_.width = std::clamp<uint32_t>(
			    width, caps.minImageExtent.width, caps.maxImageExtent.width);
			extent_.height = std::clamp<uint32_t>(
			    height, caps.minImageExtent.height, caps.maxImageExtent.height);
		}

		return *this;
	}

	SwapChainBuilder& SwapChainBuilder::imageCount(uint32_t count) noexcept
	{
		imageCount_ = count;

		// Clamp image count if there exists a limit
		if (details_.capabilities.maxImageCount > 0 &&
		    imageCount_ > details_.capabilities.maxImageCount)
		{
			imageCount_ = details_.capabilities.maxImageCount;
		}

		return *this;
	}

	SwapChainBuilder& SwapChainBuilder::usage(VkImageUsageFlags usage) noexcept
	{
		usage_ = usage;
		return *this;
	}

	SwapChainBuilder& SwapChainBuilder::share(
	    std::span<uint32_t const> queueFamilies) noexcept
	{
		queueFamilies_.assign(queueFamilies.begin(), queueFamilies.end());
		return *this;
	}

	rn<SwapChain> SwapChainBuilder::build() const
	{
		return rn<SwapChain>(new SwapChain(*this));
	}

	////////////////////
	//// Swap Chain ////
	////////////////////

	SwapChain::SwapChain(const SwapChainBuilder& builder)
	    : device_(builder.device_),
	      format_(builder.surfaceFormat_.format),
	      extent_(builder.extent_)
	{
		////////////////////
		//// Swap Chain ////
		////////////////////

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType         = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface       = builder.surface_;
		createInfo.minImageCount = builder.imageCount_;
		createInfo.imageFormat   = builder.surfaceFormat_.format;
		createInfo.imageColorSpace  = builder.surfaceFormat_.colorSpace;
		createInfo.imageExtent      = extent_;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage       = builder.usage_;

		// Handle queue family sharing
		if (builder.queueFamilies_.empty())
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}
		else
		{
			createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = builder.queueFamilies_.size();
			createInfo.pQueueFamilyIndices   = builder.queueFamilies_.data();
		}

		createInfo.preTransform =
		    builder.details_.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode    = builder.presentMode_;
		createInfo.clipped        = VK_TRUE;
		createInfo.oldSwapchain   = VK_NULL_HANDLE;

		VK_CALL(vkCreateSwapchainKHR(
		            device_->handle(), &createInfo, nullptr, &handle_),
		        "Failed to create swap chain");

		////////////////
		//// Images ////
		////////////////

		uint32_t count;
		vkGetSwapchainImagesKHR(device_->handle(), handle_, &count, nullptr);
		images_.resize(count);
		vkGetSwapchainImagesKHR(
		    device_->handle(), handle_, &count, images_.data());

		S_TRACE("Created {}x{} swap chain " S_PTR " with {} images",
		        builder.extent_.width,
		        builder.extent_.height,
		        S_THIS,
		        count);

		/////////////////////
		//// Image Views ////
		/////////////////////

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType        = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.viewType     = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format       = format_;
		viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.layerCount = 1;

		// Make space for new image views
		views_.resize(count);

		while (count--)
		{
			viewInfo.image = images_[count];

			if (vkCreateImageView(
			        device_->handle(), &viewInfo, nullptr, &views_[count]) !=
			    VK_SUCCESS)
			{
				goto abort;
			}
		}

		S_TRACE("Created {} swap chain image views", views_.size());

		return;

		///////////////
		//// Abort ////
		///////////////

	abort:

		S_ERROR(
		    "Aborting during swap chain initialization due to failure to "
		    "create image views");

		while (++count < views_.size())
		{
			vkDestroyImageView(device_->handle(), views_[count], nullptr);
		}

		S_TRACE("Destroyed swap chain image views while aborting");

		vkDestroySwapchainKHR(device_->handle(), handle_, nullptr);

		S_TRACE("Destroyed swap chain " S_PTR
		        " while aborting failed image view creation",
		        S_THIS);

		throw std::runtime_error("Failed to create swap chain image views");
	}

	SwapChain::~SwapChain() noexcept
	{
		for (VkImageView view : views_)
		{
			vkDestroyImageView(device_->handle(), view, nullptr);
		}

		S_TRACE("Destroyed {} swap chain image views", views_.size());

		vkDestroySwapchainKHR(device_->handle(), handle_, nullptr);

		S_TRACE("Destroyed swap chain " S_PTR, S_THIS);
	}
} // namespace sat
