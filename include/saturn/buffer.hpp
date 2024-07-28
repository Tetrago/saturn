#ifndef SATURN_BUFFER_HPP
#define SATURN_BUFFER_HPP

#include <vulkan/vulkan.h>

#include <span>
#include <vector>

#include "allocator.hpp"
#include "command.hpp"
#include "core.hpp"

namespace sat
{
	class Buffer;
	class BufferPool;
	class Device;

	/////////////////////////////
	//// Buffer Pool Builder ////
	/////////////////////////////

	class SATURN_API BufferPoolBuilder
	    : public Builder<BufferPoolBuilder, BufferPool>
	{
	public:
		BufferPoolBuilder(rn<Device> device,
		                  rn<CommandPool> commandPool,
		                  VkQueue queue) noexcept;

	private:
		friend class BufferPool;

		rn<Device> device_;
		rn<CommandPool> commandPool_;
		VkQueue queue_;
	};

	/////////////////////
	//// Buffer Pool ////
	/////////////////////

	class SATURN_API BufferPool
	{
	public:
		~BufferPool() noexcept;

		BufferPool(const BufferPool&)            = delete;
		BufferPool& operator=(const BufferPool&) = delete;

	private:
		friend class Buffer;
		friend class Builder<BufferPoolBuilder, BufferPool>;

		BufferPool(const BufferPoolBuilder& builder) noexcept;

		rn<Device> device_;
		rn<CommandPool> commandPool_;
		VkQueue queue_;
		VmaAllocator allocator_;
	};

	////////////////////////
	//// Buffer Builder ////
	////////////////////////

	class SATURN_API BufferBuilder : public Builder<BufferBuilder, Buffer>
	{
	public:
		BufferBuilder(rn<BufferPool> pool) noexcept;

		BufferBuilder& size(VkDeviceSize size) noexcept;
		BufferBuilder& usage(VkBufferUsageFlagBits usage) noexcept;
		BufferBuilder& share(uint32_t queueFamilyIndex) noexcept;

	private:
		friend class Buffer;

		rn<BufferPool> pool_;
		VkDeviceSize size_;
		VkBufferUsageFlagBits usage_;
		std::vector<uint32_t> queueFamilyIndices_;
	};

	////////////////
	//// Buffer ////
	////////////////

	class SATURN_API Buffer : public Container<VkBuffer>
	{
	public:
		~Buffer() noexcept;

		Buffer(const Buffer&)            = delete;
		Buffer& operator=(const Buffer&) = delete;

		template <typename T>
		void put(std::span<T const> data, size_t offset = 0) noexcept;

		void put(const void* pData, size_t size, size_t offset = 0) noexcept;

	private:
		friend class Builder<BufferBuilder, Buffer>;

		Buffer(const BufferBuilder& builder);

		rn<BufferPool> pool_;
		VkDeviceSize size_;
		VmaAllocation allocation_;
		VkBuffer stagingHandle_;
		VmaAllocation stagingAllocation_;
		VmaAllocationInfo allocInfo_;
	};

	template <typename T>
	inline void Buffer::put(std::span<T const> data, size_t offset) noexcept
	{
		put(data.data(), data.size() * sizeof(T), offset);
	}
} // namespace sat

#endif
