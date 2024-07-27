#include "framebuffer.hpp"

#include "device.hpp"
#include "error.hpp"
#include "render_pass.hpp"

namespace sat
{
	/////////////////////////////
	//// Framebuffer Builder ////
	/////////////////////////////

	FramebufferBuilder::FramebufferBuilder(rn<Device> device,
	                                       rn<RenderPass> renderPass) noexcept
	    : device_(std::move(device)), renderPass_(std::move(renderPass))
	{}

	FramebufferBuilder& FramebufferBuilder::extent(
	    const VkExtent2D& extent) noexcept
	{
		extent_ = extent;
		return *this;
	}

	FramebufferBuilder& FramebufferBuilder::add(VkImageView view) noexcept
	{
		views_.push_back(view);
		return *this;
	}

	/////////////////////
	//// Framebuffer ////
	/////////////////////

	Framebuffer::Framebuffer(const FramebufferBuilder& builder)
	    : device_(builder.device_), renderPass_(builder.renderPass_)
	{
		VkFramebufferCreateInfo createInfo{};
		createInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		createInfo.renderPass      = renderPass_;
		createInfo.attachmentCount = builder.views_.size();
		createInfo.pAttachments    = builder.views_.data();
		createInfo.width           = builder.extent_.width;
		createInfo.height          = builder.extent_.height;
		createInfo.layers          = 1;

		SATURN_CALL(
		    vkCreateFramebuffer(device_, &createInfo, nullptr, &handle_));
	}

	Framebuffer::~Framebuffer() noexcept
	{
		vkDestroyFramebuffer(device_, handle_, nullptr);
	}
} // namespace sat
