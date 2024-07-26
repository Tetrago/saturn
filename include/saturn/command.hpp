#ifndef SATURN_COMMAND_HPP
#define SATURN_COMMAND_HPP

#include <vulkan/vulkan.h>

#include "core.hpp"
#include "device.hpp"
#include "instance.hpp"

namespace sat
{
	class CommandBuffer;
	class CommandPool;

	//////////////////////////////
	//// Command Pool Builder ////
	//////////////////////////////

	class SATURN_API CommandPoolBuilder
	{
	public:
		explicit CommandPoolBuilder(rn<Device> device) noexcept;

		CommandPoolBuilder& queueFamilyIndex(uint32_t index) noexcept;
		CommandPoolBuilder& reset() noexcept;

		rn<CommandPool> build() const;

	private:
		friend class CommandPool;

		rn<Device> device_;
		uint32_t index_;
		bool reset_ = false;
	};

	//////////////////////
	//// Command Pool ////
	//////////////////////

	class SATURN_API CommandPool
	{
	public:
		~CommandPool() noexcept;

		CommandPool(const CommandPool&)            = delete;
		CommandPool& operator=(const CommandPool&) = delete;

		VkCommandPool handle() const noexcept { return handle_; }

	private:
		friend class CommandBuffer;
		friend class CommandPoolBuilder;

		explicit CommandPool(const CommandPoolBuilder& builder);

		VkCommandBuffer allocate() const;
		void free(VkCommandBuffer handle) const noexcept;

		Logger& logger() const noexcept { return device_->instance().logger(); }

		rn<Device> device_;
		VkCommandPool handle_;
	};

	////////////////////////
	//// Command Buffer ////
	////////////////////////

	class CommandBuffer
	{
	public:
		explicit CommandBuffer(const rn<CommandPool>& pool);
		~CommandBuffer() noexcept;

		CommandBuffer(const CommandBuffer&)            = delete;
		CommandBuffer& operator=(const CommandBuffer&) = delete;
		CommandBuffer(CommandBuffer&& other) noexcept;
		CommandBuffer& operator=(CommandBuffer&& other) noexcept;

		void begin();
		void end();

		VkCommandBuffer handle() const noexcept { return handle_; }

	private:
		std::weak_ptr<CommandPool> pool_;
		VkCommandBuffer handle_ = VK_NULL_HANDLE;
	};
} // namespace sat

#endif
