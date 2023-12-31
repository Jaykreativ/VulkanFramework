#pragma once

#include <iostream>
#include <chrono>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define PRINT_PHYSICAL_DEVICES true
#define PRINT_QUEUE_FAMILIES  false
#define PRINT_AVAILABLE_DEVICE_EXTENSIONS false
#define PRINT_AVAILABLE_INSTANCE_EXTENSIONS false
#define PRINT_AVAILABLE_INSTANCE_LAYERS false

#define VK_INDEX_OF_USED_PHYSICAL_DEVICE 0
#define VK_PREFERED_QUEUE_FAMILY 0
#define VK_PREFERED_AMOUNT_OF_QUEUES 4
#define VK_USED_SCREENCOLOR_FORMAT VK_FORMAT_B8G8R8A8_UNORM //TODO civ

namespace vk
{
	class CommandBuffer {
	public:
		CommandBuffer();
		CommandBuffer(bool autoAllocate);

		~CommandBuffer();

		operator VkCommandBuffer() const { return m_commandBuffer; }

		void allocate();

		void begin(VkCommandBufferUsageFlags usageFlags);

		void end();

		void submit();
		void submit(VkQueue* queue);
		void submit(VkFence fence);
		void submit(VkQueue* queue, VkFence fence);

		void addWaitSemaphore(VkSemaphore waitSemaphore, VkPipelineStageFlags waitDstStageMask);

		void delWaitSemaphore(int index);

		void addSignalSemaphore(VkSemaphore signalSemaphore) { m_signalSemaphores.push_back(signalSemaphore); }
		void delSignalSemaphore(int index) { m_signalSemaphores.erase(m_signalSemaphores.begin() + index); }

		const VkCommandBuffer& getVkCommandBuffer() { return m_commandBuffer; }

	private:
		VkCommandBuffer m_commandBuffer;

		std::vector<VkSemaphore> m_waitSemaphores;
		std::vector<VkPipelineStageFlags> m_waitDstStageMasks;
		std::vector<VkSemaphore> m_signalSemaphores;
	};

	class Buffer {
	public:
		Buffer();
		Buffer(VkDeviceSize size, VkBufferUsageFlags usage);

		~Buffer();

		Buffer& operator=(const Buffer& other);

		operator VkBuffer() { return m_buffer; }

		void init();

		void allocate(VkMemoryPropertyFlags memoryPropertyFlags);

		void map(void** ptr);
		void map(VkDeviceSize offset, void** ptr);

		void resize(VkDeviceSize size) { m_size = size; }

		void unmap();

		void uploadData(uint32_t size, void* data);

		void setUsage(VkBufferUsageFlags usage) { m_usage = usage; }

		VkBuffer getVkBuffer() { return m_buffer; }

		VkDeviceMemory getVkDeviceMemory() { return m_deviceMemory; }

		VkDeviceSize getSize() { return m_size; }

		VkBufferUsageFlags getUsage() { return m_usage; }

		VkMemoryPropertyFlags getMemoryPropertyFlags() { return m_memoryPropertyFlags; }

		static void copyBuffer(VkBuffer dst, VkBuffer src, VkDeviceSize size);

	private:
		bool m_isInit = false;
		bool m_isAlloc = false;

		VkBuffer m_buffer = VK_NULL_HANDLE;
		VkDeviceMemory m_deviceMemory = VK_NULL_HANDLE;

		VkDeviceSize m_size = 0;
		VkBufferUsageFlags m_usage;
		VkMemoryPropertyFlags m_memoryPropertyFlags;
	};

	class Image {
	public:
		Image() {}

		~Image();

		operator VkImage() { return m_image; }

		void init();

		void allocate(VkMemoryPropertyFlags memoryProperties);

		void initView();

		void changeLayout(VkImageLayout layout, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask);

		void setType(VkImageType type) { m_type = type; }

		void setFormat(VkFormat format) { m_format = format; }

		void setAspect(VkImageAspectFlags aspect) { m_aspect = aspect; }

		void setUsage(VkImageUsageFlags usage) { m_usage = usage; }

