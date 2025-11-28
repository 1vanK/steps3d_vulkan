// Файл изменён. Изменения помечены 1vanK

//#include	<vulkan/vulkan.h>
#define	VMA_IMPLEMENTATION
#include	<vk_mem_alloc.h>
#include	"Log.h"
#include	"Data.h"

#include	<iostream>
#include	<vector>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include	"stb_image.h"
#include	"stb_image_write.h"

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
	// here just check for graphics support
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
		if ( computeFamily == -1 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT )
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
	fatal () << "GpuMemory: failed to find suitable memory type! " << properties << std::endl;

	return 0;		// we won't get here, but compiler complains about returning no value
}
void	transitionLayout ( VkCommandBuffer commandBuffer, VkImage image, VkPipelineStageFlags sourceStage, VkPipelineStageFlags destinationStage, 
	VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageLayout oldLayout, VkImageLayout newLayout )
{
	VkImageMemoryBarrier	barrier       = {};

	barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout                       = oldLayout;
	barrier.newLayout                       = newLayout;
	barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
	barrier.image                           = image;
	barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel   = 0;
	barrier.subresourceRange.levelCount     = 1;	// XXX
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount     = 1;	// XXX
	barrier.srcAccessMask                   = srcAccessMask;
	barrier.dstAccessMask                   = dstAccessMask;

	vkCmdPipelineBarrier (
		commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier );
}

