#include "buffer.hpp"

#include <cstring>

#include "command.hpp"
#include "device.hpp"
#include "error.hpp"
#include "instance.hpp"
#include "sync.hpp"

namespace sat
{
	/////////////////////////////
	//// Buffer Pool Builder ////
	/////////////////////////////

	BufferPoolBuilder::BufferPoolBuilder(rn<Device> device,
	                                     rn<CommandPool> commandPool,
	                                     VkQueue queue) noexcept
	    : device_(std::move(device)),
	      commandPool_(std::move(commandPool)),
	      queue_(queue)
	{}

	/////////////////////
	//// Buffer Pool ////
	/////////////////////

	BufferPool::BufferPool(const BufferPoolBuilder& builder) noexcept
	    : device_(builder.device_),
	      commandPool_(builder.commandPool_),
	      queue_(builder.queue_)
	{
		VmaVulkanFunctions functions{};
		functions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
		functions.vkGetDeviceProcAddr   = &vkGetDeviceProcAddr;

		VmaAllocatorCreateInfo createInfo{};
		createInfo.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
		createInfo.vulkanApiVersion = VK_API_VERSION_1_2;
		createInfo.physicalDevice   = device_->device().handle;
		createInfo.device           = device_;
		createInfo.instance         = device_->instance()->handle();
		createInfo.pVulkanFunctions = &functions;

		SATURN_CALL(vmaCreateAllocator(&createInfo, &allocator_));
	}

	BufferPool::~BufferPool() noexcept
	{
		vmaDestroyAllocator(allocator_);
	}

	///////////////////////
	/// Buffer Builder ////
	///////////////////////

	BufferBuilder::BufferBuilder(rn<BufferPool> pool) noexcept
	    : pool_(std::move(pool))
	{}

	BufferBuilder& BufferBuilder::size(VkDeviceSize size) noexcept
	{
		size_ = size;
		return *this;
	}

	BufferBuilder& BufferBuilder::usage(VkBufferUsageFlagBits usage) noexcept
	{
		usage_ = usage;
		return *this;
	}

	BufferBuilder& BufferBuilder::share(uint32_t queueFamilyIndex) noexcept
	{
		queueFamilyIndices_.push_back(queueFamilyIndex);
		return *this;
	}

	////////////////
	//// Buffer ////
	////////////////

	Buffer::Buffer(const BufferBuilder& builder)
	    : pool_(builder.pool_), size_(builder.size_)
	{
		VkBufferCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		createInfo.size  = size_;
		createInfo.usage = builder.usage_ | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

		if (builder.queueFamilyIndices_.empty())
		{
			createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}
		else
		{
			createInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount =
			    builder.queueFamilyIndices_.size();
			createInfo.pQueueFamilyIndices = builder.queueFamilyIndices_.data();
		}

		VmaAllocationCreateInfo allocInfo{};
		allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
		allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

		SATURN_CALL(vmaCreateBuffer(pool_->allocator_,
		                            &createInfo,
		                            &allocInfo,
		                            &handle_,
		                            &allocation_,
		                            nullptr));

		/////////////////
		//// Staging ////
		/////////////////

		createInfo             = {};
		createInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		createInfo.size        = size_;
		createInfo.usage       = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		allocInfo       = {};
		allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
		allocInfo.flags =
		    VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
		    VMA_ALLOCATION_CREATE_MAPPED_BIT;

		SATURN_CALL(vmaCreateBuffer(pool_->allocator_,
		                            &createInfo,
		                            &allocInfo,
		                            &stagingHandle_,
		                            &stagingAllocation_,
		                            &allocInfo_));
	}

	Buffer::~Buffer() noexcept
	{
		vmaDestroyBuffer(pool_->allocator_, stagingHandle_, stagingAllocation_);
		vmaDestroyBuffer(pool_->allocator_, handle_, allocation_);
	}

	void Buffer::put(const void* pData, size_t size, size_t offset) noexcept
	{
		std::memcpy(static_cast<uint8_t*>(allocInfo_.pMappedData) + offset,
		            pData,
		            size);

		CommandBuffer cmd = pool_->commandPool_->allocate();

		cmd.record(true);
		cmd.copy(handle_, stagingHandle_, size_);
		cmd.stop();

		VkSubmitInfo submitInfo{};
		submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers    = cmd;

		rn<Fence> fence = sync::fence(pool_->device_);

		vkQueueSubmit(pool_->queue_, 1, &submitInfo, fence);

		fence->wait();

		pool_->commandPool_->free(cmd);
	}
} // namespace sat