		void setInitialLayout(VkImageLayout layout) { m_currentLayout = layout; }

		void setExtent(uint32_t width, uint32_t height, uint32_t depth) { m_extent = { width, height, depth }; }
		void setExtent(VkExtent3D extent) { m_extent = extent; }

		void setWidth(uint32_t width) { m_extent.width = width; }

		void setHeight(uint32_t height) { m_extent.height = height; }

		void setDepth(uint32_t depth) { m_extent.depth = depth; }

		VkImage getVkImage() { return m_image; }

		VkDeviceMemory getVkDeviceMemory() { return m_deviceMemory; }

		VkImageView getVkImageView() { return m_imageView; }

	private:
		bool m_isInit = false;
		bool m_isAlloc = false;
		bool m_isViewInit = false;

		VkImage m_image = VK_NULL_HANDLE;
		VkDeviceMemory m_deviceMemory = VK_NULL_HANDLE;
		VkImageView m_imageView = VK_NULL_HANDLE;

		VkImageType m_type = VK_IMAGE_TYPE_2D;
		VkImageViewType m_viewType = VK_IMAGE_VIEW_TYPE_2D;
		VkFormat m_format = VK_USED_SCREENCOLOR_FORMAT;
		VkImageAspectFlags m_aspect = VK_IMAGE_ASPECT_NONE;
		VkExtent3D m_extent;
		uint32_t m_mipLevelCount = 1;
		VkSampleCountFlagBits m_samples = VK_SAMPLE_COUNT_1_BIT;
		VkImageTiling m_tiling = VK_IMAGE_TILING_OPTIMAL;
		VkImageUsageFlags m_usage;
		VkImageLayout m_currentLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;

	};
	
	class Surface {
	public:
		Surface(){}

		~Surface();

		operator VkSurfaceKHR() { return m_surface; }

		void init();

		void setGLFWwindow(GLFWwindow* window) { m_GLFWwindow = window; }

		VkSurfaceKHR getVkSurfaceKHR() { return m_surface; }

	private:
		bool m_isInit = false;

		VkSurfaceKHR m_surface = VK_NULL_HANDLE;
		GLFWwindow* m_GLFWwindow = nullptr;
	};

	class Swapchain {
	public:
		Swapchain();

		~Swapchain();

		operator VkSwapchainKHR() { return m_swapchain; }

		void init();

		void update();

		void setSurface(VkSurfaceKHR surface) { m_surface = surface; }
		void setSurface(vk::Surface& surface) { m_surface = surface.getVkSurfaceKHR(); }

		void setWidth(uint32_t width) { m_imageExtent.width = width; }

		void setHeight(uint32_t height) { m_imageExtent.height = height; }

		void setPresentMode(VkPresentModeKHR presentMode) { m_presentMode = presentMode; }

		VkSwapchainKHR getVkSwapchainKHR() { return m_swapchain; }

		uint32_t getImageCount() { return m_images.size(); }

		VkImage getImage(uint32_t index);

		VkImageSubresourceRange getImageSubresourceRange() { return m_imageSubresourceRange; }

		VkImageView getImageView(uint32_t index);

	private:
		bool m_isInit = false;

		VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
		std::vector<VkImage> m_images;
		std::vector<VkImageView> m_imageViews;

		VkSurfaceKHR m_surface = VK_NULL_HANDLE;
		VkImageSubresourceRange m_imageSubresourceRange;
		VkPresentModeKHR m_presentMode = VK_PRESENT_MODE_FIFO_KHR;
		VkExtent2D m_imageExtent;
		VkFormat m_imageFormat = VK_USED_SCREENCOLOR_FORMAT;
	};

	struct Descriptor {//TODO descriptor count
		VkDescriptorType              type;
		VkShaderStageFlags            stages;
		uint32_t                      binding;
		const VkDescriptorImageInfo*  pImageInfo;
		const VkDescriptorBufferInfo* pBufferInfo;
		const VkBufferView*           pTexelBufferView;
	};

	class DescriptorPool {
	public:
		DescriptorPool() {}

		~DescriptorPool();

