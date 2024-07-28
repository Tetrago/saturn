#ifndef SATURN_COMMAND_HPP
#define SATURN_COMMAND_HPP

#include <vulkan/vulkan.h>

#include <mutex>
#include <semaphore>
#include <stack>

#include "core.hpp"

namespace sat
{
	class CommandBuffer;
	class CommandDispatcher;
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

		void bindIndexBuffer(VkBuffer buffer,
		                     VkDeviceSize offset = 0,
		                     VkIndexType type = VK_INDEX_TYPE_UINT32) noexcept;

		void draw(uint32_t count, uint32_t index = 0) noexcept;

		void drawIndexed(uint32_t count, uint32_t index = 0) noexcept;

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

	////////////////////////////////////
	//// Command Dispatcher Builder ////
	////////////////////////////////////

	class SATURN_API CommandDispatcherBuilder
	    : public Builder<CommandDispatcherBuilder, CommandDispatcher>
	{
	public:
		CommandDispatcherBuilder(rn<CommandPool> pool) noexcept;

		CommandDispatcherBuilder& count(unsigned count) noexcept;

	private:
		friend class CommandDispatcher;

		rn<CommandPool> pool_;
		unsigned count_ = 1;
	};

	////////////////////////////
	//// Command Dispatcher ////
	////////////////////////////

	class SATURN_API CommandDispatcher
	{
	public:
		class SATURN_API Lease
		{
		public:
			~Lease() noexcept;

			Lease(const Lease&)            = delete;
			Lease& operator=(const Lease&) = delete;

			CommandBuffer* operator->() noexcept { return &buffer_; }

			const CommandBuffer* operator->() const noexcept
			{
				return &buffer_;
			}

			const CommandBuffer& operator*() const noexcept { return buffer_; }

		private:
			friend class CommandDispatcher;

			Lease(CommandDispatcher* pDispatcher) noexcept;

			CommandDispatcher* dispatcher_;
			CommandBuffer buffer_;
		};

		~CommandDispatcher() noexcept;

		CommandDispatcher(const CommandDispatcher&)            = delete;
		CommandDispatcher& operator=(const CommandDispatcher&) = delete;

		Lease lease() noexcept;

	private:
		friend class Builder<CommandDispatcherBuilder, CommandDispatcher>;
		friend class Lease;

		explicit CommandDispatcher(
		    const CommandDispatcherBuilder& builder) noexcept;

		CommandBuffer acquire() noexcept;
		void release(CommandBuffer&& buffer) noexcept;

		rn<CommandPool> pool_;
		std::stack<CommandBuffer> buffers_;
		std::binary_semaphore available_;
		std::mutex mutex_;
	};
} // namespace sat

#endif
