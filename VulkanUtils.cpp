#include "VulkanUtils.h"

namespace vkUtils {

    std::vector<VkPhysicalDevice> getAllPhysicalDevices(VkInstance instance) {
        uint32_t amountOfPhysicalDevices;
        vkEnumeratePhysicalDevices(instance, &amountOfPhysicalDevices, nullptr);
        std::vector<VkPhysicalDevice> physicalDevices(amountOfPhysicalDevices);
        vkEnumeratePhysicalDevices(instance, &amountOfPhysicalDevices, physicalDevices.data());
        return physicalDevices;
    }

    int getQueueCount(VkPhysicalDevice physicalDevice, int familyIndex) {
        uint32_t amountOfQueueFamilies;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &amountOfQueueFamilies, nullptr);
        if (familyIndex > amountOfQueueFamilies) throw std::logic_error("Family Index out of range");
        VkQueueFamilyProperties* queueFamilyProperties = new VkQueueFamilyProperties[amountOfQueueFamilies];
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &amountOfQueueFamilies, queueFamilyProperties);

        int queueCount = queueFamilyProperties[familyIndex].queueCount;

        delete[] queueFamilyProperties;

        return queueCount;
    }

    bool checkSurfaceSupport(const VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface) {
        VkBool32 surfaceSupport = false;
        VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, 0, surface, &surfaceSupport);
        VK_ASSERT(result);

        return surfaceSupport;
    }

    std::vector<VkPresentModeKHR> getSupportedSurfacePresentModes(VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface) {
        uint32_t amountOfValidSurfacePresentModes;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &amountOfValidSurfacePresentModes, nullptr);
        std::vector<VkPresentModeKHR> validSurfacePresentModes(amountOfValidSurfacePresentModes);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &amountOfValidSurfacePresentModes, validSurfacePresentModes.data());

        return validSurfacePresentModes;
    }

    std::vector<char> readFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary | std::ios::ate);

        if (file) {
            size_t fileSize = (size_t)file.tellg();
            std::vector<char> fileBuffer(fileSize);
            file.seekg(0);
            file.read(fileBuffer.data(), fileSize);
            file.close();
            return fileBuffer;
        }
        else {
            throw std::runtime_error("Failed to open file");
        }
    }

    uint32_t findMemoryTypeIndex(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &physicalDeviceMemoryProperties);
        for (int i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount; i++) {
            if (typeFilter & (1 << i) && (physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw std::runtime_error("Found no correct memory type!");
    }

    //Allocates given amount of command buffers at the position of the pointer
    void allocateCommandBuffers(VkDevice& device, VkCommandPool& commandPool, uint32_t commandBufferCount, VkCommandBuffer* commandBuffers) {
        VkCommandBufferAllocateInfo allocateInfo;
        allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocateInfo.pNext = nullptr;
        allocateInfo.commandPool = commandPool;
        allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocateInfo.commandBufferCount = commandBufferCount;

        VkResult result = vkAllocateCommandBuffers(device, &allocateInfo, commandBuffers);
        VK_ASSERT(result);
    }

    void beginCommandBuffer(VkCommandBuffer& commandBuffer, VkCommandBufferUsageFlags usageFlags) {
        VkCommandBufferBeginInfo beginInfo;
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.pNext = nullptr;
        beginInfo.flags = usageFlags;
        beginInfo.pInheritanceInfo = nullptr;

        VkResult result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
        VK_ASSERT(result);
    }

    void endCommandBuffer(VkCommandBuffer& commandBuffer) {
        VkResult result = vkEndCommandBuffer(commandBuffer);
        VK_ASSERT(result);
    }

    void submitCommandBuffer(VkCommandBuffer& commandBuffer, VkQueue& queue) {
        VkSubmitInfo submitInfo;
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pNext = nullptr;
        submitInfo.waitSemaphoreCount = 0;
        submitInfo.pWaitSemaphores = nullptr;
        submitInfo.pWaitDstStageMask = nullptr;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;
        submitInfo.signalSemaphoreCount = 0;
        submitInfo.pSignalSemaphores = nullptr;

        VkResult result = vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
        VK_ASSERT(result);

        result = vkQueueWaitIdle(queue);
        VK_ASSERT(result);
    }

    void freeCommandBuffers(VkDevice& device, VkCommandPool& commandPool, uint32_t commandBufferCount, VkCommandBuffer* commandBuffers) {
        vkFreeCommandBuffers(device, commandPool, commandBufferCount, commandBuffers);
    }

    void createBuffer(VkDevice& device, VkPhysicalDevice& physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceMemory& deviceMemory, VkBuffer& buffer) {
        if (size <= 0) throw std::runtime_error("Trying to create buffer with size zero!");
        
        VkBufferCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.size = size;
        createInfo.usage = usage;
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;// needed only when VK_SHARING_MODE_CONCURRENT
        createInfo.pQueueFamilyIndices = nullptr;

        VkResult result = vkCreateBuffer(device, &createInfo, nullptr, &buffer);
        VK_ASSERT(result);

        VkMemoryRequirements memoryRequirements;
        vkGetBufferMemoryRequirements(device, buffer, &memoryRequirements);

        VkMemoryAllocateInfo allocateInfo;
        allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocateInfo.pNext = nullptr;
        allocateInfo.allocationSize = memoryRequirements.size;
        allocateInfo.memoryTypeIndex = findMemoryTypeIndex(physicalDevice, memoryRequirements.memoryTypeBits, memoryPropertyFlags);

        result = vkAllocateMemory(device, &allocateInfo, nullptr, &deviceMemory);
        VK_ASSERT(result);

        vkBindBufferMemory(device, buffer, deviceMemory, 0);
    }

    void copyBuffer(VkDevice& device, VkCommandPool& commandPool, VkQueue& queue, VkBuffer& src, VkBuffer& dst, VkDeviceSize size) {
        VkCommandBuffer commandBuffer;
        allocateCommandBuffers(device, commandPool, 1, &commandBuffer);
        beginCommandBuffer(commandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        VkBufferCopy bufferCopy;
        bufferCopy.srcOffset = 0;
        bufferCopy.dstOffset = 0;
        bufferCopy.size = size;
        vkCmdCopyBuffer(commandBuffer, src, dst, 1, &bufferCopy);

        endCommandBuffer(commandBuffer);
        submitCommandBuffer(commandBuffer, queue);
        freeCommandBuffers(device, commandPool, 1, &commandBuffer);
    }

    void updateBuffer(VkDevice& device, VkDeviceMemory& bufferMemory, VkDeviceSize bufferSize, const void* data) {
        void* rawData;
        vkMapMemory(device, bufferMemory, 0, bufferSize, 0, &rawData);
        memcpy(rawData, data, bufferSize);
        vkUnmapMemory(device, bufferMemory);
    }
    
    void uploadBuffer(VkDevice& device, VkPhysicalDevice& physicalDevice, VkCommandPool& commandPool, VkQueue& queue, VkDeviceSize bufferSize, void* data, VkBuffer& buffer) {
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(device, physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBufferMemory, stagingBuffer);

        updateBuffer(device, stagingBufferMemory, bufferSize, data);

        copyBuffer(device, commandPool, queue, stagingBuffer, buffer, bufferSize);

        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
    }
};

