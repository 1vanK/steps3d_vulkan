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

int	isDeviceSuitable ( VkPhysicalDevice device )
{
	uint32_t			queueFamilyCount = 0;
	int					computeFamily    = -1;
	int					i                = 0;

	vkGetPhysicalDeviceQueueFamilyProperties ( device, &queueFamilyCount, nullptr );

	std::vector<VkQueueFamilyProperties> queueFamilies ( queueFamilyCount );

	vkGetPhysicalDeviceQueueFamilyProperties ( device, &queueFamilyCount, queueFamilies.data () );

	for ( const auto& queueFamily : queueFamilies )
	{
		if ( computeFamily == -1 && queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT )
			return i;

		i++;
	}

	return -1;
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

		// pick physical device
	VkPhysicalDevice	physicalDevice = VK_NULL_HANDLE;
	uint32_t			deviceCount    = 0;

	vkEnumeratePhysicalDevices ( instance, &deviceCount, nullptr );

	std::vector<VkPhysicalDevice>	physDevices ( deviceCount );

	vkEnumeratePhysicalDevices ( instance, &deviceCount, physDevices.data () );

	std::cout << "Found " << deviceCount << "physical devices" << std::endl;

	for (const auto& device : physDevices ) 
	{
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;

		vkGetPhysicalDeviceProperties ( device, &deviceProperties );		
		vkGetPhysicalDeviceFeatures   ( device, &deviceFeatures   );

		std::cout << deviceProperties.deviceName << (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ? " discrete" : " embedded" ) << std::endl;

		if ( deviceFeatures.geometryShader )
			std::cout << "Gemeotry shaders supported" << std::endl;

		if ( deviceFeatures.tessellationShader )
			std::cout << "Tessellation shaders supported" << std::endl;

		uint32_t queueFamilyCount = 0;
		
		vkGetPhysicalDeviceQueueFamilyProperties ( device, &queueFamilyCount, nullptr );

		std::vector<VkQueueFamilyProperties> queueFamilies ( queueFamilyCount );

		vkGetPhysicalDeviceQueueFamilyProperties ( device, &queueFamilyCount, queueFamilies.data () );

		std::cout << "Queue families " << queueFamilyCount << std::endl;

		for ( const auto& family : queueFamilies )
		{
			std::cout << "\tCount " << family.queueCount;

			if ( family.queueFlags &  VK_QUEUE_GRAPHICS_BIT )
				std::cout << " Graphics";

			if ( family.queueFlags & VK_QUEUE_COMPUTE_BIT )
				std::cout << " Compute";

			if ( family.queueFlags & VK_QUEUE_TRANSFER_BIT )
				std::cout << " Transfer";

			std::cout << std::endl;
		}
	}

	int	computeFamily  = -1;

	for ( const auto& dev : physDevices )
		if ( (computeFamily = isDeviceSuitable ( dev ) ) > -1 )	
		{
			physicalDevice = dev;
			break;
		}

	if ( physicalDevice == VK_NULL_HANDLE )
		fatal () << "VulkanWindow: failed to find a suitable GPU!";

	VkPhysicalDeviceProperties	deviceProperties;
	VkPhysicalDeviceFeatures	deviceFeatures;

	vkGetPhysicalDeviceProperties ( physicalDevice, &deviceProperties );		
	vkGetPhysicalDeviceFeatures   ( physicalDevice, &deviceFeatures   );

		// now we've picked physical device with queue family with compute support, we can create logical device
	VkDeviceQueueCreateInfo queueCreateInfo = {};
	float					queuePriority   = 1.0f;
	VkDeviceCreateInfo		devCreateInfo   = {};
	VkDevice				device          = VK_NULL_HANDLE;
	VkQueue					computeQueue    = VK_NULL_HANDLE;

	queueCreateInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = computeFamily;
	queueCreateInfo.queueCount       = 1;
	queueCreateInfo.pQueuePriorities = &queuePriority;				// XXX - what about other families, std::vector<VkDeviceQueueCreateInfo>

	devCreateInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	devCreateInfo.pQueueCreateInfos       = &queueCreateInfo;
	devCreateInfo.queueCreateInfoCount    = 1;
	devCreateInfo.pEnabledFeatures        = &deviceFeatures;
	devCreateInfo.enabledExtensionCount   = 0;			//static_cast<uint32_t>(deviceExtensions.size());
	devCreateInfo.ppEnabledExtensionNames = nullptr;	//deviceExtensions.data();
	devCreateInfo.enabledLayerCount       = 0;

		// device validation layers have been deprecated

	if ( vkCreateDevice ( physicalDevice, &devCreateInfo, nullptr, &device ) != VK_SUCCESS )
		fatal () << "VulknaWindow: failed to create logical device!";

	vkGetDeviceQueue ( device, computeFamily, 0, &computeQueue  );

	// create command pool
	VkCommandPoolCreateInfo	poolInfo    = {};
	VkCommandPool			commandPool = VK_NULL_HANDLE;

	poolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = computeFamily;

	if ( vkCreateCommandPool ( device, &poolInfo, nullptr, &commandPool ) != VK_SUCCESS )
		fatal () << "VulkanWindow: failed to create command pool!" << Log::endl;




	destroyDebugUtilsMessengerEXT ( instance, debugMessenger, nullptr );

	vkDestroyInstance ( instance, nullptr );

	return 0;
}
