#include <vulkan/vulkan_core.h>

#include <iostream>
#include <saturn/saturn.hpp>
#include <utility>

#include "saturn/pipeline.hpp"
#include "saturn/sync.hpp"

namespace crit = sat::criterion;
namespace dev  = sat::device;

void debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                    VkDebugUtilsMessageTypeFlagsEXT type,
                    std::string_view message)
{
	if (severity > VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
	{
		std::cout << "[Vulkan] " << message << std::endl;
	}
}

int main()
{
	////////////////
	//// Window ////
	////////////////

	if (!glfwInit())
	{
		throw std::runtime_error("Failed to initialize GLFW");
	}

	glfwDefaultWindowHints();
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	GLFWwindow* pWindow = glfwCreateWindow(1280, 720, "dev", nullptr, nullptr);

	//////////////////
	//// Instance ////
	//////////////////

	sat::rn<sat::Instance> instance = sat::InstanceBuilder()
	                                      .applicationName("dev")
	                                      .applicationVersion(0, 1, 0)
	                                      .addGlfwExtensions()
	                                      .debugCallback(debug_callback)
	                                      .build();

	/////////////////
	//// Surface ////
	/////////////////

	VkSurfaceKHR surface;
	if (glfwCreateWindowSurface(instance, pWindow, nullptr, &surface) !=
	    VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create window surface");
	}

	/////////////////////////
	//// Physical Device ////
	/////////////////////////

	sat::PhysicalDevice physicalDevice =
	    sat::PhysicalDeviceSelector(instance)
	        .prefer(crit::weigh_devices())
	        .require(crit::graphics_queue_family())
	        .require(crit::present_queue_family(surface))
	        .require(crit::present_capable(surface))
	        .require(crit::extension(VK_KHR_SWAPCHAIN_EXTENSION_NAME))
	        .select()
	        .value();

	uint32_t graphicsQueueFamily =
	    dev::find_graphics_queue(physicalDevice).value();
	uint32_t presentQueueFamily =
	    dev::find_present_queue(surface, physicalDevice).value();


	////////////////
	//// Device ////
	////////////////

	auto deviceBuilder = sat::DeviceBuilder(instance, physicalDevice)
	                         .addExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME)
	                         .addQueue(graphicsQueueFamily);

	if (graphicsQueueFamily != presentQueueFamily)
	{
		// Don't request two queues unless they're under different families

		deviceBuilder.addQueue(presentQueueFamily);
	}

	sat::rn<sat::Device> device = deviceBuilder.build();

	VkQueue graphics = device->queue(graphicsQueueFamily);
	VkQueue present  = device->queue(presentQueueFamily);

	////////////////////
	//// Swap Chain ////
	////////////////////

	int width, height;
	glfwGetFramebufferSize(pWindow, &width, &height);

	auto swapchainBuilder =
	    sat::SwapchainBuilder(device, surface)
	        .selectSurfaceFormat(VK_FORMAT_B8G8R8A8_SRGB,
	                             VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
	        .selectPresentMode(VK_PRESENT_MODE_MAILBOX_KHR)
	        .extent(width, height);

	if (graphicsQueueFamily != presentQueueFamily)
	{
		swapchainBuilder.share(graphicsQueueFamily).share(presentQueueFamily);
	}

	sat::rn<sat::Swapchain> swapchain = swapchainBuilder.build();

	/////////////////////
	//// Render Pass ////
	/////////////////////

	sat::rn<sat::RenderPass> renderPass =
	    sat::RenderPassBuilder(device)
	        .createColorAttachment(swapchain->format())
	        .begin()
	        .addColorAttachment(0)
	        .end()
	        .build();

	//////////////////////
	//// Framebuffers ////
	//////////////////////

	std::vector<sat::rn<sat::Framebuffer>> framebuffers;
	framebuffers.reserve(swapchain->views().size());

	for (VkImageView view : swapchain->views())
	{
		sat::rn<sat::Framebuffer> framebuffer =
		    sat::FramebufferBuilder(device, renderPass)
		        .extent(swapchain->extent())
		        .add(view)
		        .build();

		framebuffers.push_back(framebuffer);
	}

	/////////////////
	//// Shaders ////
	/////////////////

	sat::ShaderLoader loader(device);
	sat::rn<sat::Shader> vert = loader.fromFile("basic.vert.spv");
	sat::rn<sat::Shader> frag = loader.fromFile("basic.frag.spv");

	//////////////////
	//// Pipeline ////
	//////////////////

	sat::rn<sat::Pipeline> pipeline =
	    sat::PipelineBuilder(device, swapchain, renderPass)
	        .addStage(VK_SHADER_STAGE_VERTEX_BIT, vert)
	        .addStage(VK_SHADER_STAGE_FRAGMENT_BIT, frag)
	        .addDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
	        .addDynamicState(VK_DYNAMIC_STATE_SCISSOR)
	        .vertexDescription(
	            sat::VertexDescription()
	                .begin(sizeof(float) * 5)
	                .add(VK_FORMAT_R32G32_SFLOAT, 0)
	                .add(VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 2)
	                .end())
	        .build();

	/////////////////
	//// Command ////
	/////////////////

	sat::rn<sat::CommandPool> pool = sat::CommandPoolBuilder(device)
	                                     .queueFamilyIndex(graphicsQueueFamily)
	                                     .reset()
	                                     .build();

	sat::CommandBuffer cmd = pool->allocate();

	////////////////
	//// Buffer ////
	////////////////

	sat::rn<sat::CommandDispatcher> dispatcher =
	    sat::CommandDispatcherBuilder(pool).count(1).build();

	/* clang-format off */
	std::vector<float> vertices = {
        -0.5f, -0.5f, 1, 0, 1,
        0.5f, -0.5f, 0, 1, 0,
        0.5f, 0.5f, 0, 0, 1,
        -0.5f, 0.5f, 1, 1, 1
    };

    std::vector<uint32_t> indices = {
        0, 1, 2,
        2, 3, 0
    };
	/* clang-format on */

	sat::rn<sat::Buffer> vertex = sat::BufferBuilder(device)
	                                  .size(vertices.size() * sizeof(float))
	                                  .usage(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
	                                  .staged(graphics, dispatcher)
	                                  .build();

	sat::rn<sat::Buffer> index = sat::BufferBuilder(device)
	                                 .size(indices.size() * sizeof(uint32_t))
	                                 .usage(VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
	                                 .staged(graphics, dispatcher)
	                                 .build();

	vertex->put(vertices);
	index->put(indices);

	//////////////
	//// Sync ////
	//////////////

	sat::rn<sat::Semaphore> imageAvailableSemaphore =
	    sat::sync::semaphore(device);
	sat::rn<sat::Semaphore> renderFinishedSemaphore =
	    sat::sync::semaphore(device);
	sat::rn<sat::Fence> inFlightFence = sat::sync::fence(device, true);

	//////////////
	//// Loop ////
	//////////////

	uint32_t imageIndex;

	while (!glfwWindowShouldClose(pWindow))
	{
		glfwPollEvents();

		inFlightFence->wait();
		inFlightFence->reset();

		if (!swapchain->acquireNextImage(imageAvailableSemaphore, imageIndex))
		{
			// Re-create swapchain
		}

		////////////////
		//// Record ////
		////////////////

		cmd.reset();
		cmd.record();
		cmd.begin(renderPass, framebuffers[imageIndex], swapchain->extent());

		cmd.bindPipeline(pipeline);
		cmd.bindVertexBuffer(vertex);
		cmd.bindIndexBuffer(index);
		cmd.viewport(swapchain->extent());
		cmd.scissor(swapchain->extent());
		cmd.drawIndexed(indices.size());

		cmd.end();
		cmd.stop();

		////////////////
		//// Submit ////
		////////////////

		VkPipelineStageFlags waitStages[]{VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT};

		VkSubmitInfo submitInfo{};
		submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount   = 1;
		submitInfo.pWaitSemaphores      = imageAvailableSemaphore;
		submitInfo.pWaitDstStageMask    = waitStages;
		submitInfo.commandBufferCount   = 1;
		submitInfo.pCommandBuffers      = cmd;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores    = renderFinishedSemaphore;

		SATURN_CALL(vkQueueSubmit(graphics, 1, &submitInfo, inFlightFence));

		/////////////////
		//// Present ////
		/////////////////

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores    = renderFinishedSemaphore;
		presentInfo.swapchainCount     = 1;
		presentInfo.pSwapchains        = swapchain;
		presentInfo.pImageIndices      = &imageIndex;

		vkQueuePresentKHR(present, &presentInfo);
	}

	/////////////////
	//// Cleanup ////
	/////////////////

	device->waitIdle();

	inFlightFence.reset();
	renderFinishedSemaphore.reset();
	imageAvailableSemaphore.reset();

	index.reset();
	vertex.reset();
	dispatcher.reset();

	pool->free(cmd);
	pool.reset();

	pipeline.reset();
	frag.reset();
	vert.reset();

	for (sat::rn<sat::Framebuffer>& framebuffer : framebuffers)
	{
		framebuffer.reset();
	}

	renderPass.reset();
	swapchain.reset();
	device.reset();
	vkDestroySurfaceKHR(instance, surface, nullptr);
	instance.reset();

	glfwDestroyWindow(pWindow);
	glfwTerminate();

	return 0;
}