void	saveScreenshot ( VkDevice device, VkPhysicalDevice physicalDevice,  VkQueue queue, VkCommandPool commandPool, VkImage srcImage, int width, int height )
{
	VkImage				image;
	VkImageCreateInfo	imageInfo = {};

	imageInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType     = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width  = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth  = 1;
	imageInfo.mipLevels     = 1;
	imageInfo.arrayLayers   = 1;
	imageInfo.format        = VK_FORMAT_R8G8B8A8_UNORM;
	imageInfo.tiling        = VK_IMAGE_TILING_LINEAR;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED ;
	imageInfo.usage         = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	imageInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;

	if ( vkCreateImage ( device, &imageInfo, nullptr, &image ) != VK_SUCCESS ) 
		fatal () << "Image: Cannot create image";

		// allocate memory with required properties
	VkMemoryRequirements memRequirements;
	VkMemoryAllocateInfo 	allocInfo  = {};
	VkDeviceMemory			memory     = VK_NULL_HANDLE;
	VkMemoryPropertyFlags	properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ;

	vkGetImageMemoryRequirements ( device, image, &memRequirements );

	allocInfo.sType                = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize       = memRequirements.size;
	allocInfo.memoryTypeIndex      = findMemoryType ( physicalDevice, memRequirements.memoryTypeBits, properties );

	if ( vkAllocateMemory ( device, &allocInfo, nullptr, &memory ) != VK_SUCCESS )
		fatal () << "Cannot allocate memory for buffer" << std::endl;

	vkBindImageMemory ( device, image, memory, 0 );

	VkCommandBuffer				commandBuffer;
	VkCommandBufferAllocateInfo	cbAllocInfo = {};

	cbAllocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cbAllocInfo.commandPool        = commandPool;
	cbAllocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cbAllocInfo.commandBufferCount = 1;

	if ( vkAllocateCommandBuffers ( device, &cbAllocInfo, &commandBuffer ) != VK_SUCCESS )
		fatal () << "failed to allocate command buffers!" << std::endl;

	VkCommandBufferBeginInfo	beginInfo = {};

	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer ( commandBuffer, &beginInfo );

	transitionLayout ( commandBuffer, image,    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL  );
	transitionLayout ( commandBuffer, srcImage, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_MEMORY_READ_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL );

		// use image copy (requires us to manually flip components)
	VkImageCopy imageCopyRegion               = {};
	imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageCopyRegion.srcSubresource.layerCount = 1;
	imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageCopyRegion.dstSubresource.layerCount = 1;
	imageCopyRegion.extent.width              = width;
	imageCopyRegion.extent.height             = height;
	imageCopyRegion.extent.depth              = 1;

		// Issue the copy command
	vkCmdCopyImage(
			commandBuffer,
			srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &imageCopyRegion );

		//image.transitionLayout  ( cmd, image.getFormat (), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL );
		//Image::transitionLayout ( cmd, srcImage, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 
		//	VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_MEMORY_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR );

	vkEndCommandBuffer ( commandBuffer );

	VkSubmitInfo submitInfo = {};

	submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers    = &commandBuffer;

	VkFence				fence               = VK_NULL_HANDLE;
	VkFenceCreateInfo fenceInfo = {};

	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

	vkCreateFence ( device, &fenceInfo, nullptr, &fence );

		// Submit to the queue
	vkQueueSubmit ( queue, 1, &submitInfo, fence );

		// Wait for the fence to signal that command buffer has finished executing
	vkWaitForFences ( device, 1, &fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT );
	vkDestroyFence  ( device, fence, nullptr );
	vkFreeCommandBuffers ( device, commandPool, 1, &commandBuffer );

	VkImageSubresource	subResource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0 };
	VkSubresourceLayout	subResourceLayout;

	vkGetImageSubresourceLayout ( device, image, &subResource, &subResourceLayout );

	const uint8_t * data;
	
	if ( vkMapMemory ( device, memory, 0, VK_WHOLE_SIZE, 0, (void **)&data ) != VK_SUCCESS )
		fatal () << "Error mapping screenshot image memory" << std::endl;

	stbi_write_png ( "screenshot-2.png", width, height, 4, data, 4*width );

	vkDestroyImage               ( device, image,          nullptr );
	vkFreeMemory                 ( device, memory,         nullptr );
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
	int	graphicsFamily  = -1;

	for ( const auto& dev : physDevices )
		if ( (graphicsFamily = isDeviceSuitable ( dev ) ) > -1 )	
		{
			physicalDevice = dev;
			break;
		}

	if ( physicalDevice == VK_NULL_HANDLE )
		fatal () << "VulkanWindow: failed to find a suitable GPU!" << std::endl;

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
	VkQueue					graphicsQueue   = VK_NULL_HANDLE;

		// we will require only one compute queue
	queueCreateInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = graphicsFamily;
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
		// now create device
	if ( vkCreateDevice ( physicalDevice, &devCreateInfo, nullptr, &device ) != VK_SUCCESS )
		fatal () << "VulknaWindow: failed to create logical device!" << std::endl;

		// get compute queue from created logical device
	vkGetDeviceQueue ( device, graphicsFamily, 0, &graphicsQueue  );

		// create command pool for our (logical) device
	VkCommandPoolCreateInfo	poolInfo    = {};
	VkCommandPool			commandPool = VK_NULL_HANDLE;

	poolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = graphicsFamily;

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

	VmaAllocator			allocator;
	VmaAllocatorCreateInfo	allocatorCreateInfo = {};

	allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_0;
	allocatorCreateInfo.physicalDevice   = physicalDevice;
	allocatorCreateInfo.device           = device;
	allocatorCreateInfo.instance         = instance;

	if ( vmaCreateAllocator ( &allocatorCreateInfo, &allocator ) != VK_SUCCESS )
		fatal () << "vmasCreateAllocator failure" << std::endl;

		// create image for rendering into
	VkImageCreateInfo		imageInfo       = {};
	VkImage					image           = VK_NULL_HANDLE;
	VmaAllocationCreateInfo	allocCreateInfo = {};
	VmaAllocation			allocation      = VK_NULL_HANDLE;

	imageInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType     = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width  = 512;
	imageInfo.extent.height = 512;
	imageInfo.extent.depth  = 1;
	imageInfo.mipLevels     = 1;
	imageInfo.arrayLayers   = 1;
	imageInfo.format        = VK_FORMAT_R8G8B8A8_UNORM;
	imageInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage         = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |  VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	imageInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;

	allocCreateInfo.usage    = VMA_MEMORY_USAGE_AUTO;
	allocCreateInfo.flags    = 0;
	allocCreateInfo.priority = 1.0f;

	if ( vmaCreateImage ( allocator, &imageInfo, &allocCreateInfo, &image, &allocation, nullptr ) != VK_SUCCESS ) 
		fatal () << "vmaImage: Cannot create image" << std::endl;

		// create imageview for the image
	VkImageView 			imageView = VK_NULL_HANDLE;
	VkImageViewCreateInfo	viewInfo  = {};
		
	viewInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image                           = image;
	viewInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format                          = VK_FORMAT_R8G8B8A8_UNORM;
	viewInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel   = 0;
	viewInfo.subresourceRange.levelCount     = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount     = 1;

	if ( vkCreateImageView ( device, &viewInfo, nullptr, &imageView ) != VK_SUCCESS )
		fatal () << "failed to create texture image view!" << std::endl;

		// create renderpass for framebuffer creation
	VkAttachmentDescription colorAttachment{};

	colorAttachment.format         = VK_FORMAT_R8G8B8A8_UNORM;
	colorAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;

	// 1vanK: Так нельзя, потому что для VK_IMAGE_LAYOUT_PRESENT_SRC_KHR требуется VK_KHR_swapchain, которого у нас нет.
	// При копировании в память CPU надо использовать VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
	colorAttachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef = {};

	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};

	subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments    = &colorAttachmentRef;

	VkRenderPass			renderPass;
	VkRenderPassCreateInfo	renderPassInfo = {};

	renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments    = &colorAttachment;
	renderPassInfo.subpassCount    = 1;
	renderPassInfo.pSubpasses      = &subpass;

	if ( vkCreateRenderPass ( device, &renderPassInfo, nullptr, &renderPass ) != VK_SUCCESS )
		fatal () << "failed to create render pass!" << std::endl;

		// create framebuffer for rendering into
	VkFramebuffer			framebuffer     = VK_NULL_HANDLE;
	VkImageView				attachments []  = { imageView };
	VkFramebufferCreateInfo	framebufferInfo = {};
			
	framebufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass      = renderPass;
	framebufferInfo.attachmentCount = 1;
	framebufferInfo.pAttachments    = attachments;
	framebufferInfo.width           = 512;
	framebufferInfo.height          = 512;
	framebufferInfo.layers          = 1;

	if ( vkCreateFramebuffer ( device, &framebufferInfo, nullptr, &framebuffer ) != VK_SUCCESS )
		fatal () << "failed to create framebuffer!" << std::endl;

		// load shaders from .spv files
	VkShaderModule				vertShader = VK_NULL_HANDLE;
	Data						vertSource ( "shaders/simple.vert.spv" );
	VkShaderModuleCreateInfo	moduleCreateInfo = {};

	moduleCreateInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	moduleCreateInfo.codeSize = vertSource.getLength ();
	moduleCreateInfo.pCode    = reinterpret_cast<const uint32_t*>( vertSource.getPtr () );

	if ( vkCreateShaderModule ( device, &moduleCreateInfo, nullptr, &vertShader ) != VK_SUCCESS )
		fatal () << "Failed to create shader module! " << std::endl;

	VkShaderModule				fragShader = VK_NULL_HANDLE;
	Data						fragSource ( "shaders/simple.frag.spv" );

	moduleCreateInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	moduleCreateInfo.codeSize = fragSource.getLength ();
	moduleCreateInfo.pCode    = reinterpret_cast<const uint32_t*>( fragSource.getPtr () );

	if ( vkCreateShaderModule ( device, &moduleCreateInfo, nullptr, &fragShader ) != VK_SUCCESS )
		fatal () << "Failed to create shader module! " << std::endl;

	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};

	vertShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage  = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShader;
	vertShaderStageInfo.pName  = "main";
	fragShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShader;
	fragShaderStageInfo.pName  = "main";

	VkPipelineShaderStageCreateInfo	shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		// setup vertex input
	VkPipelineVertexInputStateCreateInfo	vertexInputInfo = {};

	vertexInputInfo.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount   = 0;
	vertexInputInfo.pVertexBindingDescriptions      = nullptr; // Optional
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions    = nullptr; // Optional

		// setup vertex assembly
	VkPipelineInputAssemblyStateCreateInfo	inputAssembly = {};

	inputAssembly.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport	viewport = {};
	VkRect2D	scissor  = {};

	viewport.x            = 0.0f;
	viewport.y            = 0.0f;
	viewport.width        = 512.0f;
	viewport.height       = 512.0f;
	viewport.minDepth     = 0.0f;
	viewport.maxDepth     = 1.0f;
	scissor.offset        = {0, 0};
	scissor.extent.width  = 512;
	scissor.extent.height = 512;

	VkPipelineViewportStateCreateInfo	viewportState = {};

	viewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports    = &viewport;
	viewportState.scissorCount  = 1;
	viewportState.pScissors     = &scissor;

		// setup rasterizer
	VkPipelineRasterizationStateCreateInfo	rasterizer = {};

	rasterizer.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable        = VK_FALSE;
	rasterizer.polygonMode             = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth               = 1.0f;
	rasterizer.cullMode                = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace               = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable         = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;			// Optional
	rasterizer.depthBiasClamp          = 0.0f;			// Optional
	rasterizer.depthBiasSlopeFactor    = 0.0f;			// Optional

		// setup multisampling
	VkPipelineMultisampleStateCreateInfo	multisampling = {};

	multisampling.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable   = VK_FALSE;
	multisampling.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading      = 1.0f;			// Optional
	multisampling.pSampleMask           = nullptr;		// Optional
	multisampling.alphaToCoverageEnable = VK_FALSE;		// Optional
	multisampling.alphaToOneEnable      = VK_FALSE;		// Optional

		// setup color blending
	VkPipelineColorBlendAttachmentState	colorBlendAttachment = {};

	colorBlendAttachment.colorWriteMask      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable         = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;		// Optional
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;	// Optional
	colorBlendAttachment.colorBlendOp        = VK_BLEND_OP_ADD;			// Optional
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;		// Optional
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;	// Optional
	colorBlendAttachment.alphaBlendOp        = VK_BLEND_OP_ADD;			// Optional

	VkPipelineColorBlendStateCreateInfo	colorBlending = {};

	colorBlending.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable     = VK_FALSE;
	colorBlending.logicOp           = VK_LOGIC_OP_COPY;				// Optional
	colorBlending.attachmentCount   = 1;
	colorBlending.pAttachments      = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;							// Optional
	colorBlending.blendConstants[1] = 0.0f;							// Optional
	colorBlending.blendConstants[2] = 0.0f;							// Optional
	colorBlending.blendConstants[3] = 0.0f;							// Optional

		// setup pipeline layout
	VkPipelineLayout			pipelineLayout;
	VkPipelineLayoutCreateInfo	pipelineLayoutInfo = {};

	pipelineLayoutInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount         = 0;				// Optional
	pipelineLayoutInfo.pSetLayouts            = nullptr;		// Optional
	pipelineLayoutInfo.pushConstantRangeCount = 0;				// Optional
	pipelineLayoutInfo.pPushConstantRanges    = nullptr;		// Optional

	if ( vkCreatePipelineLayout ( device, &pipelineLayoutInfo, nullptr, &pipelineLayout ) != VK_SUCCESS )
		fatal () << "failed to create pipeline layout!" << std::endl;

	VkGraphicsPipelineCreateInfo	pipelineInfo = {};
	VkPipeline						pipeline;

	pipelineInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount          = 2;
	pipelineInfo.pStages             = shaderStages;
	pipelineInfo.pVertexInputState   = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState      = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState   = &multisampling;
	pipelineInfo.pDepthStencilState  = nullptr;				// Optional
	pipelineInfo.pColorBlendState    = &colorBlending;
	//pipelineInfo.pDynamicState     = &dynamicState;
	pipelineInfo.layout              = pipelineLayout;
	pipelineInfo.renderPass          = renderPass;
	pipelineInfo.subpass             = 0;

	if ( vkCreateGraphicsPipelines ( device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline ) != VK_SUCCESS )
		fatal () << "failed to create graphics pipeline!" << std::endl;

	VkCommandBuffer				commandBuffer;
	VkCommandBufferAllocateInfo	cbAllocInfo = {};

	cbAllocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cbAllocInfo.commandPool        = commandPool;
	cbAllocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cbAllocInfo.commandBufferCount = 1;

	if ( vkAllocateCommandBuffers ( device, &cbAllocInfo, &commandBuffer ) != VK_SUCCESS )
		fatal () << "failed to allocate command buffers!" << std::endl;
	
	VkCommandBufferBeginInfo	beginInfo = {};

	beginInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags            = 0;				// Optional
	beginInfo.pInheritanceInfo = nullptr;		// Optional

	if ( vkBeginCommandBuffer ( commandBuffer, &beginInfo ) != VK_SUCCESS )
		fatal () << "failed to begin recording command buffer!" << std::endl;
	
	VkRenderPassBeginInfo	rpInfo = {};
	VkClearValue			clearColor     = {{{0.0f, 0.0f, 0.0f, 1.0f}}};

	rpInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpInfo.renderPass        = renderPass;
	rpInfo.framebuffer       = framebuffer;
	rpInfo.renderArea.offset = { 0, 0 };
	rpInfo.renderArea.extent = { 512, 512 };
	rpInfo.clearValueCount   = 1;
	rpInfo.pClearValues      = &clearColor;

	vkCmdBeginRenderPass ( commandBuffer, &rpInfo, VK_SUBPASS_CONTENTS_INLINE );
	vkCmdBindPipeline    ( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline );
	vkCmdDraw            ( commandBuffer, 3, 1, 0, 0 );
	vkCmdEndRenderPass   ( commandBuffer );

	if ( vkEndCommandBuffer ( commandBuffer ) != VK_SUCCESS )
		fatal () << "failed to record command buffer!" << std::endl;
	
	VkFence					fence             = VK_NULL_HANDLE;
	VkFenceCreateInfo		fenceCreateInfo   = {};

	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

	if ( vkCreateFence ( device, &fenceCreateInfo, nullptr, &fence ) != VK_SUCCESS )
		fatal () << "Semaphore: error creating" << std::endl;

	VkSubmitInfo	submitInfo = {};

	submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount   = 0;
	submitInfo.pWaitSemaphores      = nullptr;
	submitInfo.pWaitDstStageMask    = 0;
	submitInfo.commandBufferCount   = 1;
	submitInfo.pCommandBuffers      = &commandBuffer;
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores    = nullptr;

	vkResetFences ( device, 1, &fence );		

	if ( vkQueueSubmit ( graphicsQueue, 1, &submitInfo, fence ) != VK_SUCCESS )
		fatal () << "failed to submit command buffer" << std::endl;

		// wait till results will be ready 
	if ( vkWaitForFences ( device, 1, &fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT  ) != VK_SUCCESS )
		fatal () << "Waiting for fence failed" << Log::endl;


	saveScreenshot ( device, physicalDevice,  graphicsQueue, commandPool, image, 512, 512 );


		// free all allocated resources
	vkFreeCommandBuffers         ( device, commandPool, 1, &commandBuffer );
	vkDestroyFence               ( device, fence,          nullptr );
	vkDestroyFramebuffer         ( device, framebuffer,    nullptr );
	vkDestroyImageView           ( device, imageView,      nullptr );
	//vkDestroyImage               ( device, image,          nullptr );
	vmaDestroyImage              ( allocator, image,       allocation );
	vkDestroyRenderPass          ( device, renderPass,     nullptr );
	//vkFreeMemory                 ( device, memory,         nullptr );
	vkDestroyPipeline            ( device, pipeline,       nullptr );
	vkDestroyPipelineLayout      ( device, pipelineLayout, nullptr );
	vkDestroyShaderModule        ( device, vertShader,     nullptr );
	vkDestroyShaderModule        ( device, fragShader,     nullptr );
	vkDestroyCommandPool         ( device, commandPool,    nullptr);
	vmaDestroyAllocator          ( allocator );
	vkDestroyDevice              ( device,                 nullptr );

	destroyDebugUtilsMessengerEXT ( instance, debugMessenger, nullptr );

	vkDestroyInstance ( instance, nullptr );

	return 0;
}
