#include "instance.hpp"

#include <cstring>
#include <sstream>

#include "error.hpp"
#include "physical_device.hpp"

namespace sat
{
	//////////////////////////
	//// Instance Builder ////
	//////////////////////////

	InstanceBuilder::InstanceBuilder() noexcept
	{
#if SATURN_ENABLE_VALIDATION
		extensions_.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		layers_.push_back("VK_LAYER_KHRONOS_validation");
#endif
	}

	InstanceBuilder& InstanceBuilder::applicationName(
	    const std::string& name) noexcept
	{
		appName_ = name;
		return *this;
	}

	InstanceBuilder& InstanceBuilder::applicationVersion(int major,
	                                                     int minor,
	                                                     int patch) noexcept
	{
		appVersion_ = VK_MAKE_VERSION(major, minor, patch);
		return *this;
	}

	InstanceBuilder& InstanceBuilder::engineName(
	    const std::string& name) noexcept
	{
		engineName_ = name;
		return *this;
	}

	InstanceBuilder& InstanceBuilder::engineVersion(int major,
	                                                int minor,
	                                                int patch) noexcept
	{
		engineVersion_ = VK_MAKE_VERSION(major, minor, patch * 3);
		return *this;
	}

	InstanceBuilder& InstanceBuilder::addExtension(
	    const char* pExtensionName) noexcept
	{
		extensions_.push_back(pExtensionName);
		return *this;
	}

	InstanceBuilder& InstanceBuilder::addLayer(const char* pLayerName) noexcept
	{
		layers_.push_back(pLayerName);
		return *this;
	}

	InstanceBuilder& InstanceBuilder::debugCallback(
	    DebugCallback callback) noexcept
	{
		callback_ = std::move(callback);
		return *this;
	}

	//////////////////
	//// Instance ////
	//////////////////

	namespace
	{
		void evaluate_instance_extensions(
		    const std::vector<const char*>& required)
		{
			uint32_t count;
			vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
			std::vector<VkExtensionProperties> extensions(count);
			vkEnumerateInstanceExtensionProperties(
			    nullptr, &count, extensions.data());

			for (const char* pExtensionName : required)
			{
				for (const VkExtensionProperties& props : extensions)
				{
					if (strcmp(props.extensionName, pExtensionName) == 0)
					{
						goto next;
					}
				}

				SATURN_THROW(MissingFeatureException, pExtensionName);
			next:;
			}
		}

		void evaluate_instance_layers(const std::vector<const char*>& required)
		{
			uint32_t count;
			vkEnumerateInstanceLayerProperties(&count, nullptr);
			std::vector<VkLayerProperties> layers(count);
			vkEnumerateInstanceLayerProperties(&count, layers.data());

			for (const char* pLayerName : required)
			{
				for (const VkLayerProperties& props : layers)
				{
					if (strcmp(props.layerName, pLayerName) == 0)
					{
						goto next;
					}
				}

				SATURN_THROW(MissingFeatureException, pLayerName);
			next:;
			}
		}

		VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
		    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		    VkDebugUtilsMessageTypeFlagsEXT messageType,
		    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		    void* pUserData)
		{
			reinterpret_cast<std::optional<DebugCallback>*>(pUserData)->value()(
			    messageSeverity, messageType, pCallbackData->pMessage);

			return VK_FALSE;
		}
	} // namespace

	Instance::Instance(const InstanceBuilder& builder)
	    : callback_(builder.callback_)
	{
		evaluate_instance_extensions(builder.extensions_);
		evaluate_instance_layers(builder.layers_);

		VkApplicationInfo appInfo{};
		appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName   = builder.appName_.c_str();
		appInfo.applicationVersion = builder.appVersion_;
		appInfo.pEngineName        = builder.engineName_.c_str();
		appInfo.engineVersion      = builder.engineVersion_;
		appInfo.apiVersion         = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount   = builder.extensions_.size();
		createInfo.ppEnabledExtensionNames = builder.extensions_.data();
		createInfo.enabledLayerCount       = builder.layers_.size();
		createInfo.ppEnabledLayerNames     = builder.layers_.data();

#if SATURN_ENABLE_VALIDATION
		VkDebugUtilsMessengerCreateInfoEXT messengerInfo{};

		if (callback_.has_value())
		{
			messengerInfo.sType =
			    VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			messengerInfo.messageSeverity =
			    VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			    VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
			    VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			messengerInfo.messageType =
			    VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			messengerInfo.pfnUserCallback = debug_callback;
			messengerInfo.pUserData       = &callback_;

			createInfo.pNext = &messengerInfo;
		}
#endif

		SATURN_CALL(vkCreateInstance(&createInfo, nullptr, &handle_));

#if SATURN_ENABLE_VALIDATION
		if (callback_.has_value())
		{
			auto vkCreateDebugUtilsMessengerEXT =
			    SATURN_GET(handle_, vkCreateDebugUtilsMessengerEXT);

			SATURN_CALL(vkCreateDebugUtilsMessengerEXT(
			    handle_, &messengerInfo, nullptr, &messenger_));
		}
#endif
	}

	Instance::~Instance() noexcept
	{
#if SATURN_ENABLE_VALIDATION
		if (messenger_ != VK_NULL_HANDLE)
		{
			auto vkDestroyDebugUtilsMessengerEXT =
			    SATURN_GET(handle_, vkDestroyDebugUtilsMessengerEXT);

			vkDestroyDebugUtilsMessengerEXT(handle_, messenger_, nullptr);
		}
#endif

		vkDestroyInstance(handle_, nullptr);
	}

	std::vector<PhysicalDevice> Instance::devices() const noexcept
	{
		uint32_t count;
		vkEnumeratePhysicalDevices(handle_, &count, nullptr);
		std::vector<VkPhysicalDevice> handles(count);
		vkEnumeratePhysicalDevices(handle_, &count, handles.data());

		std::vector<PhysicalDevice> devices(count);

		while (count--)
		{
			devices[count] = device::query(handles[count]);
		}

		return devices;
	}
} // namespace sat
