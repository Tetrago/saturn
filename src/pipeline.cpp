#include "pipeline.hpp"

#include <vulkan/vulkan_core.h>

#include "local.hpp"
#include "render_pass.hpp"
#include "shader.hpp"
#include "swap_chain.hpp"

namespace sat
{
	//////////////////////////
	//// Pipeline Builder ////
	//////////////////////////

	PipelineBuilder::PipelineBuilder(rn<Device> device,
	                                 rn<SwapChain> swapChain,
	                                 rn<RenderPass> renderPass) noexcept
	    : device_(std::move(device)),
	      swapChain_(std::move(swapChain)),
	      renderPass_(renderPass)
	{}

	PipelineBuilder& PipelineBuilder::addStage(VkShaderStageFlagBits stage,
	                                           rn<Shader> shader,
	                                           const char* pEntrypoint) noexcept
	{
		VkPipelineShaderStageCreateInfo createInfo{};
		createInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		createInfo.stage  = stage;
		createInfo.module = shader->handle();
		createInfo.pName  = pEntrypoint;

		stages_.push_back(createInfo);
		shaders_.push_back(std::move(shader));

		return *this;
	}

	PipelineBuilder& PipelineBuilder::addDynamicState(
	    VkDynamicState state) noexcept
	{
		dynamics_.push_back(state);

		return *this;
	}

	PipelineBuilder& PipelineBuilder::topology(
	    VkPrimitiveTopology topology) noexcept
	{
		topology_ = topology;
		return *this;
	}

	PipelineBuilder& PipelineBuilder::polygonMode(
	    VkPolygonMode polygonMode) noexcept
	{
		polygonMode_ = polygonMode;
		return *this;
	}

	PipelineBuilder& PipelineBuilder::frontFace(VkFrontFace frontFace) noexcept
	{
		frontFace_ = frontFace;
		return *this;
	}

	PipelineBuilder& PipelineBuilder::subpass(uint32_t subpass) noexcept
	{
		subpass_ = subpass;
		return *this;
	}

	rn<Pipeline> PipelineBuilder::build() const
	{
		return rn<Pipeline>(new Pipeline(*this));
	}

	//////////////////
	//// Pipeline ////
	//////////////////

	Pipeline::Pipeline(const PipelineBuilder& builder)
	    : device_(builder.device_),
	      swapChain_(builder.swapChain_),
	      renderPass_(builder.renderPass_)
	{
		VkPipelineVertexInputStateCreateInfo vertexInputState{};
		vertexInputState.sType =
		    VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};
		inputAssemblyState.sType =
		    VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyState.topology               = builder.topology_;
		inputAssemblyState.primitiveRestartEnable = VK_FALSE;

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType =
		    VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = builder.dynamics_.size();
		dynamicState.pDynamicStates    = builder.dynamics_.data();

		VkViewport viewport{};
		viewport.width    = swapChain_->extent().width;
		viewport.height   = swapChain_->extent().height;
		viewport.maxDepth = 1;

		VkRect2D scissor{};
		scissor.extent = swapChain_->extent();

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType =
		    VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports    = &viewport;
		viewportState.scissorCount  = 1;
		viewportState.pScissors     = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizationState{};
		rasterizationState.sType =
		    VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationState.depthClampEnable        = VK_FALSE;
		rasterizationState.rasterizerDiscardEnable = VK_FALSE;
		rasterizationState.polygonMode             = builder.polygonMode_;
		rasterizationState.lineWidth               = 1;
		rasterizationState.cullMode                = VK_CULL_MODE_BACK_BIT;
		rasterizationState.frontFace               = VK_FRONT_FACE_CLOCKWISE;
		rasterizationState.depthBiasEnable         = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisampleState{};
		multisampleState.sType =
		    VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleState.sampleShadingEnable  = VK_FALSE;
		multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask =
		    VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
		    VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlendState{};
		colorBlendState.sType =
		    VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendState.logicOpEnable   = VK_FALSE;
		colorBlendState.attachmentCount = 1;
		colorBlendState.pAttachments    = &colorBlendAttachment;

		////////////////
		//// Layout ////
		////////////////

		VkPipelineLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

		VK_CALL(vkCreatePipelineLayout(
		            device_->handle(), &layoutInfo, nullptr, &layout_),
		        "Failed to create pipeline layout");

		//////////////////
		//// Pipeline ////
		//////////////////

		VkGraphicsPipelineCreateInfo createInfo{};
		createInfo.sType      = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		createInfo.stageCount = builder.stages_.size();
		createInfo.pStages    = builder.stages_.data();
		createInfo.pVertexInputState   = &vertexInputState;
		createInfo.pInputAssemblyState = &inputAssemblyState;
		createInfo.pViewportState      = &viewportState;
		createInfo.pRasterizationState = &rasterizationState;
		createInfo.pMultisampleState   = &multisampleState;
		createInfo.pColorBlendState    = &colorBlendState;
		createInfo.pDynamicState       = &dynamicState;
		createInfo.layout              = layout_;
		createInfo.renderPass          = renderPass_->handle();
		createInfo.subpass             = builder.subpass_;

		VK_CALL(vkCreateGraphicsPipelines(device_->handle(),
		                                  VK_NULL_HANDLE,
		                                  1,
		                                  &createInfo,
		                                  nullptr,
		                                  &handle_),
		        "Failed to create graphics pipeline");

		S_TRACE("Created graphics pipeline " S_PTR, S_THIS);
	}

	Pipeline::~Pipeline() noexcept
	{
		vkDestroyPipelineLayout(device_->handle(), layout_, nullptr);

		S_TRACE("Destroyed graphics pipeline " S_PTR, S_THIS);
	}
} // namespace sat
