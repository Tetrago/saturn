#include "shader.hpp"

#include <fstream>

#include "device.hpp"
#include "error.hpp"

namespace sat
{
	ShaderLoader::ShaderLoader(sat::rn<Device> device) noexcept
	    : device_(device)
	{}

	sat::rn<Shader> ShaderLoader::fromFile(
	    const std::filesystem::path& path) const
	{
		std::ifstream file(path, std::ios::binary);
		if (!file.is_open())
		{
			throw std::runtime_error("Failed to load shader from file");
		}

		return fromStream(std::move(file));
	}

	sat::rn<Shader> ShaderLoader::fromStream(std::istream&& stream) const
	{
		stream.seekg(0, std::ios::end);
		size_t size = stream.tellg();
		stream.seekg(0, std::ios::beg);

		std::vector<uint8_t> binary(size);
		stream.read(reinterpret_cast<char*>(binary.data()), size);

		return fromBytes(binary);
	}

	sat::rn<Shader> ShaderLoader::fromBytes(
	    std::span<uint8_t const> bytes) const
	{
		return sat::rn<Shader>(
		    new Shader(device_,
		               reinterpret_cast<const uint32_t*>(bytes.data()),
		               bytes.size()));
	}

	Shader::Shader(sat::rn<Device> device,
	               const uint32_t* pBinary,
	               size_t byteSize)
	    : device_(device)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = byteSize;
		createInfo.pCode    = pBinary;

		SATURN_CALL(
		    vkCreateShaderModule(device_, &createInfo, nullptr, &handle_));
	}

	Shader::~Shader() noexcept
	{
		vkDestroyShaderModule(device_, handle_, nullptr);
	}
} // namespace sat
