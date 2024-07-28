#include "command.hpp"

#include "device.hpp"
#include "error.hpp"
#include "pipeline.hpp"
#include "render_pass.hpp"

namespace sat
{
	///////////////////////////////
	///// Command Pool Builder ////
	///////////////////////////////

	CommandPoolBuilder::CommandPoolBuilder(rn<Device> device) noexcept
	    : device_(std::move(device))
	{}

	CommandPoolBuilder& CommandPoolBuilder::queueFamilyIndex(
	    uint32_t index) noexcept
	{
		index_ = index;
		return *this;
	}

	CommandPoolBuilder& CommandPoolBuilder::reset() noexcept
	{
		reset_ = true;
		return *this;
	}

	//////////////////////
	//// Command Pool ////
	//////////////////////

	CommandPool::CommandPool(const CommandPoolBuilder& builder)
	    : device_(builder.device_)
	{
		VkCommandPoolCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		createInfo.queueFamilyIndex = builder.index_;

		if (builder.reset_)
		{
			createInfo.flags |= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		}

		SATURN_CALL(
		    vkCreateCommandPool(device_, &createInfo, nullptr, &handle_));
	}

	CommandPool::~CommandPool() noexcept
	{
		vkDestroyCommandPool(device_, handle_, nullptr);
	}

	CommandBuffer CommandPool::allocate() const
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = handle_;
		allocInfo.level       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer handle;
		SATURN_CALL(vkAllocateCommandBuffers(device_, &allocInfo, &handle));

		return CommandBuffer(handle);
	}

	void CommandPool::free(CommandBuffer& buffer) const noexcept
	{
		if (buffer.handle_ != VK_NULL_HANDLE)
		{
			vkFreeCommandBuffers(device_, handle_, 1, &buffer.handle_);
			buffer = CommandBuffer();
		}
	}

	////////////////////////
	//// Command Buffer ////
	////////////////////////

	CommandBuffer::CommandBuffer(VkCommandBuffer handle)
	{
		handle_ = handle;
	}

	CommandBuffer::CommandBuffer(CommandBuffer&& other) noexcept
	    : Container(other.handle_)
	{}

	CommandBuffer& CommandBuffer::operator=(CommandBuffer&& other) noexcept
	{
		handle_ = other.handle_;

		other.handle_ = VK_NULL_HANDLE;

		return *this;
	}

	void CommandBuffer::record(bool oneTime)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (oneTime)
		{
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		}

		SATURN_CALL(vkBeginCommandBuffer(handle_, &beginInfo));
	}

	void CommandBuffer::stop()
	{
		SATURN_CALL(vkEndCommandBuffer(handle_));
	}

	void CommandBuffer::reset()
	{
		SATURN_CALL(vkResetCommandBuffer(handle_, 0));
	}

	void CommandBuffer::begin(VkRenderPass renderPass,
	                          VkFramebuffer framebuffer,
	                          const VkExtent2D& extent,
	                          const VkOffset2D& offset) noexcept
	{
		VkRenderPassBeginInfo beginInfo{};
		beginInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		beginInfo.renderPass        = renderPass;
		beginInfo.framebuffer       = framebuffer;
		beginInfo.renderArea.offset = offset;
		beginInfo.renderArea.extent = extent;

		VkClearValue clearValue   = {{0, 0, 0, 1}};
		beginInfo.clearValueCount = 1;
		beginInfo.pClearValues    = &clearValue;

		vkCmdBeginRenderPass(handle_, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
	}

	void CommandBuffer::end() noexcept
	{
		vkCmdEndRenderPass(handle_);
	}

	void CommandBuffer::viewport(const VkExtent2D& extent,
	                             const VkOffset2D& offset,
	                             float min,
	                             float max) noexcept
	{
		VkViewport viewport{};
		viewport.x        = offset.x;
		viewport.y        = offset.y;
		viewport.width    = extent.width;
		viewport.height   = extent.height;
		viewport.minDepth = min;
		viewport.maxDepth = max;

		this->viewport(viewport);
	}

	void CommandBuffer::viewport(const VkViewport& viewport) noexcept
	{
		vkCmdSetViewport(handle_, 0, 1, &viewport);
	}

	void CommandBuffer::scissor(const VkExtent2D& extent,
	                            const VkOffset2D& offset) noexcept
	{
		VkRect2D scissor{};
		scissor.offset = offset;
		scissor.extent = extent;

		this->scissor(scissor);
	}

	void CommandBuffer::scissor(const VkRect2D& scissor) noexcept
	{
		vkCmdSetScissor(handle_, 0, 1, &scissor);
	}

	void CommandBuffer::bindPipeline(VkPipeline pipeline) noexcept
	{
		vkCmdBindPipeline(handle_, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	}

	void CommandBuffer::bindVertexBuffer(VkBuffer buffer,
	                                     VkDeviceSize offset) noexcept
	{
		vkCmdBindVertexBuffers(handle_, 0, 1, &buffer, &offset);
	}

	void CommandBuffer::draw(uint32_t count, uint32_t index) noexcept
	{
		vkCmdDraw(handle_, count, 1, index, 0);
	}

	void CommandBuffer::copy(VkBuffer dst,
	                         VkBuffer src,
	                         VkDeviceSize size,
	                         VkDeviceSize srcOffset,
	                         VkDeviceSize dstOffset) noexcept
	{
		VkBufferCopy copy{};
		copy.size      = size;
		copy.srcOffset = srcOffset;
		copy.dstOffset = dstOffset;

		this->copy(dst, src, copy);
	}

	void CommandBuffer::copy(VkBuffer dst,
	                         VkBuffer src,
	                         const VkBufferCopy& copy) noexcept
	{
		vkCmdCopyBuffer(handle_, src, dst, 1, &copy);
	}
} // namespace sat
