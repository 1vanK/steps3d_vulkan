#include	<vulkan/vulkan.h>
#include	"Log.h"

#include	<iostream>
#include	<vector>

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

	createInfo.enabledExtensionCount   = 0;		//glfwExtensionCount;
	createInfo.ppEnabledExtensionNames = nullptr;	//glfwExtensions;		
	createInfo.enabledLayerCount       = 0;		

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

	vkDestroyInstance ( instance, nullptr );

	return 0;
}
