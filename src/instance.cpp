#include "instance.hpp"

#include <sstream>

#include "local.hpp"
#include "physical_device.hpp"

namespace sat
{
	//////////////////////////
	//// Instance Builder ////
	//////////////////////////

	InstanceBuilder::InstanceBuilder() noexcept
	    : logger_(make_rn<Logger>()), vulkanLogger_(make_rn<Logger>())
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

	InstanceBuilder& InstanceBuilder::logger(rn<Logger> logger,
	                                         rn<Logger> vulkan) noexcept
	{
		logger_       = std::move(logger);
		vulkanLogger_ = std::move(vulkan);

		return *this;
	}

	rn<Instance> InstanceBuilder::build() const
	{
		return rn<Instance>(new Instance(*this));
	}

	//////////////////
	//// Instance ////
	//////////////////

	namespace
	{
		bool evaluate_instance_extensions(
		    Logger& logger, const std::vector<const char*>& required) noexcept
		{
			uint32_t count;
			vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
			std::vector<VkExtensionProperties> extensions(count);
			vkEnumerateInstanceExtensionProperties(
			    nullptr, &count, extensions.data());

			return evaluate_required_items<VkExtensionProperties>(
			    logger,
			    required,
			    extensions,
			    &VkExtensionProperties::extensionName,
			    "instance extensions");
		}

		bool evaluate_instance_layers(
		    Logger& logger, const std::vector<const char*>& required) noexcept
		{
			uint32_t count;
			vkEnumerateInstanceLayerProperties(&count, nullptr);
			std::vector<VkLayerProperties> layers(count);
			vkEnumerateInstanceLayerProperties(&count, layers.data());

			return evaluate_required_items<VkLayerProperties>(
			    logger,
			    required,
			    layers,
			    &VkLayerProperties::layerName,
			    "instance layers");
		}

		VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
		    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		    VkDebugUtilsMessageTypeFlagsEXT messageType,
		    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		    void* pUserData)
		{
			LogLevel level = [&]() {
				switch (messageSeverity)
				{
				case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
					return LogLevel::Trace;
				case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
					return LogLevel::Info;
				case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
					return LogLevel::Warn;
				default:
				case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
					return LogLevel::Error;
				}
			}();

			reinterpret_cast<Logger*>(pUserData)->log(
			    level, "(Vulkan) {}", pCallbackData->pMessage);

			return VK_FALSE;
		}
	} // namespace

	Instance::Instance(const InstanceBuilder& builder)
	    : logger_(builder.logger_)
	{
		if (!evaluate_instance_extensions(logger(), builder.extensions_) ||
		    !evaluate_instance_layers(logger(), builder.layers_))
		{
			throw std::runtime_error("Missing required features");
		}
		else
		{
			// Dump instance extensions

			std::ostringstream oss;
			oss << "Loading instance extenions:";

			for (const char* pExtensionName : builder.extensions_)
			{
				oss << "\n - " << pExtensionName;
			}

			S_INFO(oss.str());

			// Dump instance layers

			oss.str("");
			oss << "Loading instance layers:";

			for (const char* pLayerName : builder.layers_)
			{
				oss << "\n - " << pLayerName;
			}

			S_INFO(oss.str());
		}

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
		vulkanLogger_ = builder.vulkanLogger_;

		VkDebugUtilsMessengerCreateInfoEXT messengerInfo{};
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
		messengerInfo.pUserData       = vulkanLogger_.get();

		createInfo.pNext = &messengerInfo;
#endif

		VK_CALL(vkCreateInstance(&createInfo, nullptr, &handle_),
		        "Failed to create VkInstance");

		S_TRACE("Created instance " S_PTR " `{}`", S_THIS, builder.appName_);

#if SATURN_ENABLE_VALIDATION
		auto fn = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
		    vkGetInstanceProcAddr(handle_, "vkCreateDebugUtilsMessengerEXT"));
		if (fn == nullptr ||
		    fn(handle_, &messengerInfo, nullptr, &messenger_) != VK_SUCCESS)
		{
			S_WARN("Unable to create debug messenger");
		}
		else
		{
			S_TRACE("Created debug messenger");
		}
#endif
	}

	Instance::~Instance() noexcept
	{
#if SATURN_ENABLE_VALIDATION
		if (messenger_ != VK_NULL_HANDLE)
		{
			auto fn = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
			    vkGetInstanceProcAddr(handle_,
			                          "vkDestroyDebugUtilsMessengerEXT"));

			if (fn != nullptr)
			{
				fn(handle_, messenger_, nullptr);

				S_TRACE("Destroyed debug messenger");
			}
			else
			{
				S_ERROR("Unable to destroy debug messenger");
			}
		}
#endif

		vkDestroyInstance(handle_, nullptr);

		S_TRACE("Destroyed instance " S_PTR, S_THIS);
	}

	std::vector<PhysicalDevice> Instance::devices() const noexcept
	{
		uint32_t count;
		vkEnumeratePhysicalDevices(handle_, &count, nullptr);

		S_TRACE("Found {} physical devices", count);

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
