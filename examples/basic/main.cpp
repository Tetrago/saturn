#include <vulkan/vulkan_core.h>

#include <saturn/saturn.hpp>

#include "saturn/physical_device.hpp"
#include "saturn/pipeline.hpp"
#include "saturn/render_pass.hpp"
#include "saturn/swap_chain.hpp"

namespace crit = sat::criterion;
namespace dev  = sat::device;

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

	sat::rn<sat::Instance> instance =
	    sat::InstanceBuilder()
	        .applicationName("dev")
	        .applicationVersion(0, 1, 0)
	        .addGlfwExtensions()
	        .logger(
	            sat::make_rn<sat::ConsoleLogger>("dev", sat::LogLevel::Trace),
	            sat::make_rn<sat::ConsoleLogger>("vulkan", sat::LogLevel::Warn))
	        .build();

	/////////////////
	//// Surface ////
	/////////////////

	VkSurfaceKHR surface;
	if (glfwCreateWindowSurface(
	        instance->handle(), pWindow, nullptr, &surface) != VK_SUCCESS)
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
	VkQueue preset   = device->queue(presentQueueFamily);

	////////////////////
	//// Swap Chain ////
	////////////////////

	int width, height;
	glfwGetFramebufferSize(pWindow, &width, &height);

	auto swapChainBuilder =
	    sat::SwapChainBuilder(device, surface)
	        .selectSurfaceFormat(VK_FORMAT_B8G8R8A8_SRGB,
	                             VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
	        .selectPresentMode(VK_PRESENT_MODE_MAILBOX_KHR)
	        .extent(width, height);

	if (graphicsQueueFamily != presentQueueFamily)
	{
		swapChainBuilder.share({{graphicsQueueFamily, presentQueueFamily}});
	}

	sat::rn<sat::SwapChain> swapChain = swapChainBuilder.build();

	/////////////////////
	//// Render Pass ////
	/////////////////////

	sat::rn<sat::RenderPass> renderPass =
	    sat::RenderPassBuilder(device)
	        .createColorAttachment(swapChain->format())
	        .begin()
	        .addColorAttachment(0)
	        .end()
	        .build();

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
	    sat::PipelineBuilder(device, swapChain, renderPass)
	        .addStage(VK_SHADER_STAGE_VERTEX_BIT, vert)
	        .addStage(VK_SHADER_STAGE_FRAGMENT_BIT, frag)
	        .addDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
	        .addDynamicState(VK_DYNAMIC_STATE_SCISSOR)
	        .build();

	//////////////
	//// Loop ////
	//////////////

	while (!glfwWindowShouldClose(pWindow))
	{
		glfwPollEvents();
	}

	/////////////////
	//// Cleanup ////
	/////////////////

	pipeline.reset();
	frag.reset();
	vert.reset();

	renderPass.reset();
	swapChain.reset();
	device.reset();
	vkDestroySurfaceKHR(instance->handle(), surface, nullptr);
	instance.reset();

	glfwDestroyWindow(pWindow);
	glfwTerminate();

	return 0;
}
