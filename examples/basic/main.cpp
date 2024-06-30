#include <saturn/saturn.hpp>

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
	        .require(crit::surface_support(surface))
	        .select()
	        .value();

	uint32_t graphicsQueueFamily =
	    dev::find_graphics_queue(physicalDevice).value();
	uint32_t presentQueueFamily =
	    dev::find_surface_queue(surface, physicalDevice).value();


	////////////////
	//// Device ////
	////////////////

	auto builder = sat::DeviceBuilder(instance, physicalDevice)
	                   .addQueue(graphicsQueueFamily);

	if (graphicsQueueFamily != presentQueueFamily)
	{
		// Don't request two queues unless they're under different families

		builder.addQueue(presentQueueFamily);
	}

	sat::rn<sat::Device> device = builder.build();

	VkQueue graphics = device->queue(graphicsQueueFamily);
	VkQueue preset   = device->queue(presentQueueFamily);

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

	device.reset();
	vkDestroySurfaceKHR(instance->handle(), surface, nullptr);
	instance.reset();

	glfwDestroyWindow(pWindow);
	glfwTerminate();

	return 0;
}