		operator VkDescriptorPool() { return m_descriptorPool; }

		void init();

		void update();

		void addDescriptorSet();

		void addDescriptor(const Descriptor& descriptor, uint32_t setIndex);

		uint32_t getDescriptorSetCount() { return m_descriptorSetCount; }

		 VkDescriptorPool getVkDescriptorPool() { return m_descriptorPool; }

		VkDescriptorSet getVkDescriptorSet(int index) { return m_pDescriptorSets[index]; }

		VkDescriptorSetLayout getVkDescriptorSetLayout(int index) { return m_pDescriptorSetLayouts[index]; }

	private:
		bool m_isInit = false;

		VkDescriptorPool m_descriptorPool;
		uint32_t m_descriptorSetArrayLength = 0;
		VkDescriptorSet* m_pDescriptorSets = nullptr;
		VkDescriptorSetLayout* m_pDescriptorSetLayouts = nullptr;

		uint32_t m_descriptorSetCount = 0;
		std::vector<VkDescriptorPoolSize> m_poolSizes;
		std::vector<VkDescriptorSetLayoutCreateInfo> m_setLayoutCreateInfos;
		std::vector<std::vector<VkDescriptorSetLayoutBinding>> m_setLayoutCreateInfoBindings;
		std::vector<VkWriteDescriptorSet> m_writeDescriptorSets;
		std::vector<uint32_t> m_writeDescriptorSetIndices;
	};

	class Shader {
	public:
		Shader();

		~Shader();

		void init();

		void setStage(VkShaderStageFlagBits stage) { m_shaderStage.stage = stage; }
		VkPipelineShaderStageCreateInfo getShaderStage() { return m_shaderStage; }

		VkShaderModule getModule() { return m_module; }

		void setPath(std::string path) { m_path = path; }

		std::string getPath() { return m_path; }

		static void compile(std::string srcDir, std::vector<std::string> srcNames, std::vector<std::string> dstDirs);

	private:
		bool m_isInit = false;

		std::string m_path = "";
		VkShaderModule m_module = VK_NULL_HANDLE;

		VkPipelineShaderStageCreateInfo m_shaderStage{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0};
	};

	class RenderPass {
	public:
		RenderPass();

		~RenderPass();

		operator VkRenderPass() { return m_renderPass; }

		void init();

		void addAttachmentDescription(const VkAttachmentDescription& description);

		void addAttachmentReference(VkAttachmentReference** referencePtr);

		void addSubpassDescription(const VkSubpassDescription& description);

		void addSubpassDependency(const VkSubpassDependency& dependency);

		VkRenderPass getVkRenderPass() { return m_renderPass; }

	private:
		bool m_isInit = false;

		VkRenderPass m_renderPass;

		std::vector<VkAttachmentDescription> m_attachmentDescriptions;
		std::vector<VkAttachmentReference*> m_attachmentReferencePtrs;
		std::vector<VkSubpassDescription> m_subpassDescriptions;
		std::vector<VkSubpassDependency> m_subpassDependencies;
	};

	class Framebuffer {
	public:
		Framebuffer();

		~Framebuffer();

		operator VkFramebuffer() { return m_framebuffer; }

		void init();

		void addAttachment(VkImageView attachment) { m_attachments.push_back(attachment); }

		void delAttachment(int index) { m_attachments.erase(m_attachments.begin() + index); }

		void setRenderPass(VkRenderPass renderPass) { m_renderPass = renderPass; }
		void setRenderPass(vk::RenderPass& renderPass){ setRenderPass(renderPass.getVkRenderPass()); }

		void setWidth(uint32_t width) { m_width = width; }

		void setHeight(uint32_t height) { m_height = height; }

		uint32_t getWidth() { return m_width; }

		uint32_t getHeight() { return m_height; }

		VkFramebuffer getVkFramebuffer() { return m_framebuffer; }

	private:
		bool m_isInit = false;

		VkFramebuffer m_framebuffer;

		VkRenderPass m_renderPass;
		std::vector<VkImageView> m_attachments;
		uint32_t m_width, m_height;
	};

