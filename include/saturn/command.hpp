#ifndef SATURN_COMMAND_HPP
#define SATURN_COMMAND_HPP

#include <vulkan/vulkan.h>

#include "core.hpp"

namespace sat
{
	class CommandBuffer;
	class CommandPool;
	class Device;

	//////////////////////////////
	//// Command Pool Builder ////
	//////////////////////////////

	class SATURN_API CommandPoolBuilder
	    : public Builder<CommandPoolBuilder, CommandPool>
	{
	public:
		explicit CommandPoolBuilder(rn<Device> device) noexcept;

		CommandPoolBuilder& queueFamilyIndex(uint32_t index) noexcept;
		CommandPoolBuilder& reset() noexcept;

	private:
		friend class CommandPool;

		rn<Device> device_;
		uint32_t index_;
		bool reset_ = false;
	};

	//////////////////////
	//// Command Pool ////
	//////////////////////

	class SATURN_API CommandPool : public Container<VkCommandPool>
	{
	public:
		~CommandPool() noexcept;

		CommandPool(const CommandPool&)            = delete;
		CommandPool& operator=(const CommandPool&) = delete;

		CommandBuffer allocate() const;
		void free(CommandBuffer& buffer) const noexcept;

	private:
		friend class CommandBuffer;
		friend class Builder<CommandPoolBuilder, CommandPool>;

		explicit CommandPool(const CommandPoolBuilder& builder);

		rn<Device> device_;
	};

	////////////////////////
	//// Command Buffer ////
	////////////////////////

	class SATURN_API CommandBuffer : public Container<VkCommandBuffer>
	{
	public:
		CommandBuffer() noexcept = default;

		CommandBuffer(const CommandBuffer&)            = delete;
		CommandBuffer& operator=(const CommandBuffer&) = delete;
		CommandBuffer(CommandBuffer&& other) noexcept;
		CommandBuffer& operator=(CommandBuffer&& other) noexcept;

		void record(bool oneTime = false);
		void stop();
		void reset();

		void begin(VkRenderPass renderPass,
		           VkFramebuffer framebuffer,
		           const VkExtent2D& extent,
		           const VkOffset2D& offset = {0, 0}) noexcept;
		void end() noexcept;

		void viewport(const VkExtent2D& extent,
		              const VkOffset2D& offset = {0, 0},
		              float min                = 0,
		              float max                = 1) noexcept;
		void viewport(const VkViewport& viewport) noexcept;

		void scissor(const VkExtent2D& extent,
		             const VkOffset2D& offset = {0, 0}) noexcept;
		void scissor(const VkRect2D& scissor) noexcept;

		void bindPipeline(VkPipeline pipeline) noexcept;

		void bindVertexBuffer(VkBuffer buffer,
		                      VkDeviceSize offset = 0) noexcept;

		void draw(uint32_t count, uint32_t index = 0) noexcept;

		void copy(VkBuffer dst,
		          VkBuffer src,
		          VkDeviceSize size,
		          VkDeviceSize srcOffset = 0,
		          VkDeviceSize dstOffset = 0) noexcept;
		void copy(VkBuffer dst,
		          VkBuffer src,
		          const VkBufferCopy& copy) noexcept;

	private:
		friend class CommandPool;

		explicit CommandBuffer(VkCommandBuffer handle);
	};
} // namespace sat

#endif
