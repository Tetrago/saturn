#ifndef SATURN_SHADER_HPP
#define SATURN_SHADER_HPP

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <filesystem>
#include <istream>
#include <span>

#include "core.hpp"
#include "device.hpp"

namespace sat
{
	class Device;

	///////////////////////
	//// Shader Loader ////
	///////////////////////

	class Shader;

	class SATURN_API ShaderLoader
	{
	public:
		ShaderLoader(sat::rn<Device> device) noexcept;

		sat::rn<Shader> fromFile(const std::filesystem::path& path) const;
		sat::rn<Shader> fromStream(std::istream&& stream) const;
		sat::rn<Shader> fromBytes(std::span<uint8_t const> bytes) const;

	private:
		Logger& logger() const noexcept { return device_->instance().logger(); }

		sat::rn<Device> device_;
	};

	////////////////
	//// Shader ////
	////////////////

	class SATURN_API Shader
	{
	public:
		~Shader() noexcept;

		Shader(const Shader&)            = delete;
		Shader& operator=(const Shader&) = delete;

		VkShaderModule handle() const noexcept { return handle_; }

	private:
		friend class ShaderLoader;

		Shader(sat::rn<Device> device,
		       const uint32_t* pBinary,
		       size_t byteSize);

		Logger& logger() const noexcept { return device_->instance().logger(); }

		sat::rn<Device> device_;
		VkShaderModule handle_;
	};
} // namespace sat

#endif
