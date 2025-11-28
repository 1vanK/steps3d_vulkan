// Файл изменён. Изменения помечены 1vanK

// 1vanK: Добавлено
#include <cstring>

#include	<vulkan/vulkan.h>
#include	"Log.h"
#include	"Data.h"

#include	<iostream>
#include	<vector>

#define DEFAULT_FENCE_TIMEOUT 100000000000

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

	// check whether device is ok for us
	// here just check for compute support
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

	// return index of memory type with given property bits
	// satifying type bitmask
uint32_t findMemoryType ( VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties )
{
	VkPhysicalDeviceMemoryProperties	memoryProperties;

		// get memory properties for the device
	vkGetPhysicalDeviceMemoryProperties ( physicalDevice, &memoryProperties );

		// chgeck every nonmasked type to have all required bits
	for ( uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++ )
		if ( (typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties ) 
			return i;

		// type was not found
	fatal () << "GpuMemory: failed to find suitable memory type! " << properties;

	return 0;		// we won't get here, but compiler complains about returning no value
}

int main ()
{
		// start with providing info about our application
	VkApplicationInfo appInfo = {};

	appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName   = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName        = "No Engine";	
	appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion         = VK_API_VERSION_1_0;

		// create instance
	VkInstance				instance;
	VkInstanceCreateInfo	createInfo = {};

	createInfo.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;	

		// specify extensions that we require
	const std::vector<const char*> requiredExtensions = { VK_EXT_DEBUG_UTILS_EXTENSION_NAME };

		// we will require debug messenger support
	VkDebugUtilsMessengerCreateInfoEXT	debugCreateInfo = {};
	VkDebugUtilsMessengerEXT			debugMessenger  = VK_NULL_HANDLE;

	populateDebugMessengerCreateInfo ( debugCreateInfo );

		// now we're ready to creater instance
	createInfo.enabledExtensionCount   = (uint32_t) requiredExtensions.size ();		//glfwExtensionCount;
	createInfo.ppEnabledExtensionNames = requiredExtensions.data ();				//glfwExtensions;		
	createInfo.enabledLayerCount       = static_cast<uint32_t>(validationLayers.size());
	createInfo.ppEnabledLayerNames     = validationLayers.data();
	createInfo.pNext                   = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;		

	if ( vkCreateInstance ( &createInfo, nullptr, &instance ) != VK_SUCCESS) 
		fatal () << "failed to create instance!" << Log::endl;

		// print info about supported extensions
	uint32_t extensionCount = 0;

		// get number of extensions supported
	vkEnumerateInstanceExtensionProperties ( nullptr, &extensionCount, nullptr );

		// reserve memory for extensions info
	std::vector<VkExtensionProperties> extensions ( extensionCount );

		// get actual extensions properties
	vkEnumerateInstanceExtensionProperties ( nullptr, &extensionCount, extensions.data () );

		// print supported extensions
	std::cout << "available extensions:" << std::endl;

	for ( const auto& extension : extensions ) 
		std::cout << "\t" << extension.extensionName << std::endl;

		// now get info about supported debug layers
	uint32_t layerCount;

		// get number of supported layers
	vkEnumerateInstanceLayerProperties ( &layerCount, nullptr );

		// allocate memory for layers properties
	std::vector<VkLayerProperties> availableLayers ( layerCount );

		// get actual properties
	vkEnumerateInstanceLayerProperties ( &layerCount, availableLayers.data () );

		// print supported extensions
	std::cout << "available layers:" << std::endl;

	for ( auto& layerProperties : availableLayers )
		std::cout << '\t' << layerProperties.layerName << std::endl;

		// setup debug messenger
	populateDebugMessengerCreateInfo ( debugCreateInfo );

	if ( createDebugUtilsMessengerEXT ( instance, &debugCreateInfo, nullptr, &debugMessenger ) != VK_SUCCESS )
		fatal () << "failed to set up debug messenger!" << Log::endl;

		// pick suitable physical device
	VkPhysicalDevice	physicalDevice = VK_NULL_HANDLE;
	uint32_t			deviceCount    = 0;

		// get number of physical devices in the system
	vkEnumeratePhysicalDevices ( instance, &deviceCount, nullptr );

		// allocate memory for them
	std::vector<VkPhysicalDevice>	physDevices ( deviceCount );

		// get actual devices
	vkEnumeratePhysicalDevices ( instance, &deviceCount, physDevices.data () );

		// print info about all found devices
	std::cout << "Found " << deviceCount << "physical devices" << std::endl;

	for (const auto& device : physDevices ) 
	{
		VkPhysicalDeviceProperties	deviceProperties;
		VkPhysicalDeviceFeatures	deviceFeatures;

			// properties and features for device
		vkGetPhysicalDeviceProperties ( device, &deviceProperties );		
		vkGetPhysicalDeviceFeatures   ( device, &deviceFeatures   );

			// print whether it is descrete or embedded
		std::cout << deviceProperties.deviceName << (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ? " discrete" : " embedded" ) << std::endl;

			// print geometry shaders support
		if ( deviceFeatures.geometryShader )
			std::cout << "Gemeotry shaders supported" << std::endl;

			// print tessellation shaders support
		if ( deviceFeatures.tessellationShader )
			std::cout << "Tessellation shaders supported" << std::endl;

			// get supported queue families for the device
		uint32_t queueFamilyCount = 0;
		
			// get number of queue families
		vkGetPhysicalDeviceQueueFamilyProperties ( device, &queueFamilyCount, nullptr );

			// allocate memory for them
		std::vector<VkQueueFamilyProperties> queueFamilies ( queueFamilyCount );

			// get queue families properties
		vkGetPhysicalDeviceQueueFamilyProperties ( device, &queueFamilyCount, queueFamilies.data () );

			// print info for every found family
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

		// pick device with computer family suport and family index
	int	computeFamily  = -1;

	for ( const auto& dev : physDevices )
		if ( (computeFamily = isDeviceSuitable ( dev ) ) > -1 )	
		{
			physicalDevice = dev;
			break;
		}

	if ( physicalDevice == VK_NULL_HANDLE )
		fatal () << "VulkanWindow: failed to find a suitable GPU!";

		// get properties and features for chosen physical device
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

		// we will require only one compute queue
	queueCreateInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = computeFamily;
	queueCreateInfo.queueCount       = 1;
	queueCreateInfo.pQueuePriorities = &queuePriority;

		// prepare createInfo structure
	devCreateInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	devCreateInfo.pQueueCreateInfos       = &queueCreateInfo;
	devCreateInfo.queueCreateInfoCount    = 1;
	devCreateInfo.pEnabledFeatures        = &deviceFeatures;
	devCreateInfo.enabledExtensionCount   = 0;
	devCreateInfo.ppEnabledExtensionNames = nullptr;
	devCreateInfo.enabledLayerCount       = 0;

		// device validation layers have been deprecated, so we do not specify them

	if ( vkCreateDevice ( physicalDevice, &devCreateInfo, nullptr, &device ) != VK_SUCCESS )
		fatal () << "VulknaWindow: failed to create logical device!";

		// get compute queue from created logical device
	vkGetDeviceQueue ( device, computeFamily, 0, &computeQueue  );

		// create command pool for our (logical) device
	VkCommandPoolCreateInfo	poolInfo    = {};
	VkCommandPool			commandPool = VK_NULL_HANDLE;

	poolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = computeFamily;

	if ( vkCreateCommandPool ( device, &poolInfo, nullptr, &commandPool ) != VK_SUCCESS )
		fatal () << "VulkanWindow: failed to create command pool!" << Log::endl;

		// get memory properties for physical device
	VkPhysicalDeviceMemoryProperties	memoryProperties;

	vkGetPhysicalDeviceMemoryProperties ( physicalDevice, &memoryProperties );

		// print info about all found types and heaps
	std::cout << "Memory properties:" << std::endl;
	std::cout << "\tmemoryTypeCount " << memoryProperties.memoryTypeCount << std::endl;

	for ( uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++ )
	{
		std::cout << "\t" << i << ": ";

			// most efficient for device access
		if ( memoryProperties.memoryTypes [i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT )
			std::cout << "device local, ";

			// can be mapped for host access
		if ( memoryProperties.memoryTypes [i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT )
			std::cout << "host visible, ";

			// host cache management commands are not needed to flush host writes to device
			// or make device writes visible to host
		if ( memoryProperties.memoryTypes [i].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT )
			std::cout << "host coherent, ";

			// is cached on the host, uncached memory is slower but are always host coherent
		if ( memoryProperties.memoryTypes [i].propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT )
			std::cout << "host cached, ";

			// only allows device access
		if ( memoryProperties.memoryTypes [i].propertyFlags & VK_MEMORY_PROPERTY_PROTECTED_BIT )
			std::cout << "protected (1.1), ";

		std::cout << "Heap index " << memoryProperties.memoryTypes [i].heapIndex << std::endl;
	}

	std::cout << "memoryHeapCount " << memoryProperties.memoryHeapCount << std::endl;

	for ( uint32_t i = 0; i < memoryProperties.memoryHeapCount; i++ )
	{
		std::cout << "\t Heap " << i << ": ";

		std::cout << "\tSize " << memoryProperties.memoryHeaps [i].size;

			// heap belongs to device-only memory
		if ( memoryProperties.memoryHeaps [i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT )
			std::cout << " device local, ";

			// for logical device representing more than one physical device
		if ( memoryProperties.memoryHeaps [i].flags & VK_MEMORY_HEAP_MULTI_INSTANCE_BIT )
			std::cout << " multi instance ";

		std::cout << std::endl;
	}

		// now create storage buffer for 1024 floats
	VkBuffer				buffer     = VK_NULL_HANDLE;
	VkBufferCreateInfo		bufferInfo = {};
	uint32_t				size       = 1024 * sizeof ( float );
	VkMemoryRequirements	memRequirements;

	bufferInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size        = size;
	bufferInfo.usage       = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if ( vkCreateBuffer ( device, &bufferInfo, nullptr, &buffer ) != VK_SUCCESS )
		fatal () << "Buffer: failed to create buffer!" << std::endl;;

		// now we need to allocate memory for this buffer
		// so get memory requirements for it
	vkGetBufferMemoryRequirements ( device, buffer, &memRequirements );

		// allocate memory with required properties (
	VkMemoryAllocateInfo 	allocInfo  = {};
	VkDeviceMemory			memory     = VK_NULL_HANDLE;
	VkMemoryPropertyFlags	properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ;

	memRequirements.memoryTypeBits = properties;
	memRequirements.size           = size;

	allocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize  = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType ( physicalDevice, memRequirements.memoryTypeBits, properties );

	if ( vkAllocateMemory ( device, &allocInfo, nullptr, &memory ) != VK_SUCCESS )
		fatal () << "Cannot allocate memory for buffer" << std::endl;

		// map allocated memory into CPU address space and copy data
	void * data;
	std::vector<float>	testData ( 1024 );

	for ( int i = 0; i < 1024; i++ )
		testData [i] = i;

	vkMapMemory   ( device, memory, 0, size, 0, &data );
	memcpy        ( (char *) data,  testData.data (), size );
	vkUnmapMemory ( device, memory );

		// now we can bind allocated and initialized memory to our buffer
	vkBindBufferMemory ( device, buffer, memory, 0 );

		// load compute shader from .spv file
	VkShaderModule	shader = VK_NULL_HANDLE;
	Data			shaderSource ( "shaders/test-compute.comp.spv" );

	VkShaderModuleCreateInfo moduleCreateInfo = {};

	moduleCreateInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	moduleCreateInfo.codeSize = shaderSource.getLength ();
	moduleCreateInfo.pCode    = reinterpret_cast<const uint32_t*>( shaderSource.getPtr () );

	if ( vkCreateShaderModule ( device, &moduleCreateInfo, nullptr, &shader ) != VK_SUCCESS )
		fatal () << "Failed to create shader module! ";

		// prepare to create compute pipeline
	VkComputePipelineCreateInfo		pipelineInfo       = {};
	VkPipelineLayoutCreateInfo		pipelineLayoutInfo = {};

	pipelineLayoutInfo.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;

		// prepare layout bindings data
	std::vector<VkDescriptorSetLayoutBinding>	bindings;
	VkDescriptorSetLayoutBinding				layoutBinding = {};
	VkDescriptorSetLayout						descriptorSetLayout = VK_NULL_HANDLE;

	layoutBinding.binding            = 0;
	layoutBinding.descriptorCount    = 1;		// one storage buffer
	layoutBinding.descriptorType     = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	layoutBinding.pImmutableSamplers = nullptr;
												// for compute stage only
	layoutBinding.stageFlags         = VK_SHADER_STAGE_COMPUTE_BIT;

	bindings.push_back ( layoutBinding );

		// now we're ready to create descriptor set layout 
		// for our bindings array
	if ( bindings.size () > 0 )
	{
		VkDescriptorSetLayoutCreateInfo layoutInfo = {};

		layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = (uint32_t) bindings.size ();
		layoutInfo.pBindings    = bindings.data  ();

		if ( vkCreateDescriptorSetLayout ( device, &layoutInfo, nullptr, &descriptorSetLayout ) != VK_SUCCESS )
			fatal () << "DescSetLayout: failed to create descriptor set layout!";

			// set this layout for pipeline layout
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts    = &descriptorSetLayout;
	}

		// now create pipeline layout
	VkPipelineLayout 				pipelineLayout = VK_NULL_HANDLE;
	VkPipelineShaderStageCreateInfo stageInfo      = {};	

	if ( vkCreatePipelineLayout ( device, &pipelineLayoutInfo, nullptr, &pipelineLayout ) != VK_SUCCESS )
		fatal () << "Pipeline: failed to create pipeline layout!";

		// we have only one stage - compute 
	stageInfo.sType     = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stageInfo.stage     = VK_SHADER_STAGE_COMPUTE_BIT;
	stageInfo.module    = shader;
	stageInfo.pName     = "main";

	pipelineInfo.sType  = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	pipelineInfo.stage  = stageInfo;
	pipelineInfo.layout = pipelineLayout;

		// now we can create compute pipeline
	VkPipeline			pipeline       = VK_NULL_HANDLE;

	if ( vkCreateComputePipelines ( device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline ) != VK_SUCCESS )
		fatal () << "Pipeline: failed to create compute pipeline!";

		// allocate command buffer from pool
	VkCommandBuffer					commandBuffer = VK_NULL_HANDLE;
	VkCommandBufferAllocateInfo		commandBufferAllocInfo = {};

	commandBufferAllocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocInfo.commandPool        = commandPool;
	commandBufferAllocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocInfo.commandBufferCount = 1;

	if ( vkAllocateCommandBuffers ( device, &commandBufferAllocInfo, &commandBuffer ) != VK_SUCCESS )
		fatal () << "VulkanWindow: failed to allocate command buffers!";

		// prepare actual descriptor set
		// start with createing descriptor pool
	VkDescriptorPool 					descriptorPool = VK_NULL_HANDLE;
	VkDescriptorSet						set            = VK_NULL_HANDLE;
	std::vector<VkDescriptorPoolSize>	poolSizes      = {};
	uint32_t							maxSets        = 10;
	VkDescriptorPoolSize				item;

	item.type            = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	item.descriptorCount = 1;

	poolSizes.push_back ( item );

	VkDescriptorPoolCreateInfo descSetPoolInfo = {};

	descSetPoolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descSetPoolInfo.poolSizeCount = (uint32_t) poolSizes.size ();
	descSetPoolInfo.pPoolSizes    = poolSizes.data();
	descSetPoolInfo.maxSets       = maxSets;

	if ( vkCreateDescriptorPool ( device, &descSetPoolInfo, nullptr, &descriptorPool ) != VK_SUCCESS )
		fatal () << "DescriptorPool: failed to create descriptor pool!";

		// now allocate descriptor set
	VkDescriptorSetAllocateInfo descSetAllocInfo = {};

	descSetAllocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descSetAllocInfo.descriptorPool     = descriptorPool;
	descSetAllocInfo.descriptorSetCount = 1;
	descSetAllocInfo.pSetLayouts        = &descriptorSetLayout;

	if ( vkAllocateDescriptorSets ( device, &descSetAllocInfo, &set ) != VK_SUCCESS )
		fatal () << "DescriptorSet: failed to allocate descriptor sets!";

	std::vector<VkWriteDescriptorSet>	writes;
	VkDescriptorBufferInfo			  * descBufferInfo   = new VkDescriptorBufferInfo {};
	VkWriteDescriptorSet				descriptorWrites = {};

	descBufferInfo->buffer = buffer;
	descBufferInfo->offset = 0;
	descBufferInfo->range  = size;

	descriptorWrites.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites.dstSet          = set;
	descriptorWrites.dstBinding      = 0;
	descriptorWrites.dstArrayElement = 0;
	descriptorWrites.descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptorWrites.descriptorCount = 1;
	descriptorWrites.pBufferInfo     = descBufferInfo;

	writes.push_back ( descriptorWrites );

	vkUpdateDescriptorSets ( device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr );

		// fill command buffer with compute dispatch command
	VkCommandBufferBeginInfo beginInfo = {};

	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if ( vkBeginCommandBuffer ( commandBuffer, &beginInfo ) != VK_SUCCESS )
		fatal () << "VulkanWindow: failed to begin recording command buffer!";

		// bind compute pipeline, descriptor set and issue dispatch command
	vkCmdBindPipeline       ( commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline );
	vkCmdBindDescriptorSets ( commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, &set, 0, nullptr );
	vkCmdDispatch           ( commandBuffer, 1000, 1, 1 );
	vkEndCommandBuffer      ( commandBuffer );

	VkSubmitInfo			submitInfo        = {};
	VkPipelineStageFlags	computeWaitStages = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

		// we will need fence to wait till results will be ready,
		// so we create a fence
	VkFence					fence             = VK_NULL_HANDLE;
	VkFenceCreateInfo		fenceCreateInfo   = {};

	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

	if ( vkCreateFence ( device, &fenceCreateInfo, nullptr, &fence ) != VK_SUCCESS )
		fatal () << "Semaphore: error creating" << Log::endl;

		// start submitting out command buffer to compute queue
	submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount   = 0;
	submitInfo.pWaitSemaphores      = 0;
	submitInfo.pWaitDstStageMask    = nullptr;
	submitInfo.commandBufferCount   = 1;
	submitInfo.pCommandBuffers      = &commandBuffer;
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores    = nullptr;

	vkResetFences ( device, 1, &fence );		// ????

	if ( vkQueueSubmit ( computeQueue, 1, &submitInfo, fence ) != VK_SUCCESS )
		fatal () << "failed to submit draw command buffer!";

		// wait till results will be ready 
	if ( vkWaitForFences ( device, 1, &fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT  ) != VK_SUCCESS )
		fatal () << "Waiting for fence failed" << Log::endl;

		// copy them back to CPUs memory
	std::vector<float>	outData ( 1024 );

	vkMapMemory   ( device, memory, 0, size, 0, &data );
	memcpy        ( outData.data (), data,  size );
	vkUnmapMemory ( device, memory );

		// free all allocated resources
	vkDestroyBuffer              ( device, buffer, nullptr );
	vkFreeMemory                 ( device, memory, nullptr );
	vkDestroyPipeline            ( device, pipeline,       nullptr );
	vkDestroyPipelineLayout      ( device, pipelineLayout, nullptr );
	vkDestroyShaderModule        ( device, shader, nullptr );
	vkDestroyFence               ( device, fence, nullptr );
	vkFreeCommandBuffers         ( device, commandPool, 1, &commandBuffer );
	vkDestroyCommandPool         ( device, commandPool, nullptr);
	vkDestroyDescriptorSetLayout ( device, descriptorSetLayout, nullptr );
	vkDestroyDescriptorPool      ( device, descriptorPool, 0 );
	vkDestroyDevice              ( device, nullptr );

	destroyDebugUtilsMessengerEXT ( instance, debugMessenger, nullptr );

	vkDestroyInstance ( instance, nullptr );

	return 0;
}
