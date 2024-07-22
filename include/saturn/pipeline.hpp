#ifndef SATURN_PIPELINE_HPP
#define SATURN_PIPELINE_HPP

#include <vulkan/vulkan.h>

#include <vector>

#include "core.hpp"
#include "device.hpp"
#include "instance.hpp"

namespace sat
{
	class RenderPass;
	class Shader;
	class SwapChain;

	//////////////////////////
	//// Pipeline Builder ////
	//////////////////////////

	class Pipeline;

	class SATURN_API PipelineBuilder
	{
	public:
		PipelineBuilder(rn<Device> device,
		                rn<SwapChain> swapChain,
		                rn<RenderPass> renderPass) noexcept;

		PipelineBuilder& addStage(VkShaderStageFlagBits stage,
		                          rn<Shader> shader,
		                          const char* pEntrypoint = "main") noexcept;

		PipelineBuilder& addDynamicState(VkDynamicState state) noexcept;

		PipelineBuilder& topology(VkPrimitiveTopology topology) noexcept;
		PipelineBuilder& polygonMode(VkPolygonMode polygonMode) noexcept;
		PipelineBuilder& frontFace(VkFrontFace frontFace) noexcept;
		PipelineBuilder& subpass(uint32_t subpass) noexcept;

		sat::rn<Pipeline> build() const;

	private:
		friend class Pipeline;

		rn<Device> device_;
		rn<SwapChain> swapChain_;
		rn<RenderPass> renderPass_;
		std::vector<VkPipelineShaderStageCreateInfo> stages_;
		std::vector<rn<Shader>> shaders_;
		std::vector<VkDynamicState> dynamics_;
		VkPrimitiveTopology topology_ = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		VkPolygonMode polygonMode_    = VK_POLYGON_MODE_FILL;
		VkFrontFace frontFace_        = VK_FRONT_FACE_CLOCKWISE;
		uint32_t subpass_             = 0;
	};

	//////////////////
	//// Pipeline ////
	//////////////////

	class SATURN_API Pipeline
	{
	public:
		~Pipeline() noexcept;

		Pipeline(const Pipeline&)            = delete;
		Pipeline& operator=(const Pipeline&) = delete;

		VkPipeline handle() const noexcept { return handle_; }

	private:
		friend class PipelineBuilder;

		Pipeline(const PipelineBuilder& builder);

		Logger& logger() const noexcept { return device_->instance().logger(); }

		rn<Device> device_;
		rn<SwapChain> swapChain_;
		rn<RenderPass> renderPass_;
		VkPipelineLayout layout_;
		VkPipeline handle_;
	};
} // namespace sat

#endif
