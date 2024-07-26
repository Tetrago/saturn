#include "command.hpp"

#include "local.hpp"

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

	rn<CommandPool> CommandPoolBuilder::build() const
	{
		return rn<CommandPool>(new CommandPool(*this));
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

		VK_CALL(vkCreateCommandPool(
		            device_->handle(), &createInfo, nullptr, &handle_),
		        "Failed to create command pool");

		S_TRACE("Created command pool " S_PTR, S_THIS);
	}

	CommandPool::~CommandPool() noexcept
	{
		vkDestroyCommandPool(device_->handle(), handle_, nullptr);

		S_TRACE("Destroyed command pool " S_PTR, S_THIS);
	}

	VkCommandBuffer CommandPool::allocate() const
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = handle_;
		allocInfo.level       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer handle;
		VK_CALL(
		    vkAllocateCommandBuffers(device_->handle(), &allocInfo, &handle),
		    "Failed to allocate command buffer");

		return handle;
	}

	void CommandPool::free(VkCommandBuffer handle) const noexcept
	{
		vkFreeCommandBuffers(device_->handle(), handle_, 1, &handle);
	}

	////////////////////////
	//// Command Buffer ////
	////////////////////////

	CommandBuffer::CommandBuffer(const rn<CommandPool>& pool)
	    : pool_(pool)
	{
		if (auto pool = pool_.lock())
		{
			handle_ = pool->allocate();

			S_TRACE("Created command buffer " S_PTR, S_THIS);
		}
		else
		{
			throw std::runtime_error(
			    "Attempting to create command buffer with deallocated pool");
		}
	}

	CommandBuffer::~CommandBuffer() noexcept
	{
		if (handle_ == VK_NULL_HANDLE) return;

		if (auto pool = pool_.lock())
		{
			pool->free(handle_);

			S_TRACE("Destroyed command buffer " S_PTR, S_THIS);
		}
		else
		{
			S_ERROR(
			    "Attempting to deallocate buffer from destroyed pool " S_PTR,
			    S_THIS);
		}
	}

	CommandBuffer::CommandBuffer(CommandBuffer&& other) noexcept
	    : pool_(std::move(other.pool_)), handle_(other.handle_)
	{}

	CommandBuffer& CommandBuffer::operator=(CommandBuffer&& other) noexcept
	{
		pool_   = std::move(other.pool_);
		handle_ = other.handle_;

		return *this;
	}

	void CommandBuffer::begin()
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		VK_CALL(vkBeginCommandBuffer(handle_, &beginInfo),
		        "Failed to begin recording command buffer");
	}

	void CommandBuffer::end()
	{
		VK_CALL(vkEndCommandBuffer(handle_),
		        "Failed to end recording command buffer");
	}
} // namespace sat