	class Pipeline {
	public:
		Pipeline();
		~Pipeline();

		operator VkPipeline() { return m_pipeline; }

		void init();

		void addShader(const VkPipelineShaderStageCreateInfo& shaderStage);

		void delShader(int index);

		void addVertexInputBindingDescription(const VkVertexInputBindingDescription& vertexInputBindingDescription);

		void delVertexInputBindingDescription(int index);

		void addVertexInputAttrubuteDescription(const VkVertexInputAttributeDescription& vertexInputAttributeDescription);

		void delVertexInputAttrubuteDescription(int index);

		void addDescriptorSetLayout(VkDescriptorSetLayout setLayout) { m_setLayouts.push_back(setLayout); }

		void delDescriptorSetLayout(int index) { m_setLayouts.erase(m_setLayouts.begin()+index); }

		void addViewport(const VkViewport& viewport);

		void delViewport(int index);
		
		void addScissor(const VkRect2D& scissor);

		void delScissor(int index);

		void addDynamicState(VkDynamicState dynamicState) { m_dynamicStates.push_back(dynamicState); }

		void delDynamicState(int index) { m_dynamicStates.erase(m_dynamicStates.begin() + index); }

		void setRenderPass(VkRenderPass renderPass) { m_renderPass = renderPass; }
		void setRenderPass(vk::RenderPass& renderPass) { setRenderPass(renderPass.getVkRenderPass()); }

		void enableDepthTest();

		void disableDepthTest();

		VkPipeline getVkPipeline() { return m_pipeline; }

		VkPipelineLayout getVkPipelineLayout() { return m_pipelineLayout; }

	private:
		bool m_isInit = false;

		VkPipeline       m_pipeline;
		VkPipelineLayout m_pipelineLayout;

		VkRenderPass m_renderPass;
		std::vector<VkDescriptorSetLayout> m_setLayouts;
		std::vector<VkPipelineShaderStageCreateInfo> m_shaderStages;
		std::vector<VkVertexInputBindingDescription> m_vertexInputBindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> m_vertexInputAttributeDescriptions;
		std::vector<VkViewport> m_viewports;
		std::vector<VkRect2D> m_scissors;
		std::vector<VkDynamicState> m_dynamicStates;

		VkPipelineVertexInputStateCreateInfo   m_vertexInputStateCreateInfo;
		VkPipelineInputAssemblyStateCreateInfo m_inputAssemblyStateCreateInfo;
		VkPipelineViewportStateCreateInfo      m_viewportStateCreateInfo;
		VkPipelineRasterizationStateCreateInfo m_rasterizationStateCreateInfo;
		VkPipelineMultisampleStateCreateInfo   m_multisampleStateCreateInfo;
		VkPipelineDepthStencilStateCreateInfo  m_depthStencilStateCreateInfo;
		VkPipelineColorBlendAttachmentState    m_colorBlendAttachment;
		VkPipelineColorBlendStateCreateInfo    m_colorBlendStateCreateInfo;
		VkPipelineDynamicStateCreateInfo       m_dynamicStateCreateInfo;
	};

	void acquireNextImage(VkSwapchainKHR swapchain, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex);
	void acquireNextImage(Swapchain& swapchain, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex);

	void queuePresent(VkQueue queue, VkSwapchainKHR swapchain, uint32_t imageIndex);
	void queuePresent(VkQueue queue, Swapchain& swapchain, uint32_t imageIndex);
	void queuePresent(VkQueue queue, VkSwapchainKHR swapchain, uint32_t imageIndex, VkSemaphore waitSemaphore);
	void queuePresent(VkQueue queue, Swapchain& swapchain, uint32_t imageIndex, VkSemaphore waitSemaphore);

	void deviceWaitIdle();
	void allQueuesWaitIdle();

	void createSemaphore(VkSemaphore* semaphore);
	void destroySemaphore(VkSemaphore semaphore);

	void createFence(VkFence* fence);
	void destroyFence(VkFence fence);

	void waitForFence(VkFence fence);
}

void initVulkan(const char* applicationName);

void terminateVulkan();

void printStats();
