#ifndef SATURN_FRAMEBUFFER_HPP
#define SATURN_FRAMEBUFFER_HPP

#include <vulkan/vulkan.h>

#include <vector>

#include "core.hpp"

namespace sat
{
	class Device;
	class Framebuffer;
	class RenderPass;

	/////////////////////////////
	//// Framebuffer Builder ////
	/////////////////////////////

	class SATURN_API FramebufferBuilder
	    : public Builder<FramebufferBuilder, Framebuffer>
	{
	public:
		FramebufferBuilder(rn<Device> device,
		                   rn<RenderPass> renderPass) noexcept;

		FramebufferBuilder& extent(const VkExtent2D& extent) noexcept;
		FramebufferBuilder& add(VkImageView view) noexcept;

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

	class SATURN_API Framebuffer : public Container<VkFramebuffer>
	{
	public:
		~Framebuffer() noexcept;

		Framebuffer(const Framebuffer&)            = delete;
		Framebuffer& operator=(const Framebuffer&) = delete;

	private:
		friend class Builder<FramebufferBuilder, Framebuffer>;

		explicit Framebuffer(const FramebufferBuilder& builder);

		rn<Device> device_;
		rn<RenderPass> renderPass_;
	};
} // namespace sat

#endif
