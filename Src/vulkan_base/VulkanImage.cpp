#include "VulkanImage.h"
#include "Utilities.h"

VulkanImage::VulkanImage()
{
}

void VulkanImage::create(	VulkanDevice* device, 
							uint32_t width, uint32_t height, 
							uint32_t mip_levels, 
							VkFormat format, VkImageTiling tiling, 
							VkImageUsageFlags use_flags, VkMemoryPropertyFlags prop_flags)
{
	this->device = device;
	// CREATE image
	// image creation info
	VkImageCreateInfo image_create_info{};
	image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_create_info.imageType = VK_IMAGE_TYPE_2D;						// type of image (1D, 2D, 3D)
	image_create_info.extent.width = width;								// width if image extent
	image_create_info.extent.height = height;							// height if image extent
	image_create_info.extent.depth = 1;									// height if image extent
	image_create_info.mipLevels = mip_levels;							// number of mipmap levels 
	image_create_info.arrayLayers = 1;									// number of levels in image array
	image_create_info.format = format;									// format type of image 
	image_create_info.tiling = tiling;									// tiling of image ("arranged" for optimal reading)
	image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;		// layout of image data on creation 
	image_create_info.usage = use_flags;								// bit flags defining what image will be used for
	image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;					// number of samples for multisampling 
	image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;			// whether image can be shared between queues

	VkResult result = vkCreateImage(device->getLogicalDevice(), &image_create_info, nullptr, &image);
	ASSERT_VULKAN(result, "Failed to create an image!")

	// CREATE memory for image
	// get memory requirements for a type of image
	VkMemoryRequirements memory_requirements;
	vkGetImageMemoryRequirements(device->getLogicalDevice(), image, &memory_requirements);

	// allocate memory using image requirements and user defined properties
	VkMemoryAllocateInfo memory_alloc_info{};
	memory_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memory_alloc_info.allocationSize = memory_requirements.size;
	memory_alloc_info.memoryTypeIndex = find_memory_type_index(	device->getPhysicalDevice(), 
																memory_requirements.memoryTypeBits,
																prop_flags);

	result = vkAllocateMemory(device->getLogicalDevice(), &memory_alloc_info, nullptr, &imageMemory);
	ASSERT_VULKAN(result, "Failed to allocate memory!")

	// connect memory to image
	vkBindImageMemory(device->getLogicalDevice(), image, imageMemory, 0);

}

void VulkanImage::setImage(VkImage image)
{
	this->image = image;
}

void VulkanImage::cleanUp()
{
	vkDestroyImage(device->getLogicalDevice(), image, nullptr);
	vkFreeMemory(device->getLogicalDevice(), imageMemory, nullptr);
}

VulkanImage::~VulkanImage()
{
}
