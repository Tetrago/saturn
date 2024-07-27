#include "render_pass.hpp"

#include "device.hpp"
#include "error.hpp"

namespace sat
{
	/////////////////////////////
	//// Render Pass Builder ////
	/////////////////////////////

	RenderPassBuilder::RenderPassBuilder(rn<Device> device) noexcept
	    : device_(std::move(device))
	{}

	RenderPassBuilder& RenderPassBuilder::createColorAttachment(
	    VkFormat format) noexcept
	{
		VkAttachmentDescription attachment{};
		attachment.format         = format;
		attachment.samples        = VK_SAMPLE_COUNT_1_BIT;
		attachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
		attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
		attachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		attachments_.push_back(attachment);

		return *this;
	}

	RenderPassBuilder& RenderPassBuilder::begin(
	    VkPipelineBindPoint bind) noexcept
	{
		VkSubpassDescription desc{};
		desc.pipelineBindPoint = bind;

		subpasses_.push_back(desc);

		return *this;
	}

	RenderPassBuilder& RenderPassBuilder::end() noexcept
	{
		std::vector<VkAttachmentReference>& references = store_.emplace_back();
		references.reserve(colorAttachments_.size());

		VkSubpassDescription& desc = subpasses_.back();

		desc.colorAttachmentCount = colorAttachments_.size();
		desc.pColorAttachments    = references.data() + references.size();
		references.insert(references.end(),
		                  colorAttachments_.begin(),
		                  colorAttachments_.end());

		colorAttachments_.clear();

		return *this;
	}

	RenderPassBuilder& RenderPassBuilder::addColorAttachment(
	    uint32_t index, VkImageLayout layout) noexcept
	{
		VkAttachmentReference reference{};
		reference.attachment = index;
		reference.layout     = layout;

		colorAttachments_.push_back(reference);

		return *this;
	}

	/////////////////////
	//// Render Pass ////
	/////////////////////

	RenderPass::RenderPass(const RenderPassBuilder& builder)
	    : device_(builder.device_)
	{
		VkRenderPassCreateInfo createInfo{};
		createInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		createInfo.subpassCount    = builder.subpasses_.size();
		createInfo.pSubpasses      = builder.subpasses_.data();
		createInfo.attachmentCount = builder.attachments_.size();
		createInfo.pAttachments    = builder.attachments_.data();

		SATURN_CALL(
		    vkCreateRenderPass(device_, &createInfo, nullptr, &handle_));
	}

	RenderPass::~RenderPass() noexcept
	{
		vkDestroyRenderPass(device_, handle_, nullptr);
	}
} // namespace sat
