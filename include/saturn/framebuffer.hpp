#ifndef SATURN_FRAMEBUFFER_HPP
#define SATURN_FRAMEBUFFER_HPP

#include <vulkan/vulkan.h>

#include <vector>

#include "core.hpp"
#include "device.hpp"
#include "instance.hpp"

namespace sat
{
	class Device;
	class Framebuffer;
	class RenderPass;

	/////////////////////////////
	//// Framebuffer Builder ////
	/////////////////////////////

	class SATURN_API FramebufferBuilder
	{
	public:
		FramebufferBuilder(rn<Device> device,
		                   rn<RenderPass> renderPass) noexcept;

		FramebufferBuilder& extent(const VkExtent2D& extent) noexcept;
		FramebufferBuilder& add(VkImageView view) noexcept;

		rn<Framebuffer> build() const;

	private:
		friend class Framebuffer;

		rn<Device> device_;
		rn<RenderPass> renderPass_;
		VkExtent2D extent_;
		std::vector<VkImageView> views_;
	};

	/////////////////////
	//// Framebuffer ////
	/////////////////////

	class SATURN_API Framebuffer
	{
	public:
		~Framebuffer() noexcept;

		Framebuffer(const Framebuffer&)            = delete;
		Framebuffer& operator=(const Framebuffer&) = delete;

	private:
		friend class FramebufferBuilder;

		explicit Framebuffer(const FramebufferBuilder& builder);

		Logger& logger() const noexcept { return device_->instance().logger(); }

		rn<Device> device_;
		rn<RenderPass> renderPass_;
		VkFramebuffer handle_;
	};
} // namespace sat

#endif
