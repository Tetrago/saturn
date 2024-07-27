#ifndef SATURN_RENDER_PASS_HPP
#define SATURN_RENDER_PASS_HPP

#include <vulkan/vulkan.h>

#include <list>
#include <vector>

#include "core.hpp"

namespace sat
{
	class Device;
	class RenderPass;

	/////////////////////////////
	//// Render Pass Builder ////
	/////////////////////////////

	class SATURN_API RenderPassBuilder
	    : public Builder<RenderPassBuilder, RenderPass>
	{
	public:
		explicit RenderPassBuilder(rn<Device> device) noexcept;

		RenderPassBuilder& createColorAttachment(VkFormat format) noexcept;

		RenderPassBuilder& begin(VkPipelineBindPoint bind =
		                             VK_PIPELINE_BIND_POINT_GRAPHICS) noexcept;
		RenderPassBuilder& end() noexcept;

		RenderPassBuilder& addColorAttachment(
		    uint32_t index,
		    VkImageLayout layout =
		        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) noexcept;

	private:
		friend class RenderPass;

		rn<Device> device_;
		std::vector<VkSubpassDescription> subpasses_;

		/**
		 * \brief Store of all attachment references used in all subpasses
		 */
		std::list<std::vector<VkAttachmentReference>> store_;

		/**
		 * \brief Vector of all attachments in render pass
		 */
		std::vector<VkAttachmentDescription> attachments_;

		/**
		 * \brief Temporary vector of all color attachments on the current
		 * subpass
		 */
		std::vector<VkAttachmentReference> colorAttachments_;
	};

	/////////////////////
	//// Render Pass ////
	/////////////////////

	class SATURN_API RenderPass : public Container<VkRenderPass>
	{
	public:
		~RenderPass() noexcept;

		RenderPass(const RenderPass&)            = delete;
		RenderPass& operator=(const RenderPass&) = delete;

	private:
		friend class Builder<RenderPassBuilder, RenderPass>;

		explicit RenderPass(const RenderPassBuilder& builder);

		rn<Device> device_;
	};
} // namespace sat

#endif
