#include "framebuffer.hpp"

#include "local.hpp"
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

	rn<Framebuffer> FramebufferBuilder::build() const
	{
		return rn<Framebuffer>(new Framebuffer(*this));
	}

	/////////////////////
	//// Framebuffer ////
	/////////////////////

	Framebuffer::Framebuffer(const FramebufferBuilder& builder)
	    : device_(builder.device_), renderPass_(builder.renderPass_)
	{
		VkFramebufferCreateInfo createInfo{};
		createInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		createInfo.renderPass      = renderPass_->handle();
		createInfo.attachmentCount = builder.views_.size();
		createInfo.pAttachments    = builder.views_.data();
		createInfo.width           = builder.extent_.width;
		createInfo.height          = builder.extent_.height;
		createInfo.layers          = 1;

		VK_CALL(vkCreateFramebuffer(
		            device_->handle(), &createInfo, nullptr, &handle_),
		        "Failed to create framebuffer");

		S_TRACE("Created framebuffer " S_PTR, S_THIS);
	}

	Framebuffer::~Framebuffer() noexcept
	{
		vkDestroyFramebuffer(device_->handle(), handle_, nullptr);

		S_TRACE("Destroyed framebuffer " S_PTR, S_THIS);
	}
} // namespace sat
