#include	<vulkan/vulkan.h>
#include	"Log.h"

#include	<iostream>
#include	<vector>

const std::vector<const char*> validationLayers = 
{
	"VK_LAYER_KHRONOS_validation",
};

		// Vulkan debug callback
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback ( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData )
{
	if ( messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT )		// display only warning or higher
		log () << "validation layer: " << pCallbackData->pMessage << Log::endl;

	return VK_FALSE;
}

VkResult createDebugUtilsMessengerEXT ( VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger )
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

	if ( func != nullptr )
		return func ( instance, pCreateInfo, pAllocator, pDebugMessenger );

	return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void destroyDebugUtilsMessengerEXT ( VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator )
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

	if ( func != nullptr )
		func ( instance, debugMessenger, pAllocator );
}

void	populateDebugMessengerCreateInfo ( VkDebugUtilsMessengerCreateInfoEXT& createInfo )
{
	createInfo = {};

	createInfo.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT     | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT  | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
}

int main ()
{
	VkApplicationInfo appInfo = {};

	appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName   = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName        = "No Engine";	
	appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion         = VK_API_VERSION_1_0;

	VkInstance				instance;
	VkInstanceCreateInfo	createInfo = {};

	createInfo.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;	

	//uint32_t glfwExtensionCount = 0;
	//const char ** glfwExtensions;

	//glfwExtensions = glfwGetRequiredInstanceExtensions ( &glfwExtensionCount );

	const std::vector<const char*> requiredExtensions = { VK_EXT_DEBUG_UTILS_EXTENSION_NAME };


	VkDebugUtilsMessengerCreateInfoEXT	debugCreateInfo = {};
	VkDebugUtilsMessengerEXT			debugMessenger  = VK_NULL_HANDLE;

	populateDebugMessengerCreateInfo ( debugCreateInfo );


	createInfo.enabledExtensionCount   = (uint32_t) requiredExtensions.size ();		//glfwExtensionCount;
	createInfo.ppEnabledExtensionNames = requiredExtensions.data ();				//glfwExtensions;		
	createInfo.enabledLayerCount       = static_cast<uint32_t>(validationLayers.size());
	createInfo.ppEnabledLayerNames     = validationLayers.data();
	createInfo.pNext                   = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;		

	if ( vkCreateInstance ( &createInfo, nullptr, &instance ) != VK_SUCCESS) 
		fatal () << "failed to create instance!" << Log::endl;

	uint32_t extensionCount = 0;

	vkEnumerateInstanceExtensionProperties ( nullptr, &extensionCount, nullptr );

	std::vector<VkExtensionProperties> extensions ( extensionCount );

	vkEnumerateInstanceExtensionProperties ( nullptr, &extensionCount, extensions.data () );

	std::cout << "available extensions:" << std::endl;

	for ( const auto& extension : extensions ) 
		std::cout << "\t" << extension.extensionName << std::endl;

	uint32_t layerCount;

	vkEnumerateInstanceLayerProperties ( &layerCount, nullptr );

	std::vector<VkLayerProperties> availableLayers ( layerCount );

	vkEnumerateInstanceLayerProperties ( &layerCount, availableLayers.data () );

	std::cout << "available layers:" << std::endl;

	for ( auto& layerProperties : availableLayers )
		std::cout << '\t' << layerProperties.layerName << std::endl;

	populateDebugMessengerCreateInfo ( debugCreateInfo );

	if ( createDebugUtilsMessengerEXT ( instance, &debugCreateInfo, nullptr, &debugMessenger ) != VK_SUCCESS )
		fatal () << "failed to set up debug messenger!" << Log::endl;

	destroyDebugUtilsMessengerEXT ( instance, debugMessenger, nullptr );

	vkDestroyInstance ( instance, nullptr );

	return 0;
}
