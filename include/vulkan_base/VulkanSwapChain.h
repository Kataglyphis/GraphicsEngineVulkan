#pragma once
#include "VulkanDevice.h"
#include "Window.h"

#include "VulkanDebug.h"
#include "SwapChainImage.h"

class VulkanSwapChain
{
public:

	VulkanSwapChain();

	void					initVulkanContext(	VulkanDevice* device,
												Window* window,
												const VkSurfaceKHR& surface);

	const VkSwapchainKHR&	getSwapChain() const { return swapchain; };
	uint32_t				getNumberSwapChainImages() const { return static_cast<uint32_t>(swap_chain_images.size()); };
	const VkExtent2D&		getSwapChainExtent() const { return swap_chain_extent; };
	const VkFormat&			getSwapChainFormat() const { return swap_chain_image_format; };
	SwapChainImage&			getSwapChainImage(uint32_t index) { return swap_chain_images[index]; };

	~VulkanSwapChain();

private:

	VulkanDevice*	device;
	Window*			window;

	VkSwapchainKHR					swapchain;
	std::vector<SwapChainImage>		swap_chain_images;
	VkFormat						swap_chain_image_format;
	VkExtent2D						swap_chain_extent;

	VkSurfaceFormatKHR	choose_best_surface_format(const std::vector<VkSurfaceFormatKHR>& formats);
	VkPresentModeKHR	choose_best_presentation_mode(const std::vector<VkPresentModeKHR>& presentation_modes);
	VkExtent2D			choose_swap_extent(const VkSurfaceCapabilitiesKHR& surface_capabilities);

};

