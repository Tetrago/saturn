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
	class Device;

	////////////////////////
	//// Buffer Builder ////
	////////////////////////

	class SATURN_API BufferBuilder
	{
	public:
		explicit BufferBuilder(rn<Device> device) noexcept;

		BufferBuilder& size(VkDeviceSize size) noexcept;
		BufferBuilder& usage(VkBufferUsageFlagBits usage) noexcept;
		BufferBuilder& share(uint32_t queueFamilyIndex) noexcept;
		BufferBuilder& staged(VkQueue queue,
		                      rn<CommandDispatcher> dispatcher) noexcept;

		rn<Buffer> build() const;

	private:
		rn<Device> device_;
		VkDeviceSize size_;
		VkBufferUsageFlagBits usage_;
		std::vector<uint32_t> queueFamilyIndices_;
		bool staged_ = false;
		VkQueue queue_;
		rn<CommandDispatcher> dispatcher_;
	};

	////////////////
	//// Buffer ////
	////////////////

	class SATURN_API Buffer : public Container<VkBuffer>
	{
	public:
		virtual ~Buffer() noexcept;

		Buffer(const Buffer&)            = delete;
		Buffer& operator=(const Buffer&) = delete;

		template <typename T>
		void put(const std::vector<T>& data, size_t offset = 0);

		template <typename T>
		void put(std::span<T const> data, size_t offset = 0);

		virtual void put(const void* pData, size_t size, size_t offset = 0) = 0;

		VkDeviceSize size() const noexcept { return size_; }

	protected:
		Buffer(rn<Device> device,
		       VkDeviceSize size,
		       VkBufferUsageFlags usage,
		       std::span<uint32_t const> queueFamilyIndices,
		       VmaAllocationCreateFlags flags);

		rn<Device> device_;
		VkDeviceSize size_;
		VmaAllocation allocation_;
	};

	template <typename T>
	inline void Buffer::put(const std::vector<T>& data, size_t offset)
	{
		put(data.data(), data.size() * sizeof(T), offset);
	}

	template <typename T>
	inline void Buffer::put(std::span<T const> data, size_t offset)
	{
		put(data.data(), data.size() * sizeof(T), offset);
	}
} // namespace sat

#endif
