#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <iostream>
#include <fstream>

#define VK_ASSERT(val)\
			if (val != VK_SUCCESS) {\
				throw std::runtime_error("Vulkan Error!");\
			}

namespace vkUtils {

	namespace queueHandler {
		bool isInitialized();
		void init(std::vector<VkQueue>& queues);

		VkQueue& getQueue();
	}

	std::vector<VkPhysicalDevice> getAllPhysicalDevices(VkInstance instance);

	int getQueueCount(VkPhysicalDevice physicalDevice, int familyIndex);

	bool checkSurfaceSupport(const VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface);

	std::vector<VkPresentModeKHR> getSupportedSurfacePresentModes(VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface);

	std::vector<char> readFile(const char* filename);

	//Allocates given amount of command buffers at the position of the pointer
	void allocateCommandBuffers(VkDevice& device, VkCommandPool& commandPool, uint32_t commandBufferCount, VkCommandBuffer* commandBuffers);

	void beginCommandBuffer(VkCommandBuffer& commandBuffer, VkCommandBufferUsageFlags usageFlags);

	void endCommandBuffer(VkCommandBuffer& commandBuffer);void endCommandBuffer(VkCommandBuffer& commandBuffer);

	void submitCommandBuffer(VkCommandBuffer& commandBuffer, VkQueue& queue);

	void freeCommandBuffers(VkDevice& device, VkCommandPool& commandPool, uint32_t commandBufferCount, VkCommandBuffer* commandBuffers);

	uint32_t findMemoryTypeIndex(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

	void createBuffer(VkDevice& device, VkPhysicalDevice& physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceMemory& deviceMemory, VkBuffer& buffer);

	void copyBuffer(VkDevice& device, VkCommandPool& commandPool, VkQueue& queue, VkBuffer& src, VkBuffer& dst, VkDeviceSize size);

	void updateBuffer(VkDevice& device, VkDeviceMemory& bufferMemory, VkDeviceSize bufferSize, const void* data);

	void uploadBuffer(VkDevice& device, VkPhysicalDevice& physicalDevice, VkCommandPool& commandPool, VkQueue& queue, VkDeviceSize bufferSize, void* data, VkBuffer& buffer);
};