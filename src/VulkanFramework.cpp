#include "VulkanFramework.h"

#include "VulkanUtils.h"

namespace vk
{
	VkInstance instance;
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	size_t queueFamily;
	std::vector<VkQueue> queues;

	VkCommandPool commandPool;

	void createInstance(VkInstance &instance, std::vector<const char *> &enabledLayers, std::vector<const char *> &enabledExtensions, const char *applicationName)
	{
		VkApplicationInfo applicationInfo;
		applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		applicationInfo.pNext = nullptr;
		applicationInfo.pApplicationName = applicationName;
		applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		// TODO specify engine name
		applicationInfo.pEngineName = "RTXEngine";
		applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		applicationInfo.apiVersion = VK_API_VERSION_1_3;

		VkInstanceCreateInfo instanceCreateInfo;
		instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceCreateInfo.pNext = nullptr;
		instanceCreateInfo.flags = 0;
		instanceCreateInfo.pApplicationInfo = &applicationInfo;
		instanceCreateInfo.enabledLayerCount = enabledLayers.size();
		instanceCreateInfo.ppEnabledLayerNames = enabledLayers.data();
		instanceCreateInfo.enabledExtensionCount = enabledExtensions.size();
		instanceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();

		VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &instance);
		VK_ASSERT(result);
	}

	void createLogicalDevice(const VkPhysicalDevice &physicalDevice, VkDevice &device, std::vector<const char *> &enabledLayers, std::vector<const char *> &enabledExtensions, VkPhysicalDeviceFeatures usedFeatures)
	{
		VkDeviceQueueCreateInfo deviceQueueCreateInfo;
		deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		deviceQueueCreateInfo.pNext = nullptr;
		deviceQueueCreateInfo.flags = 0;
		uint32_t familyIndex = 0;
		deviceQueueCreateInfo.queueFamilyIndex = familyIndex; // TODO Choose correct famlily index automatically

		uint32_t queueCreateCount = VK_PREFERED_AMOUNT_OF_QUEUES; // Set and Validate the amount of created queues
		uint32_t queueCount = vkUtils::getQueueCount(vk::physicalDevice, familyIndex);
		if (queueCount < queueCreateCount)
		{
			queueCreateCount = queueCount;
		}
		vk::queues.resize(queueCreateCount);
		deviceQueueCreateInfo.queueCount = queueCreateCount;

		std::vector<float> prios(queueCount);
		for (size_t i = 0; i < prios.size(); i++)
			prios[i] = 1.0f;
		deviceQueueCreateInfo.pQueuePriorities = prios.data();

		VkDeviceCreateInfo deviceCreateInfo;
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.pNext = nullptr;
		deviceCreateInfo.flags = 0;
		deviceCreateInfo.queueCreateInfoCount = 1;
		deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
		deviceCreateInfo.enabledLayerCount = enabledLayers.size();
		deviceCreateInfo.ppEnabledLayerNames = enabledLayers.data();
		deviceCreateInfo.enabledExtensionCount = enabledExtensions.size();
		deviceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
		deviceCreateInfo.pEnabledFeatures = &usedFeatures;

		VkResult result = vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device);
		VK_ASSERT(result);
	}

	void createSemaphore(VkSemaphore *semaphore)
	{
		VkSemaphoreCreateInfo createInfo{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, nullptr, 0};
		vkCreateSemaphore(vk::device, &createInfo, nullptr, semaphore);
	}
	void destroySemaphore(VkSemaphore semaphore)
	{
		vkDestroySemaphore(vk::device, semaphore, nullptr);
	}

	void createFence(VkFence* fence) {
		VkFenceCreateInfo createInfo = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, 0};
		vkCreateFence(vk::device, &createInfo, nullptr, fence);
	}
	void destroyFence(VkFence fence) {
		vkDestroyFence(vk::device, fence, nullptr);
	}

	void resetFence(VkFence fence) {
		vkResetFences(vk::device, 1, &fence);
	}
	void waitForFence(VkFence fence) {
		vkWaitForFences(vk::device, 1, &fence, true, std::numeric_limits<uint64_t>::max());
		resetFence(fence);
	}

	/*CommandBuffer*/
	CommandBuffer::CommandBuffer(){}

	CommandBuffer::CommandBuffer(bool autoAllocate)
	{
		if (autoAllocate) this->allocate();
	}

	CommandBuffer::~CommandBuffer()
	{
		if (!m_isAlloc) return;

		vkFreeCommandBuffers(vk::device, vk::commandPool, 1, &m_commandBuffer);
		m_isAlloc = false;
	}

	void CommandBuffer::allocate()
	{
		if (m_isAlloc) return;

		VkCommandBufferAllocateInfo allocateInfo;
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.pNext = nullptr;
		allocateInfo.commandPool = vk::commandPool;
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocateInfo.commandBufferCount = 1;

		VkResult result = vkAllocateCommandBuffers(vk::device, &allocateInfo, &m_commandBuffer);
		VK_ASSERT(result);
		m_isAlloc = true;
	}

	void CommandBuffer::begin(VkCommandBufferUsageFlags usageFlags)
	{
		if (!m_isAlloc) {
			std::cerr << "CommandBuffer: " << this << " begin has been called but the buffer wasn't allocated\n";
			throw std::runtime_error("ERROR: CommandBuffer.begin()");
		}

		VkCommandBufferBeginInfo beginInfo;
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.pNext = nullptr;
		beginInfo.flags = usageFlags;
		beginInfo.pInheritanceInfo = nullptr;

		VkResult result = vkBeginCommandBuffer(m_commandBuffer, &beginInfo);
		VK_ASSERT(result);
	}

	void CommandBuffer::end()
	{
		VkResult result = vkEndCommandBuffer(m_commandBuffer);
		VK_ASSERT(result);
	}

	void CommandBuffer::submit(VkQueue* queue, VkFence fence) {
		VkSubmitInfo submitInfo;
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		submitInfo.waitSemaphoreCount = m_waitSemaphores.size();
		submitInfo.pWaitSemaphores = m_waitSemaphores.data();
		submitInfo.pWaitDstStageMask = m_waitDstStageMasks.data();
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_commandBuffer;
		submitInfo.signalSemaphoreCount = m_signalSemaphores.size();
		submitInfo.pSignalSemaphores = m_signalSemaphores.data();

		*queue = vkUtils::queueHandler::getQueue();
		vkQueueSubmit(*queue, 1, &submitInfo, fence);
	}
	void CommandBuffer::submit(VkFence fence) {
		VkQueue queue;
		submit(&queue, fence);
	}
	void CommandBuffer::submit()
	{
		VkQueue queue;
		VkFence fence; vk::createFence(&fence);
		submit(&queue, fence);
		vk::waitForFence(fence); vk::destroyFence(fence);
	}
	void CommandBuffer::submit(VkQueue* queue)
	{
		VkFence fence; vk::createFence(&fence);
		submit(queue, fence);
		vk::waitForFence(fence); vk::destroyFence(fence);
	}

	void CommandBuffer::addWaitSemaphore(VkSemaphore waitSemaphore, VkPipelineStageFlags waitDstStageMask) {
		m_waitSemaphores.push_back(waitSemaphore);
		m_waitDstStageMasks.push_back(waitDstStageMask);
	}
	void CommandBuffer::delWaitSemaphore(int index) {
		m_waitSemaphores.erase(m_waitSemaphores.begin() + index);
		m_waitDstStageMasks.erase(m_waitDstStageMasks.begin() + index);
	}

	/*Buffer*/
	Buffer::Buffer() {}
	Buffer::Buffer(VkDeviceSize size, VkBufferUsageFlags usage)
		: m_size(size), m_usage(usage)
	{}

	Buffer::~Buffer()
	{
		if (m_isAlloc)
		{
			m_isAlloc = false;
			vkFreeMemory(vk::device, m_deviceMemory, nullptr);
		}

		if (m_isInit)
		{
			m_isInit = false;
			vkDestroyBuffer(vk::device, m_buffer, nullptr);
		}
	}

	Buffer& Buffer::operator=(const Buffer& other) {
		this->~Buffer();

		m_size = other.m_size;
		m_usage = other.m_usage;
		if (!other.m_isInit) return *this;
		m_isInit = other.m_isInit;
		m_buffer = other.m_buffer;
		if (!other.m_isAlloc) return *this;
		m_isAlloc = other.m_isAlloc;
		m_memoryPropertyFlags = other.m_memoryPropertyFlags;
		m_deviceMemory = other.m_deviceMemory;
		
		return *this;
	}

	void Buffer::init()
	{
		if (m_isInit)
			return;
		m_isInit = true;

		VkBufferCreateInfo createInfo;
		createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.size = m_size;
		createInfo.usage = m_usage;
		createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;

		VkResult result = vkCreateBuffer(vk::device, &createInfo, nullptr, &m_buffer);
		VK_ASSERT(result);
	}

	void Buffer::allocate(VkMemoryPropertyFlags memoryPropertyFlags)
	{
		if (m_isAlloc)
			return;
		m_isAlloc = true;

		VkMemoryRequirements memoryRequirements;
		vkGetBufferMemoryRequirements(vk::device, m_buffer, &memoryRequirements);

		VkMemoryAllocateInfo allocateInfo;
		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocateInfo.pNext = nullptr;
		allocateInfo.allocationSize = memoryRequirements.size;
		allocateInfo.memoryTypeIndex = vkUtils::findMemoryTypeIndex(vk::physicalDevice, memoryRequirements.memoryTypeBits, memoryPropertyFlags);

		m_memoryPropertyFlags = memoryPropertyFlags;
		VkResult result = vkAllocateMemory(vk::device, &allocateInfo, nullptr, &m_deviceMemory);
		VK_ASSERT(result);

		vkBindBufferMemory(vk::device, m_buffer, m_deviceMemory, 0);
	}

	void Buffer::map(void **data)
	{
		map(0, data);
	}
	void Buffer::map(VkDeviceSize offset, void **data)
	{
		if ((m_memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
		{
			std::cerr << "Memory is not host visible: enable VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT\n";
			throw std::runtime_error("Memory is not host visible");
		}
		vkMapMemory(vk::device, m_deviceMemory, offset, m_size, 0, data);
	}

	void Buffer::unmap()
	{
		vkUnmapMemory(vk::device, m_deviceMemory);
	}

	void Buffer::uploadData(vk::Buffer* buffer) {
		if ((buffer->m_usage & VK_BUFFER_USAGE_TRANSFER_SRC_BIT) != VK_BUFFER_USAGE_TRANSFER_SRC_BIT) {
			std::cerr << "Buffer cant be source of upload transfer: enable VK_BUFFER_USAGE_TRANSFER_SRC_BIT\n";
			throw std::runtime_error("Buffer cant be source of upload transfer");
		}
		Buffer::copyBuffer(this, buffer, m_size);
	}
	void Buffer::uploadData(uint32_t size, void *data)
	{
		if ((m_usage & VK_BUFFER_USAGE_TRANSFER_DST_BIT) != VK_BUFFER_USAGE_TRANSFER_DST_BIT)
		{
			std::cerr << "Buffer cant be destination of upload transfer: enable VK_BUFFER_USAGE_TRANSFER_DST_BIT\n";
			throw std::runtime_error("Buffer cant be destination of upload transfer");
		}
		Buffer stagingBuffer = Buffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
		stagingBuffer.init();
		stagingBuffer.allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		void *rawData;
		stagingBuffer.map(&rawData);
		memcpy(rawData, data, size);
		stagingBuffer.unmap();

		Buffer::copyBuffer(this, &stagingBuffer, m_size);
	}

	void Buffer::copyBuffer(vk::Buffer* dst, vk::Buffer* src, VkDeviceSize size)
	{
		CommandBuffer commandBuffer = CommandBuffer(true);
		commandBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		VkBufferCopy bufferCopy;
		bufferCopy.srcOffset = 0;
		bufferCopy.dstOffset = 0;
		bufferCopy.size = size;
		vkCmdCopyBuffer(commandBuffer.getVkCommandBuffer(), *src, *dst, 1, &bufferCopy);

		commandBuffer.end();
		commandBuffer.submit();
	}

	/*Image*/
	Image::~Image()
	{
		if (m_isViewInit)
		{
			m_isViewInit = false;
			vkDestroyImageView(vk::device, m_imageView, nullptr);
		}

		if (m_isAlloc)
		{
			m_isAlloc = false;
			vkFreeMemory(vk::device, m_deviceMemory, nullptr);
		}

		if (m_isInit)
		{
			m_isInit = false;
			vkDestroyImage(vk::device, m_image, nullptr);
		}
	}

	void Image::init()
	{
		if (m_isInit)
			return;
		m_isInit = true;

		VkImageCreateInfo createInfo;
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.imageType = m_type;
		createInfo.format = m_format;
		createInfo.extent = m_extent;
		createInfo.mipLevels = m_mipLevelCount;
		createInfo.arrayLayers = 1;
		createInfo.samples = m_samples;
		createInfo.tiling = m_tiling;
		createInfo.usage = m_usage;
		createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
		createInfo.initialLayout = m_currentLayout;

		vkCreateImage(vk::device, &createInfo, nullptr, &m_image);
	}

	void Image::allocate(VkMemoryPropertyFlags memoryProperties)
	{
		if (m_isAlloc)
			return;
		m_isAlloc = true;

		VkMemoryRequirements memoryRequirements;
		vkGetImageMemoryRequirements(vk::device, m_image, &memoryRequirements);

		VkMemoryAllocateInfo allocateInfo{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, nullptr};
		allocateInfo.allocationSize = memoryRequirements.size;
		allocateInfo.memoryTypeIndex = vkUtils::findMemoryTypeIndex(vk::physicalDevice, memoryRequirements.memoryTypeBits, memoryProperties);

		vkAllocateMemory(vk::device, &allocateInfo, nullptr, &m_deviceMemory);

		vkBindImageMemory(vk::device, m_image, m_deviceMemory, 0);
	}

	void Image::initView()
	{
		if (m_isViewInit)
			return;
		m_isViewInit = true;

		VkImageViewCreateInfo viewCreateInfo;
		viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewCreateInfo.pNext = nullptr;
		viewCreateInfo.flags = 0;
		viewCreateInfo.image = m_image;
		viewCreateInfo.viewType = m_viewType;
		viewCreateInfo.format = m_format;
		viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		m_subresourceRange.aspectMask = m_aspect;
		m_subresourceRange.baseMipLevel = 0;
		m_subresourceRange.levelCount = m_mipLevelCount;
		m_subresourceRange.baseArrayLayer = 0;
		m_subresourceRange.layerCount = 1;
		viewCreateInfo.subresourceRange = m_subresourceRange;

		vkCreateImageView(vk::device, &viewCreateInfo, nullptr, &m_imageView);
	}

	void Image::changeLayout(VkImageLayout layout, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask){
		vk::changeImageLayout(*this, layout, srcAccessMask, dstAccessMask);
	}

	/*Surface*/
	Surface::~Surface()
	{
		if (!m_isInit)
			return;
		m_isInit = false;

		vkDestroySurfaceKHR(vk::instance, m_surface, nullptr);
	}

	void Surface::init()
	{
		if (m_isInit)
			return;
		m_isInit = true;

		if (m_GLFWwindow != nullptr)
		{
			VkResult result = glfwCreateWindowSurface(vk::instance, m_GLFWwindow, nullptr, &m_surface);
			VK_ASSERT(result);
		}

		if (!vkUtils::checkSurfaceSupport(vk::physicalDevice, m_surface))
			throw std::runtime_error("Surface not Supported!");
	}

	/*Swapchain*/
	Swapchain::Swapchain(){}

	Swapchain::~Swapchain()
	{
		if (!m_isInit)
			return;
		m_isInit = false;

		for (VkImageView imageView : m_imageViews)
		{
			vkDestroyImageView(vk::device, imageView, nullptr);
		}

		vkDestroySwapchainKHR(vk::device, m_swapchain, nullptr);
	}

	void Swapchain::init()
	{
		if (m_isInit)
			return;
		m_isInit = true;

		VkSwapchainCreateInfoKHR createInfo;
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.surface = m_surface;
		createInfo.minImageCount = 3;
		createInfo.imageFormat = m_imageFormat;
		createInfo.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
		createInfo.imageExtent = m_imageExtent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
		createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = m_presentMode;
		createInfo.clipped = true;
		createInfo.oldSwapchain = m_swapchain;

		VkSurfaceCapabilitiesKHR surfaceCapabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk::physicalDevice, m_surface, &surfaceCapabilities);

		auto supportedSurfacePresentModes = vkUtils::getSupportedSurfacePresentModes(vk::physicalDevice, m_surface);

		if (createInfo.minImageCount > surfaceCapabilities.maxImageCount)
			createInfo.minImageCount = surfaceCapabilities.maxImageCount;
		if (createInfo.imageSharingMode == VK_SHARING_MODE_CONCURRENT)
			throw std::runtime_error("VK_SHARING_MODE_CONCURRENT is not yet supported!");
		bool supported = false;
		for (size_t i = 0; i < supportedSurfacePresentModes.size(); i++)
			supported |= m_presentMode == supportedSurfacePresentModes[i];
		if (!supported)
		{
			std::cerr << "VkPresentModeKHR: " << m_presentMode << " not supported, changed to VK_PRESENT_MODE_FIFO_KHR\n";
			m_presentMode = VK_PRESENT_MODE_FIFO_KHR;
		}

		vkCreateSwapchainKHR(vk::device, &createInfo, nullptr, &m_swapchain);

		uint32_t imageCount = 0;
		vkGetSwapchainImagesKHR(vk::device, m_swapchain, &imageCount, nullptr);
		m_images.resize(imageCount);
		m_imageViews.resize(imageCount);
		vkGetSwapchainImagesKHR(vk::device, m_swapchain, &imageCount, m_images.data());

		m_imageSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		m_imageSubresourceRange.baseMipLevel = 0;
		m_imageSubresourceRange.levelCount = 1;
		m_imageSubresourceRange.baseArrayLayer = 0;
		m_imageSubresourceRange.layerCount = 1;

		for (int i = 0; i < imageCount; i++)
		{
			VkImageViewCreateInfo viewCreateInfo;
			viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewCreateInfo.pNext = nullptr;
			viewCreateInfo.flags = 0;
			viewCreateInfo.image = m_images[i];
			viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewCreateInfo.format = m_imageFormat;
			viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			viewCreateInfo.subresourceRange = m_imageSubresourceRange;

			vkCreateImageView(vk::device, &viewCreateInfo, nullptr, &m_imageViews[i]);
		}
	}

	void Swapchain::update()
	{
		this->~Swapchain();
		init();
	}

	VkImage Swapchain::getImage(uint32_t index) {
		if (index >= m_images.size()) return VK_NULL_HANDLE;
		return m_images[index];
	}

	VkImageView Swapchain::getImageView(uint32_t index) {
		if (index >= m_imageViews.size()) return VK_NULL_HANDLE;
		return m_imageViews[index];
	}

	/*DescriptorPool*/
	void descriptorPoolCreate(
		std::vector<VkDescriptorPoolSize>&                      poolSizes,
		std::vector<VkDescriptorSetLayoutCreateInfo>&           setLayoutCreateInfos,
		std::vector<std::vector<VkDescriptorSetLayoutBinding>>& setLayoutCreateInfoBindings,
		uint32_t&                                               descriptorSetCount,
		uint32_t&                                               descriptorSetArrayLength,
		VkDescriptorSet*&                                       pDescriptorSets,
		VkDescriptorSetLayout*&                                 pDescriptorSetLayouts,
		std::vector<VkWriteDescriptorSet>&                      writeDescriptorSets,
		std::vector<uint32_t>&                                  writeDescriptorSetIndices,
		VkDescriptorPool&                                       m_descriptorPool
	) {
		VkDescriptorPoolCreateInfo descriptorPoolCreateInfo;
		descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolCreateInfo.pNext = nullptr;
		descriptorPoolCreateInfo.flags = 0;
		descriptorPoolCreateInfo.maxSets = setLayoutCreateInfos.size();
		descriptorPoolCreateInfo.poolSizeCount = poolSizes.size();
		descriptorPoolCreateInfo.pPoolSizes = poolSizes.data();

		vkCreateDescriptorPool(vk::device, &descriptorPoolCreateInfo, nullptr, &m_descriptorPool);

		pDescriptorSetLayouts = new VkDescriptorSetLayout[descriptorSetCount];
		descriptorSetArrayLength = descriptorSetCount;
		for (int i = 0; i < descriptorSetArrayLength; i++)
		{
			setLayoutCreateInfos[i].bindingCount = setLayoutCreateInfoBindings[i].size();
			setLayoutCreateInfos[i].pBindings = setLayoutCreateInfoBindings[i].data();
			vkCreateDescriptorSetLayout(vk::device, &setLayoutCreateInfos[i], nullptr, &pDescriptorSetLayouts[i]);
		}

		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo;
		descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocateInfo.pNext = nullptr;
		descriptorSetAllocateInfo.descriptorPool = m_descriptorPool;
		descriptorSetAllocateInfo.descriptorSetCount = descriptorSetArrayLength;
		descriptorSetAllocateInfo.pSetLayouts = pDescriptorSetLayouts;

		pDescriptorSets = new VkDescriptorSet[descriptorSetArrayLength];
		vkAllocateDescriptorSets(vk::device, &descriptorSetAllocateInfo, pDescriptorSets);

		for (int i = 0; i < writeDescriptorSets.size(); i++)
		{
			writeDescriptorSets[i].dstSet = pDescriptorSets[writeDescriptorSetIndices[i]];
		}

		vkUpdateDescriptorSets(vk::device, writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
	}

	void descriptorPoolDestroy(
		uint32_t&                          descriptorSetArrayLenght,
		VkDescriptorSet*&                  pDescriptorSets,
		VkDescriptorSetLayout*&            pDescriptorSetLayouts,
		std::vector<VkWriteDescriptorSet>& writeDescriptorSets,
		VkDescriptorPool&                  descriptorPool
	) {
		for (int i = 0; i < descriptorSetArrayLenght; i++)
		{
			vkDestroyDescriptorSetLayout(vk::device, pDescriptorSetLayouts[i], nullptr);
		}
		vkDestroyDescriptorPool(vk::device, descriptorPool, nullptr);

		delete[] pDescriptorSetLayouts;
		delete[] pDescriptorSets;
	}

	DescriptorPool::~DescriptorPool()
	{
		if (!m_isInit)
			return;
		m_isInit = false;

		descriptorPoolDestroy(
			m_descriptorSetCount,
			m_pDescriptorSets,
			m_pDescriptorSetLayouts,
			m_writeDescriptorSets,
			m_descriptorPool
		);
		for (int i = 0; i < m_writeDescriptorSets.size(); i++)
		{
			delete m_writeDescriptorSets[i].pImageInfo;
			delete m_writeDescriptorSets[i].pBufferInfo;
			delete m_writeDescriptorSets[i].pTexelBufferView;
		}
	}

	void DescriptorPool::init()
	{
		if (m_isInit)
			return;
		m_isInit = true;

		descriptorPoolCreate(
			m_poolSizes,
			m_setLayoutCreateInfos,
			m_setLayoutCreateInfoBindings,
			m_descriptorSetCount,
			m_descriptorSetArrayLength,
			m_pDescriptorSets,
			m_pDescriptorSetLayouts,
			m_writeDescriptorSets,
			m_writeDescriptorSetIndices,
			m_descriptorPool
		);
	}

	void DescriptorPool::update() {
		if (m_isInit) {
			descriptorPoolDestroy(
				m_descriptorSetArrayLength,
				m_pDescriptorSets,
				m_pDescriptorSetLayouts,
				m_writeDescriptorSets,
				m_descriptorPool
			);
		}
		else {
			m_isInit = true;
		}

		descriptorPoolCreate(
			m_poolSizes,
			m_setLayoutCreateInfos,
			m_setLayoutCreateInfoBindings,
			m_descriptorSetCount,
			m_descriptorSetArrayLength,
			m_pDescriptorSets,
			m_pDescriptorSetLayouts,
			m_writeDescriptorSets,
			m_writeDescriptorSetIndices,
			m_descriptorPool
		);
	}

	void DescriptorPool::addDescriptorSet()
	{
		VkDescriptorSetLayoutCreateInfo createInfo;
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.bindingCount = 0;
		createInfo.pBindings = nullptr;

		m_descriptorSetCount++;
		m_setLayoutCreateInfos.push_back(createInfo);
		m_setLayoutCreateInfoBindings.push_back(std::vector<VkDescriptorSetLayoutBinding>());
	}

	void DescriptorPool::addDescriptor(const Descriptor &descriptor, uint32_t setIndex)
	{
		if (setIndex >= m_setLayoutCreateInfos.size())
		{
			std::cerr << "ERROR: DescriptorSet Index out of range: " << setIndex << "\n";
			throw std::runtime_error("DescriptorSet Index out of range");
		}

		{
			bool typeAlreadyExists = false;
			for (VkDescriptorPoolSize poolSize : m_poolSizes)
			{
				if (poolSize.type == descriptor.type)
				{
					typeAlreadyExists = true;
					poolSize.descriptorCount++;
					break;
				}
			}
			if (!typeAlreadyExists)
			{
				VkDescriptorPoolSize descriptorPoolSize;
				descriptorPoolSize.type = descriptor.type;
				descriptorPoolSize.descriptorCount = 1;

				m_poolSizes.push_back(descriptorPoolSize);
			}
		}

		VkDescriptorSetLayoutBinding layoutBinding;
		layoutBinding.binding = descriptor.binding;
		layoutBinding.descriptorType = descriptor.type;
		layoutBinding.descriptorCount = 1;
		layoutBinding.stageFlags = descriptor.stages;
		layoutBinding.pImmutableSamplers = nullptr;

		m_setLayoutCreateInfoBindings[setIndex].push_back(layoutBinding);

		VkWriteDescriptorSet writeDescriptorSet;
		writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet.pNext = nullptr;
		writeDescriptorSet.dstBinding = descriptor.binding;
		writeDescriptorSet.dstArrayElement = 0;
		writeDescriptorSet.descriptorCount = 1;
		writeDescriptorSet.descriptorType = descriptor.type;
		VkDescriptorImageInfo *pImageInfo = nullptr;
		if (descriptor.pImageInfo != nullptr)
		{
			pImageInfo = new VkDescriptorImageInfo;
			*pImageInfo = *descriptor.pImageInfo;
		}
		writeDescriptorSet.pImageInfo = pImageInfo;
		VkDescriptorBufferInfo *pBufferInfo = nullptr;
		if (descriptor.pBufferInfo != nullptr)
		{
			pBufferInfo = new VkDescriptorBufferInfo;
			*pBufferInfo = *descriptor.pBufferInfo;
		}
		writeDescriptorSet.pBufferInfo = pBufferInfo;
		VkBufferView *pTexelBufferView = nullptr;
		if (descriptor.pTexelBufferView != nullptr)
		{
			pTexelBufferView = new VkBufferView;
			*pTexelBufferView = *descriptor.pTexelBufferView;
		}
		writeDescriptorSet.pTexelBufferView = pTexelBufferView;

		m_writeDescriptorSets.push_back(writeDescriptorSet);
		m_writeDescriptorSetIndices.push_back(setIndex);
	}

	/*Shader*/
	Shader::Shader()
	{
		m_shaderStage.pName = "main";
		m_shaderStage.pSpecializationInfo = nullptr;
	}

	Shader::~Shader()
	{
		if (!m_isInit)
			return;
		m_isInit = false;

		vkDestroyShaderModule(vk::device, m_module, nullptr);
	}

	void Shader::init()
	{
		if (m_isInit)
			return;
		m_isInit = true;

		auto code = vkUtils::readFile(m_path.c_str());

		VkShaderModuleCreateInfo moduleCreateInfo;
		moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		moduleCreateInfo.pNext = nullptr;
		moduleCreateInfo.flags = 0;
		moduleCreateInfo.codeSize = code.size();
		moduleCreateInfo.pCode = (uint32_t *)code.data();

		VkResult result = vkCreateShaderModule(vk::device, &moduleCreateInfo, nullptr, &m_module);
		VK_ASSERT(result);
		m_shaderStage.module = m_module;
	}

	void Shader::compile(std::string srcDir, std::vector<std::string> srcNames, std::vector<std::string> dstDirs) {
		for (auto srcName : srcNames) {
			std::string dstName = srcName + ".spv";
			system(("%VULKAN_SDK%\\Bin\\glslangValidator.exe -V " + srcDir + srcName + " -o " + dstDirs[0] + dstName).c_str());
			for (int i = 1; i < dstDirs.size(); i++) {
				system(("copy " + dstDirs[0] + dstName + " " + dstDirs[i]).c_str());
			}
		}
	}

	/*RenderPass*/
	RenderPass::RenderPass()
	{
	}

	RenderPass::~RenderPass()
	{
		for (VkAttachmentReference *ptr : m_attachmentReferencePtrs)
		{
			delete ptr;
		}

		if (!m_isInit)
			return;
		m_isInit = false;

		vkDestroyRenderPass(vk::device, m_renderPass, nullptr);
	}

	void RenderPass::init()
	{
		if (m_isInit)
			return;
		m_isInit = true;

		VkRenderPassCreateInfo createInfo;
		createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.attachmentCount = m_attachmentDescriptions.size();
		createInfo.pAttachments = m_attachmentDescriptions.data();
		createInfo.subpassCount = m_subpassDescriptions.size();
		createInfo.pSubpasses = m_subpassDescriptions.data();
		createInfo.dependencyCount = m_subpassDependencies.size();
		createInfo.pDependencies = m_subpassDependencies.data();

		vkCreateRenderPass(vk::device, &createInfo, nullptr, &m_renderPass);
	}

	void RenderPass::addAttachmentDescription(const VkAttachmentDescription& description) {
		m_attachmentDescriptions.push_back(description);
	}

	void RenderPass::addAttachmentReference(VkAttachmentReference** referencePtr) {
		VkAttachmentReference* attachmentReference = new VkAttachmentReference;
		attachmentReference->attachment = (*referencePtr)->attachment;
		attachmentReference->layout = (*referencePtr)->layout;

		*referencePtr = attachmentReference;

		m_attachmentReferencePtrs.push_back(attachmentReference);
	}

	void RenderPass::addSubpassDescription(const VkSubpassDescription& description) {
		m_subpassDescriptions.push_back(description);
	}

	void RenderPass::addSubpassDependency(const VkSubpassDependency& dependency) {
		m_subpassDependencies.push_back(dependency);
	}

	/*Framebuffer*/
	Framebuffer::Framebuffer()
	{
	}

	Framebuffer::~Framebuffer()
	{
		if (!m_isInit)
			return;
		m_isInit = false;

		vkDestroyFramebuffer(vk::device, m_framebuffer, nullptr);
	}

	void Framebuffer::init()
	{
		if (m_isInit)
			return;
		m_isInit = true;

		VkFramebufferCreateInfo createInfo;
		createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.renderPass = m_renderPass;
		createInfo.attachmentCount = m_attachments.size();
		createInfo.pAttachments = m_attachments.data();
		createInfo.width = m_width;
		createInfo.height = m_height;
		createInfo.layers = 1;

		vkCreateFramebuffer(vk::device, &createInfo, nullptr, &m_framebuffer);
	}

	/*Pipeline*/
	Pipeline::Pipeline()
	{
		m_vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		m_vertexInputStateCreateInfo.pNext = nullptr;
		m_vertexInputStateCreateInfo.flags = 0;
		m_vertexInputStateCreateInfo.vertexBindingDescriptionCount = 0;
		m_vertexInputStateCreateInfo.pVertexBindingDescriptions = nullptr;
		m_vertexInputStateCreateInfo.vertexAttributeDescriptionCount = 0;
		m_vertexInputStateCreateInfo.pVertexAttributeDescriptions = nullptr;

		m_inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		m_inputAssemblyStateCreateInfo.pNext = nullptr;
		m_inputAssemblyStateCreateInfo.flags = 0;
		m_inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		m_inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

		m_viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		m_viewportStateCreateInfo.pNext = nullptr;
		m_viewportStateCreateInfo.flags = 0;
		m_viewportStateCreateInfo.viewportCount = 0;
		m_viewportStateCreateInfo.pViewports = nullptr; // addViewport();
		m_viewportStateCreateInfo.scissorCount = 0;
		m_viewportStateCreateInfo.pScissors = nullptr; // addScissor();

		m_rasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		m_rasterizationStateCreateInfo.pNext = nullptr;
		m_rasterizationStateCreateInfo.flags = 0;
		m_rasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
		m_rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
		m_rasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
		m_rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_NONE;
		m_rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		m_rasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
		m_rasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
		m_rasterizationStateCreateInfo.depthBiasClamp = 0.0f;
		m_rasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;
		m_rasterizationStateCreateInfo.lineWidth = 1.0f;

		m_multisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		m_multisampleStateCreateInfo.pNext = nullptr;
		m_multisampleStateCreateInfo.flags = 0;
		m_multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		m_multisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
		m_multisampleStateCreateInfo.minSampleShading = 1.0f;
		m_multisampleStateCreateInfo.pSampleMask = nullptr;
		m_multisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
		m_multisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;

		m_depthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		m_depthStencilStateCreateInfo.pNext = nullptr;
		m_depthStencilStateCreateInfo.flags = 0;
		m_depthStencilStateCreateInfo.depthTestEnable = VK_FALSE;
		m_depthStencilStateCreateInfo.depthWriteEnable = VK_FALSE;
		m_depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
		m_depthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
		m_depthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;
		m_depthStencilStateCreateInfo.front = {};
		m_depthStencilStateCreateInfo.back = {};
		m_depthStencilStateCreateInfo.minDepthBounds = 0.0f;
		m_depthStencilStateCreateInfo.maxDepthBounds = 1.0f;

		m_colorBlendAttachment.blendEnable = VK_TRUE;
		m_colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		m_colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		m_colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		m_colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		m_colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		m_colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
		m_colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

		m_colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		m_colorBlendStateCreateInfo.pNext = nullptr;
		m_colorBlendStateCreateInfo.flags = 0;
		m_colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
		m_colorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_NO_OP;
		m_colorBlendStateCreateInfo.attachmentCount = 1;
		m_colorBlendStateCreateInfo.pAttachments = &m_colorBlendAttachment;
		m_colorBlendStateCreateInfo.blendConstants[0] = 0.0f;
		m_colorBlendStateCreateInfo.blendConstants[1] = 0.0f;
		m_colorBlendStateCreateInfo.blendConstants[2] = 0.0f;
		m_colorBlendStateCreateInfo.blendConstants[3] = 0.0f;

		m_dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		m_dynamicStateCreateInfo.pNext = nullptr;
		m_dynamicStateCreateInfo.flags = 0;
		m_dynamicStateCreateInfo.dynamicStateCount = 0;
		m_dynamicStateCreateInfo.pDynamicStates = nullptr;
	}

	Pipeline::~Pipeline()
	{
		if (!m_isInit)
			return;
		m_isInit = false;

		vkDestroyPipeline(vk::device, m_pipeline, nullptr);
		vkDestroyPipelineLayout(vk::device, m_pipelineLayout, nullptr);
	}

	void Pipeline::init()
	{
		if (m_isInit)
			return;
		m_isInit = true;

		VkPipelineLayoutCreateInfo layoutCreateInfo;
		layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutCreateInfo.pNext = nullptr;
		layoutCreateInfo.flags = 0;
		layoutCreateInfo.setLayoutCount = m_setLayouts.size();
		layoutCreateInfo.pSetLayouts = m_setLayouts.data();
		layoutCreateInfo.pushConstantRangeCount = 0;
		layoutCreateInfo.pPushConstantRanges = nullptr;

		vkCreatePipelineLayout(vk::device, &layoutCreateInfo, nullptr, &m_pipelineLayout);

		m_vertexInputStateCreateInfo.vertexAttributeDescriptionCount = m_vertexInputAttributeDescriptions.size();
		m_vertexInputStateCreateInfo.pVertexAttributeDescriptions = m_vertexInputAttributeDescriptions.data();
		m_vertexInputStateCreateInfo.vertexBindingDescriptionCount = m_vertexInputBindingDescriptions.size();
		m_vertexInputStateCreateInfo.pVertexBindingDescriptions = m_vertexInputBindingDescriptions.data();

		m_viewportStateCreateInfo.viewportCount = m_viewports.size();
		m_viewportStateCreateInfo.pViewports = m_viewports.data();
		m_viewportStateCreateInfo.scissorCount = m_scissors.size();
		m_viewportStateCreateInfo.pScissors = m_scissors.data();

		m_dynamicStateCreateInfo.dynamicStateCount = m_dynamicStates.size();
		m_dynamicStateCreateInfo.pDynamicStates = m_dynamicStates.data();

		VkGraphicsPipelineCreateInfo createInfo;
		createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.stageCount = m_shaderStages.size();
		createInfo.pStages = m_shaderStages.data();
		createInfo.pVertexInputState = &m_vertexInputStateCreateInfo;
		createInfo.pInputAssemblyState = &m_inputAssemblyStateCreateInfo;
		createInfo.pTessellationState = nullptr;
		createInfo.pViewportState = &m_viewportStateCreateInfo;
		createInfo.pRasterizationState = &m_rasterizationStateCreateInfo;
		createInfo.pMultisampleState = &m_multisampleStateCreateInfo;
		createInfo.pDepthStencilState = &m_depthStencilStateCreateInfo;
		createInfo.pColorBlendState = &m_colorBlendStateCreateInfo;
		createInfo.pDynamicState = &m_dynamicStateCreateInfo;
		createInfo.layout = m_pipelineLayout;
		createInfo.renderPass = m_renderPass;
		createInfo.subpass = 0;
		createInfo.basePipelineHandle = VK_NULL_HANDLE;
		createInfo.basePipelineIndex = -1;

		VkResult result = vkCreateGraphicsPipelines(vk::device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &m_pipeline);
		VK_ASSERT(result);
	}

	void Pipeline::addShader(const VkPipelineShaderStageCreateInfo &shaderStage)
	{
		m_shaderStages.push_back(shaderStage);
	}

	void Pipeline::delShader(int index)
	{
		m_shaderStages.erase(m_shaderStages.begin() + index);
	}

	void Pipeline::addVertexInputBindingDescription(const VkVertexInputBindingDescription &vertexInputBindingDescription)
	{
		m_vertexInputBindingDescriptions.push_back(vertexInputBindingDescription);
	}

	void Pipeline::delVertexInputBindingDescription(int index)
	{
		m_vertexInputBindingDescriptions.erase(m_vertexInputBindingDescriptions.begin() + index);
	}

	void Pipeline::addVertexInputAttrubuteDescription(const VkVertexInputAttributeDescription &vertexInputAttributeDescription)
	{
		m_vertexInputAttributeDescriptions.push_back(vertexInputAttributeDescription);
	}

	void Pipeline::delVertexInputAttrubuteDescription(int index)
	{
		m_vertexInputAttributeDescriptions.erase(m_vertexInputAttributeDescriptions.begin() + index);
	}

	void Pipeline::addViewport(const VkViewport &viewport)
	{
		m_viewports.push_back(viewport);
	}

	void Pipeline::delViewport(int index)
	{
		m_viewports.erase(m_viewports.begin() + index);
	}

	void Pipeline::addScissor(const VkRect2D &scissor)
	{
		m_scissors.push_back(scissor);
	}

	void Pipeline::delScissor(int index)
	{
		m_scissors.erase(m_scissors.begin() + index);
	}

	void Pipeline::enableDepthTest() {
		m_depthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
		m_depthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
	}

	void Pipeline::disableDepthTest() {
		m_depthStencilStateCreateInfo.depthTestEnable = VK_FALSE;
		m_depthStencilStateCreateInfo.depthWriteEnable = VK_FALSE;
	}

	void createCommandPool(VkDevice &device, size_t queueFamily, VkCommandPool &commandPool)
	{
		VkCommandPoolCreateInfo createInfo;
		createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		createInfo.queueFamilyIndex = queueFamily;

		vkCreateCommandPool(device, &createInfo, nullptr, &commandPool);
	}
	// Raytracing

	void createBottomLevelAccelerationStructure()
	{
	}

	void createTopLevelAccelerationStructure(VkDevice &device, VkPhysicalDevice &physicalDevice, VkCommandPool &commandPool, VkQueue &queue, std::vector<VkAccelerationStructureInstanceKHR> &instances, VkAccelerationStructureKHR &accelerationStructure)
	{
		uint32_t countInstances = static_cast<uint32_t>(instances.size());

		VkBuffer instanceBuffer;
		VkDeviceSize instanceBufferSize = countInstances * sizeof(VkAccelerationStructureInstanceKHR);
		VkDeviceMemory instancesDeviceMemory;
		vkUtils::createBuffer(device, physicalDevice, instanceBufferSize, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, instancesDeviceMemory, instanceBuffer);
		vkUtils::uploadBuffer(device, physicalDevice, commandPool, queue, instanceBufferSize, instances.data(), instanceBuffer);

		VkBufferDeviceAddressInfo bufferDeviceAddressInfo{VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, nullptr};
		bufferDeviceAddressInfo.buffer = instanceBuffer;
		VkDeviceAddress instBufferAddr = vkGetBufferDeviceAddress(device, &bufferDeviceAddressInfo);

		VkAccelerationStructureGeometryInstancesDataKHR instancesData{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR};
		instancesData.data.deviceAddress = instBufferAddr;

		VkAccelerationStructureGeometryKHR topASGeometry{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR};
		topASGeometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
		topASGeometry.geometry.instances = instancesData;

		VkAccelerationStructureBuildGeometryInfoKHR geometryInfo;
		geometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		geometryInfo.geometryCount = 1;
		geometryInfo.pGeometries = &topASGeometry;
		geometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		geometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		geometryInfo.srcAccelerationStructure = VK_NULL_HANDLE;

		VkAccelerationStructureBuildSizesInfoKHR sizeInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR};
		auto vkGetAccelerationStructureBuildSizesKHR = (PFN_vkGetAccelerationStructureBuildSizesKHR)vkGetInstanceProcAddr(vk::instance, "vkGetAccelerationStructureBuildSizesKHR");
		vkGetAccelerationStructureBuildSizesKHR(device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &geometryInfo, &countInstances, &sizeInfo);

		VkBuffer accelBuffer;
		VkDeviceMemory accelDeviceMemory;
		vkUtils::createBuffer(device, physicalDevice, sizeInfo.accelerationStructureSize, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, accelDeviceMemory, accelBuffer);

		VkAccelerationStructureCreateInfoKHR createInfo;
		createInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
		createInfo.pNext = nullptr;
		createInfo.createFlags = 0;
		createInfo.buffer = accelBuffer;
		createInfo.offset = 0;
		createInfo.size = sizeInfo.accelerationStructureSize;
		createInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		createInfo.deviceAddress;

		auto vkCreateAccelerationStructureKHR = (PFN_vkCreateAccelerationStructureKHR)vkGetInstanceProcAddr(vk::instance, "vkCreateAccelerationStructureKHR");
		vkCreateAccelerationStructureKHR(device, &createInfo, nullptr, &accelerationStructure);
	}

	void acquireNextImage(VkSwapchainKHR swapchain, VkSemaphore semaphore, VkFence fence, uint32_t *pImageIndex)
	{
		VkResult result = vkAcquireNextImageKHR(vk::device, swapchain, std::numeric_limits<uint64_t>::max(), semaphore, fence, pImageIndex);
		VK_ASSERT(result);
	}
	void acquireNextImage(Swapchain &swapchain, VkSemaphore semaphore, VkFence fence, uint32_t *pImageIndex)
	{
		VkResult result = vkAcquireNextImageKHR(vk::device, swapchain.getVkSwapchainKHR(), std::numeric_limits<uint64_t>::max(), semaphore, fence, pImageIndex);
		VK_ASSERT(result);
	}

	void changeImageLayout(vk::Image& image, VkImageLayout layout, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask) {
		changeImageLayout(image, image.getSubresourceRange(), image.getLayout(), layout, srcAccessMask, dstAccessMask);
	}
	void changeImageLayout(VkImage image, VkImageSubresourceRange subresourceRange, VkImageLayout currentLayout, VkImageLayout layout, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask) {
		vk::CommandBuffer cmdBuffer = vk::CommandBuffer(true);
		cmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		VkImageMemoryBarrier imageMemoryBarrier;
		imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier.pNext = nullptr;
		imageMemoryBarrier.srcAccessMask = srcAccessMask;
		imageMemoryBarrier.dstAccessMask = dstAccessMask;
		imageMemoryBarrier.oldLayout = currentLayout;
		imageMemoryBarrier.newLayout = layout;
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.image = image;
		imageMemoryBarrier.subresourceRange = subresourceRange;

		vkCmdPipelineBarrier(cmdBuffer.getVkCommandBuffer(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

		cmdBuffer.end();
		cmdBuffer.submit();
	}

	void queuePresent(VkQueue queue, VkSwapchainKHR swapchain, uint32_t imageIndex)
	{
		VkPresentInfoKHR presentInfo;
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = nullptr;
		presentInfo.waitSemaphoreCount = 0;
		presentInfo.pWaitSemaphores = nullptr;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &swapchain;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr;

		VkResult result = vkQueuePresentKHR(queue, &presentInfo);
		VK_ASSERT(result);
	}
	void queuePresent(VkQueue queue, Swapchain &swapchain, uint32_t imageIndex)
	{
		queuePresent(queue, swapchain.getVkSwapchainKHR(), imageIndex);
	}
	void queuePresent(VkQueue queue, VkSwapchainKHR swapchain, uint32_t imageIndex, VkSemaphore waitSemaphore) {
		VkPresentInfoKHR presentInfo;
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = nullptr;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &waitSemaphore;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &swapchain;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr;

		VkResult result = vkQueuePresentKHR(queue, &presentInfo);
		VK_ASSERT(result);
	}
	void queuePresent(VkQueue queue, Swapchain& swapchain, uint32_t imageIndex, VkSemaphore waitSemaphore) {
		queuePresent(queue, swapchain.getVkSwapchainKHR(), imageIndex, waitSemaphore);
	}

	void deviceWaitIdle()
	{
		vkDeviceWaitIdle(vk::device);
	}
	void allQueuesWaitIdle()
	{
		for (VkQueue queue : vk::queues)
		{
			vkQueueWaitIdle(queue);
		}
	}
}

void initVulkan(const char *applicationName)
{
	std::vector<const char *> enabledInstanceLayers = {
#if _DEBUG
		"VK_LAYER_KHRONOS_validation"
#endif
	};
	std::vector<const char *> enabledInstanceExtensions = {};

	uint32_t amountOfRequiredGLFWExtensions; // Add Required GLFW Extensions to Instance Extensions
	auto glfwExtensions = glfwGetRequiredInstanceExtensions(&amountOfRequiredGLFWExtensions);
	for (size_t i = 0; i < amountOfRequiredGLFWExtensions; i++)
		enabledInstanceExtensions.push_back(glfwExtensions[i]);

	vk::createInstance(vk::instance, enabledInstanceLayers, enabledInstanceExtensions, applicationName); // Create Instance
	vk::physicalDevice = vkUtils::getAllPhysicalDevices(vk::instance)[VK_INDEX_OF_USED_PHYSICAL_DEVICE];

	VkPhysicalDeviceFeatures usedDeviceFeatures = {};
	std::vector<const char *> enabledDeviceLayers = {};
	std::vector<const char *> enabledDeviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
		//"VK_KHR_acceleration_structure"
	};
	vk::createLogicalDevice(vk::physicalDevice, vk::device, enabledDeviceLayers, enabledDeviceExtensions, usedDeviceFeatures); // Create Logical Device

	vk::queueFamily = VK_PREFERED_QUEUE_FAMILY; // TODO civ
	for (size_t i = 0; i < vk::queues.size(); i++)
		vkGetDeviceQueue(vk::device, vk::queueFamily, i, &vk::queues[i]); // Get Queues from Device
	vkUtils::queueHandler::init(vk::queues);

	// TODO Make compile automatic in shader class

	vk::createCommandPool(vk::device, vk::queueFamily, vk::commandPool);
}

void terminateVulkan()
{
	for (VkQueue queue : vk::queues)
	{
		vkQueueWaitIdle(queue);
	}

	vkDestroyCommandPool(vk::device, vk::commandPool, nullptr);

	vkDestroyDevice(vk::device, nullptr);
	vkDestroyInstance(vk::instance, nullptr);
}

void printStats()
{
#if PRINT_PHYSICAL_DEVICES
	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(vk::physicalDevice, &physicalDeviceProperties);
	std::cout << "Name: " << physicalDeviceProperties.deviceName << std::endl;
	uint32_t apiVer = physicalDeviceProperties.apiVersion;
	std::cout << "API Version: " << VK_VERSION_MAJOR(apiVer) << "." << VK_VERSION_MINOR(apiVer) << "." << VK_VERSION_PATCH(apiVer) << std::endl;

	VkPhysicalDeviceFeatures physicalDeviceFeatures;
	vkGetPhysicalDeviceFeatures(vk::physicalDevice, &physicalDeviceFeatures); // Check if feature is supported

#if PRINT_QUEUE_FAMILIES
	uint32_t amountOfQueueFamilies;
	vkGetPhysicalDeviceQueueFamilyProperties(vk::physicalDevice, &amountOfQueueFamilies, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamiliesProperties(amountOfQueueFamilies);
	vkGetPhysicalDeviceQueueFamilyProperties(vk::physicalDevice, &amountOfQueueFamilies, queueFamiliesProperties.data());
	for (size_t i = 0; i < amountOfQueueFamilies; i++)
	{
		std::cout << std::endl;
		VkQueueFamilyProperties properties = queueFamiliesProperties[i];
		std::cout << "Queue Family #" << i << std::endl;
		std::cout << "Count: " << properties.queueCount << std::endl;
		std::cout << "VK_QUEUE_GRAPHICS_BIT " << ((properties.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) << std::endl;
		std::cout << "VK_QUEUE_COMPUTE_BIT " << ((properties.queueFlags & VK_QUEUE_COMPUTE_BIT) != 0) << std::endl;
		std::cout << "VK_QUEUE_TRANSFER_BIT " << ((properties.queueFlags & VK_QUEUE_TRANSFER_BIT) != 0) << std::endl;
		std::cout << "VK_QUEUE_SPARSE_BINDING_BIT " << ((properties.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) != 0) << std::endl;
		std::cout << "Timestamp Valid Bits: " << properties.timestampValidBits << std::endl;
		uint32_t width = properties.minImageTransferGranularity.width;
		uint32_t height = properties.minImageTransferGranularity.height;
		uint32_t depth = properties.minImageTransferGranularity.depth;
		std::cout << "Min Image Tranfer Granularity: " << width << ", " << height << ", " << depth << std::endl;
	}
#endif

#if PRINT_AVAILABLE_DEVICE_EXTENSIONS
	uint32_t amountOfDeviceExtensions;
	vkEnumerateDeviceExtensionProperties(vk::physicalDevice, nullptr, &amountOfDeviceExtensions, nullptr);
	std::vector<VkExtensionProperties> deviceExtensionProperties(amountOfDeviceExtensions);
	vkEnumerateDeviceExtensionProperties(vk::physicalDevice, nullptr, &amountOfDeviceExtensions, deviceExtensionProperties.data());
	for (size_t i = 0; i < amountOfDeviceExtensions; i++)
	{
		std::cout << std::endl;
		VkExtensionProperties properties = deviceExtensionProperties[i];
		std::cout << "Name: " << properties.extensionName << std::endl;
		std::cout << "Spec Version: " << properties.specVersion << std::endl;
	}
#endif

#endif

#if PRINT_AVAILABLE_INSTANCE_LAYERS
	uint32_t amountOfInstanceLayers;
	vkEnumerateInstanceLayerProperties(&amountOfInstanceLayers, nullptr);
	std::vector<VkLayerProperties> instanceLayerProperties(amountOfInstanceLayers);
	vkEnumerateInstanceLayerProperties(&amountOfInstanceLayers, instanceLayerProperties.data());
	for (size_t i = 0; i < amountOfInstanceLayers; i++)
	{
		std::cout << std::endl;
		VkLayerProperties properties = instanceLayerProperties[i];
		std::cout << "Name: " << properties.layerName << std::endl;
		std::cout << "Description: " << properties.description << std::endl;
		std::cout << "Implementation Version: " << properties.implementationVersion << std::endl;
		std::cout << "Spec Version: " << properties.specVersion << std::endl;
	}
#endif

#if PRINT_AVAILABLE_INSTANCE_EXTENSIONS
	uint32_t amountOfInstanceExtensions;
	vkEnumerateInstanceExtensionProperties(nullptr, &amountOfInstanceExtensions, nullptr);
	std::vector<VkExtensionProperties> instanceExtensionProperties(amountOfInstanceExtensions);
	vkEnumerateInstanceExtensionProperties(nullptr, &amountOfInstanceExtensions, instanceExtensionProperties.data());
	for (size_t i = 0; i < amountOfInstanceExtensions; i++)
	{
		std::cout << std::endl;
		VkExtensionProperties properties = instanceExtensionProperties[i];
		std::cout << "Name: " << properties.extensionName << std::endl;
		std::cout << "Spec Version: " << properties.specVersion << std::endl;
	}
#endif
}