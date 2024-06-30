#ifndef SATURN_INSTANCE_HPP
#define SATURN_INSTANCE_HPP

#include <vulkan/vulkan.h>

#include <cstdint>
#include <string>
#include <vector>

#include "core.hpp"
#include "logger.hpp"
#include "physical_device.hpp"

#ifdef SATURN_INCLUDE_GLFW
	#define GLFW_INCLUDE_NONE
	#include <GLFW/glfw3.h>
#endif

namespace sat
{
	class Instance;

	//////////////////////////
	//// Instance Builder ////
	//////////////////////////

	class SATURN_API InstanceBuilder
	{
	public:
		InstanceBuilder() noexcept;

		InstanceBuilder& applicationName(const std::string& name) noexcept;
		InstanceBuilder& applicationVersion(int major,
		                                    int minor,
		                                    int patch) noexcept;
		InstanceBuilder& engineName(const std::string& name) noexcept;
		InstanceBuilder& engineVersion(int major,
		                               int minor,
		                               int patch) noexcept;
		InstanceBuilder& addExtension(const char* pExtensionName) noexcept;
		InstanceBuilder& addLayer(const char* pLayerName) noexcept;
		InstanceBuilder& logger(rn<Logger> logger,
		                        rn<Logger> vulkan = make_rn<Logger>()) noexcept;

#ifdef SATURN_INCLUDE_GLFW
		/**
		 * Adds instance extensions required by GLFW. GLFW must have been
		 * initialized.
		 */
		InstanceBuilder& addGlfwExtensions() noexcept;
#endif

		rn<Instance> build() const;

	private:
		friend class Instance;

		std::string appName_    = "saturn";
		uint32_t appVersion_    = 0;
		std::string engineName_ = "saturn";
		uint32_t engineVersion_ = VK_MAKE_VERSION(0, 1, 0);
		std::vector<const char*> extensions_;
		std::vector<const char*> layers_;
		rn<Logger> logger_;
		rn<Logger> vulkanLogger_;
	};

	//////////////////
	//// Instance ////
	//////////////////

	class SATURN_API Instance
	{
	public:
		~Instance() noexcept;

		Instance(const Instance&)            = delete;
		Instance& operator=(const Instance&) = delete;

		std::vector<PhysicalDevice> devices() const noexcept;

		VkInstance handle() const noexcept { return handle_; }

		Logger& logger() const noexcept { return *logger_; }

	private:
		friend class InstanceBuilder;

		explicit Instance(const InstanceBuilder& builder);

		VkInstance handle_;
		rn<Logger> logger_;

#if SATURN_ENABLE_VALIDATION
		rn<Logger> vulkanLogger_;
		VkDebugUtilsMessengerEXT messenger_ = VK_NULL_HANDLE;
#endif
	};

	////////////////////
	//// Extensions ////
	////////////////////

#ifdef SATURN_INCLUDE_GLFW
	inline InstanceBuilder& InstanceBuilder::addGlfwExtensions() noexcept
	{
		uint32_t count            = 0;
		const char** ppExtensions = glfwGetRequiredInstanceExtensions(&count);

		while (count--)
		{
			extensions_.push_back(ppExtensions[count]);
		}

		return *this;
	}
#endif

} // namespace sat

#endif
