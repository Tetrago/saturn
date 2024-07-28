#ifndef SATURN_PIPELINE_HPP
#define SATURN_PIPELINE_HPP

#include <vulkan/vulkan.h>

#include <optional>
#include <span>
#include <vector>

#include "core.hpp"

namespace sat
{
	class RenderPass;
	class Device;
	class Pipeline;
	class Shader;
	class Swapchain;

	////////////////////////////
	//// Vertex Description ////
	////////////////////////////

	class SATURN_API VertexDescription
	{
	public:
		VertexDescription() noexcept = default;

		VertexDescription& begin(
		    size_t size,
		    VkVertexInputRate inputRate     = VK_VERTEX_INPUT_RATE_VERTEX,
		    std::optional<uint32_t> binding = std::nullopt) noexcept;

		VertexDescription& end() noexcept;

		VertexDescription& add(
		    VkFormat format,
		    size_t offset,
		    std::optional<uint32_t> location = std::nullopt) noexcept;

		std::span<VkVertexInputBindingDescription const> bindings()
		    const noexcept
		{
			return bindings_;
		}

		std::span<VkVertexInputAttributeDescription const> attributes()
		    const noexcept
		{
			return attributes_;
		}

	private:
		uint32_t nextBinding_ = 0;
		uint32_t nextLocation_;

		std::vector<VkVertexInputBindingDescription> bindings_;
		std::vector<VkVertexInputAttributeDescription> attributes_;
	};

	//////////////////////////
	//// Pipeline Builder ////
	//////////////////////////

	class SATURN_API PipelineBuilder : public Builder<PipelineBuilder, Pipeline>
	{
	public:
		PipelineBuilder(rn<Device> device,
		                rn<Swapchain> swapchain,
		                rn<RenderPass> renderPass) noexcept;

		PipelineBuilder& addStage(VkShaderStageFlagBits stage,
		                          rn<Shader> shader,
		                          const char* pEntrypoint = "main") noexcept;

		PipelineBuilder& addDynamicState(VkDynamicState state) noexcept;

		PipelineBuilder& topology(VkPrimitiveTopology topology) noexcept;
		PipelineBuilder& polygonMode(VkPolygonMode polygonMode) noexcept;
		PipelineBuilder& frontFace(VkFrontFace frontFace) noexcept;
		PipelineBuilder& subpass(uint32_t subpass) noexcept;
		PipelineBuilder& vertexDescription(
		    const VertexDescription& description) noexcept;

	private:
		friend class Pipeline;

		rn<Device> device_;
		rn<Swapchain> swapchain_;
		rn<RenderPass> renderPass_;
		std::vector<VkPipelineShaderStageCreateInfo> stages_;
		std::vector<rn<Shader>> shaders_;
		std::vector<VkDynamicState> dynamics_;
		VkPrimitiveTopology topology_ = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		VkPolygonMode polygonMode_    = VK_POLYGON_MODE_FILL;
		VkFrontFace frontFace_        = VK_FRONT_FACE_CLOCKWISE;
		uint32_t subpass_             = 0;
		VertexDescription description_;
	};

	//////////////////
	//// Pipeline ////
	//////////////////

	class SATURN_API Pipeline : public Container<VkPipeline>
	{
	public:
		~Pipeline() noexcept;

		Pipeline(const Pipeline&)            = delete;
		Pipeline& operator=(const Pipeline&) = delete;

	private:
		friend class Builder<PipelineBuilder, Pipeline>;

		explicit Pipeline(const PipelineBuilder& builder);

		rn<Device> device_;
		rn<Swapchain> swapchain_;
		rn<RenderPass> renderPass_;
		VkPipelineLayout layout_;
	};
} // namespace sat

#endif
