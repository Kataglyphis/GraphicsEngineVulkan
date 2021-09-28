#include "VulkanRenderer.h"

VulkanRenderer::VulkanRenderer()
{
}

int VulkanRenderer::init(std::shared_ptr<MyWindow> window)
{
	this->window = window;
	
	try {
		create_instance();
	}
	catch (const std::runtime_error &e) {
		printf("ERROR: %s\n", e.what());
		return EXIT_FAILURE;
	}

	return 0;
}

VulkanRenderer::~VulkanRenderer()
{
}

void VulkanRenderer::create_instance()
{

}
