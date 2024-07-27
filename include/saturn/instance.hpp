#ifndef SATURN_INSTANCE_HPP
#define SATURN_INSTANCE_HPP

#include <vulkan/vulkan.h>

#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include "core.hpp"
#include "physical_device.hpp"

#ifdef SATURN_INCLUDE_GLFW
	#define GLFW_INCLUDE_NONE
	#include <GLFW/glfw3.h>
#endif

namespace sat
{
	class Instance;

	using DebugCallback =
	    std::function<void(VkDebugUtilsMessageSeverityFlagBitsEXT,
	                       VkDebugUtilsMessageTypeFlagsEXT,
	                       std::string_view)>;

	//////////////////////////
	//// Instance Builder ////
	//////////////////////////

	class SATURN_API InstanceBuilder : public Builder<InstanceBuilder, Instance>
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
		InstanceBuilder& debugCallback(DebugCallback callback) noexcept;

#ifdef SATURN_INCLUDE_GLFW
		/**
		 * Adds instance extensions required by GLFW. GLFW must have been
		 * initialized.
		 */
		InstanceBuilder& addGlfwExtensions() noexcept;
#endif

	private:
		friend class Instance;

		std::string appName_    = "saturn";
		uint32_t appVersion_    = 0;
		std::string engineName_ = "saturn";
		uint32_t engineVersion_ = VK_MAKE_VERSION(0, 1, 0);
		std::vector<const char*> extensions_;
		std::vector<const char*> layers_;
		std::optional<DebugCallback> callback_;
	};

	//////////////////
	//// Instance ////
	//////////////////

	class SATURN_API Instance : public Container<VkInstance>
	{
	public:
		~Instance() noexcept;

		Instance(const Instance&)            = delete;
		Instance& operator=(const Instance&) = delete;

		std::vector<PhysicalDevice> devices() const noexcept;

	private:
		friend class Builder<InstanceBuilder, Instance>;

		explicit Instance(const InstanceBuilder& builder);

#if SATURN_ENABLE_VALIDATION
		std::optional<DebugCallback> callback_;
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
