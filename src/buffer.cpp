#include "buffer.hpp"

#include <cstring>

#include "command.hpp"
#include "device.hpp"
#include "error.hpp"
#include "sync.hpp"

namespace sat
{
	///////////////////////
	//// Mapped Buffer ////
	///////////////////////

	class MappedBuffer : public Buffer
	{
	public:
		MappedBuffer(rn<Device> device,
		             VkDeviceSize size,
		             VkBufferUsageFlags usage,
		             std::span<uint32_t const> queueFamilyIndices);

		MappedBuffer(const MappedBuffer&)            = delete;
		MappedBuffer& operator=(const MappedBuffer&) = delete;

		void put(const void* pData, size_t size, size_t offset) override;
	};

	MappedBuffer::MappedBuffer(rn<Device> device,
	                           VkDeviceSize size,
	                           VkBufferUsageFlags usage,
	                           std::span<uint32_t const> queueFamilyIndices)
	    : Buffer(std::move(device),
	             size,
	             usage,
	             queueFamilyIndices,
	             VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT)
	{}

	void MappedBuffer::put(const void* pData, size_t size, size_t offset)
	{
		void* pMap;
		SATURN_CALL(vmaMapMemory(device_->allocator(), allocation_, &pMap));

		std::memcpy(static_cast<uint8_t*>(pMap) + offset, pData, size);

		vmaUnmapMemory(device_->allocator(), allocation_);
	}

	///////////////////////
	//// Staged Buffer ////
	///////////////////////

	class StagedBuffer : public Buffer
	{
	public:
		StagedBuffer(rn<Device> device,
		             VkQueue queue,
		             rn<CommandDispatcher> dispatcher,
		             VkDeviceSize size,
		             VkBufferUsageFlags usage,
		             std::span<uint32_t const> queueFamilyIndices);

		StagedBuffer(const StagedBuffer&)            = delete;
		StagedBuffer& operator=(const StagedBuffer&) = delete;

		void put(const void* pData, size_t size, size_t offset) override;

	private:
		VkQueue queue_;
		rn<CommandDispatcher> dispatcher_;
		MappedBuffer staging_;
	};

	StagedBuffer::StagedBuffer(rn<Device> device,
	                           VkQueue queue,
	                           rn<CommandDispatcher> dispatcher,
	                           VkDeviceSize size,
	                           VkBufferUsageFlags usage,
	                           std::span<uint32_t const> queueFamilyIndices)
	    : queue_(queue),
	      dispatcher_(dispatcher),
	      Buffer(device,
	             size,
	             usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
	             queueFamilyIndices,
	             VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT),
	      staging_(std::move(device),
	               size,
	               usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	               {})
	{}

	void StagedBuffer::put(const void* pData, size_t size, size_t offset)
	{
		staging_.put(pData, size, offset);
		auto cmd = dispatcher_->lease();

		cmd->record(true);
		cmd->copy(handle_, staging_.handle(), size_);
		cmd->stop();

		VkSubmitInfo submitInfo{};
		submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers    = *cmd;

		rn<Fence> fence = sync::fence(device_);

		vkQueueSubmit(queue_, 1, &submitInfo, fence);

		fence->wait();
	}

	///////////////////////
	/// Buffer Builder ////
	///////////////////////

	BufferBuilder::BufferBuilder(rn<Device> device) noexcept
	    : device_(std::move(device))
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

	BufferBuilder& BufferBuilder::staged(
	    VkQueue queue, rn<CommandDispatcher> dispatcher) noexcept
	{
		staged_     = true;
		queue_      = queue;
		dispatcher_ = std::move(dispatcher);

		return *this;
	}

	rn<Buffer> BufferBuilder::build() const
	{
		if (staged_)
		{
			return rn<Buffer>(new StagedBuffer(device_,
			                                   queue_,
			                                   dispatcher_,
			                                   size_,
			                                   usage_,
			                                   queueFamilyIndices_));
		}
		else
		{
			return rn<Buffer>(
			    new MappedBuffer(device_, size_, usage_, queueFamilyIndices_));
		}
	}

	////////////////
	//// Buffer ////
	////////////////

	Buffer::Buffer(rn<Device> device,
	               VkDeviceSize size,
	               VkBufferUsageFlags usage,
	               std::span<uint32_t const> queueFamilyIndices,
	               VmaAllocationCreateFlags flags)
	    : device_(std::move(device)), size_(size)
	{
		VkBufferCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		createInfo.size  = size_;
		createInfo.usage = usage;

		if (queueFamilyIndices.empty())
		{
			createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}
		else
		{
			createInfo.sharingMode           = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = queueFamilyIndices.size();
			createInfo.pQueueFamilyIndices   = queueFamilyIndices.data();
		}

		VmaAllocationCreateInfo allocInfo{};
		allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
		allocInfo.flags = flags;

		SATURN_CALL(vmaCreateBuffer(device_->allocator(),
		                            &createInfo,
		                            &allocInfo,
		                            &handle_,
		                            &allocation_,
		                            nullptr));
	}

	Buffer::~Buffer() noexcept
	{
		vmaDestroyBuffer(device_->allocator(), handle_, allocation_);
	}
} // namespace sat
