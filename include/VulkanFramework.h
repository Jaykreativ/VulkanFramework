#pragma once

#include <iostream>
#include <chrono>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include <unordered_map>
#include <vector>

//define extension functions
extern PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR_;
#define vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR_
extern PFN_vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandlesKHR_;
#define vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandlesKHR_
extern PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR_;
#define vkCmdTraceRaysKHR vkCmdTraceRaysKHR_
extern PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR_;
#define vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR_
extern PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR_;
#define vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR_
extern PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR_;
#define vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR_
extern PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR_;
#define vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR_
extern PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR_;
#define vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR_

#define PRINT_PHYSICAL_DEVICES true
#define PRINT_QUEUE_FAMILIES  false
#define PRINT_AVAILABLE_DEVICE_EXTENSIONS false
#define PRINT_AVAILABLE_INSTANCE_EXTENSIONS false
#define PRINT_AVAILABLE_INSTANCE_LAYERS false

#define VK_PREFERED_QUEUE_FAMILY 0
#define VK_PREFERED_AMOUNT_OF_QUEUES 1
#define VK_MIN_AMOUNT_OF_SWAPCHAIN_IMAGES 3
#define VK_USED_SCREENCOLOR_FORMAT VK_FORMAT_B8G8R8A8_UNORM //TODO civ

namespace vk
{
	class Registery; // forwarded decleration

	class Registerable {
	public:
		Registerable() = default;
		virtual ~Registerable();

		virtual void init();

		virtual void update();
		
		virtual void destroy();
	private:


		Registery* m_registery = nullptr;

		friend class Registery;
	};

	enum RegisteryFunction {
		eUPDATE = 0x0,
		eINIT = 0x1,
		eDESTROY = 0x2
	};

	// args: ( obj, dependency, functionType )
	typedef void(*RegisteryCallback)(Registerable*, Registerable*, RegisteryFunction);

	class Registery {
	public:
		Registery();
		~Registery();

		/*
		Connect an registerable object with a dependency
		
		WARNING:
		Can't be used twice with the same pointers
		An object can be registered with only one registery
		*/
		void connect(Registerable* obj, Registerable* dependency, RegisteryCallback callback);

	private:
		std::unordered_map<Registerable*, std::vector<std::pair<Registerable*, RegisteryCallback>>> m_objConnectionMap; // callback is specific to the connection
		std::unordered_map<Registerable*, std::vector<Registerable*>> m_dependencyObjMap;

		friend class Registerable;
	};

	class CommandBuffer {
	public:
		CommandBuffer();
		CommandBuffer(bool autoAllocate);

		~CommandBuffer();

		operator VkCommandBuffer() const { return m_commandBuffer; }

		void allocate();

		void free();

		void begin(VkCommandBufferUsageFlags usageFlags);

		void end();

		void submit(VkQueue* queue, VkFence fence, uint32_t waitSemaphoreCount, VkSemaphore* waitSemaphores, VkPipelineStageFlags* waitDstStageMask, uint32_t signalSemaphoreCount, VkSemaphore* signalSemaphores);
		void submit(VkQueue* queue, VkFence fence);
		void submit(VkFence fence);
		void submit(VkQueue* queue);
		void submit();

		void addWaitSemaphore(VkSemaphore waitSemaphore, VkPipelineStageFlags waitDstStageMask);

		void delWaitSemaphore(int index);

		void addSignalSemaphore(VkSemaphore signalSemaphore) { m_signalSemaphores.push_back(signalSemaphore); }
		void delSignalSemaphore(int index) { m_signalSemaphores.erase(m_signalSemaphores.begin() + index); }

		const VkCommandBuffer& getVkCommandBuffer() { return m_commandBuffer; }

	private:
		bool m_isAlloc = false;

		VkCommandBuffer m_commandBuffer;

		std::vector<VkSemaphore> m_waitSemaphores;
		std::vector<VkPipelineStageFlags> m_waitDstStageMasks;
		std::vector<VkSemaphore> m_signalSemaphores;
	};

	class Buffer : public Registerable{
	public:
		Buffer();
		Buffer(VkDeviceSize size, VkBufferUsageFlags usage);

		~Buffer();

		Buffer& operator=(const Buffer& other);

		operator VkBuffer() { return m_buffer; }

		void init();

		void destroy();

		void update();

		void allocate(VkMemoryPropertyFlags memoryPropertyFlags);

		void free();

		void map(void** ptr);
		void map(VkDeviceSize offset, void** ptr);

		void resize(VkDeviceSize size) { m_size = size; } // TODO make update automatic and add a setSize method

		void unmap();

		void uploadData(vk::Buffer* buffer);
		void uploadData(uint32_t size, void* data);

		void setUsage(VkBufferUsageFlags usage) { m_usage = usage; }

		VkBuffer getVkBuffer() { return m_buffer; }

		VkDeviceMemory getVkDeviceMemory() { return m_deviceMemory; }

		VkDeviceAddress getVkDeviceAddress();

		VkDeviceSize getSize() { return m_size; }

		VkBufferUsageFlags getUsage() { return m_usage; }

		VkMemoryPropertyFlags getMemoryPropertyFlags() { return m_memoryPropertyFlags; }

		static VkDeviceAddress getBufferVkDeviceAddress(VkBuffer buffer);

		static void copyBuffer(vk::Buffer* dst, vk::Buffer* src, VkDeviceSize size);

	private:
		bool m_isInit = false;
		bool m_isAlloc = false;

		VkBuffer m_buffer = VK_NULL_HANDLE;
		VkDeviceMemory m_deviceMemory = VK_NULL_HANDLE;

		VkDeviceSize m_size = 0;
		VkBufferUsageFlags m_usage;
		VkMemoryPropertyFlags m_memoryPropertyFlags;
	};

	class Image : public Registerable {
	public:
		Image() {}

		~Image();

		operator VkImage() { return m_image; }

		void init();

		void allocate(VkMemoryPropertyFlags memoryProperties);

		void initView();

		void destroy();

		void free();

		void destroyView();

		void destroySampler();

		void update();

		void resize(uint32_t width, uint32_t height, uint32_t depth = 1);

		void uploadData(uint32_t size, void* data);

		void cmdChangeLayout(VkCommandBuffer cmd, VkImageLayout layout, VkAccessFlags dstAccessMask);
		void changeLayout(VkImageLayout layout, VkAccessFlags dstAccessMask);

		void setType(VkImageType type) { m_type = type; }

		void setFormat(VkFormat format) { m_format = format; }

		void setAspect(VkImageAspectFlags aspect) { m_aspect = aspect; }

		void setUsage(VkImageUsageFlags usage) { m_usage = usage; }

		void setInitialLayout(VkImageLayout layout) { m_currentLayout = layout; }

		void setExtent(uint32_t width, uint32_t height, uint32_t depth) { m_extent = { width, height, depth }; }
		void setExtent(VkExtent3D extent) { m_extent = extent; }

		void setWidth(uint32_t width) { m_extent.width = std::max<uint32_t>(width, 1); }

		void setHeight(uint32_t height) { m_extent.height = std::max<uint32_t>(height, 1); }

		void setDepth(uint32_t depth) { m_extent.depth = std::max<uint32_t>(depth, 1); }

		void setMemoryProperties(VkMemoryPropertyFlags memoryProperties) {
			m_memoryProperties = memoryProperties;
		}

		VkImage getVkImage() { return m_image; }

		VkDeviceMemory getVkDeviceMemory() { return m_deviceMemory; }

		VkImageView getVkImageView() { return m_imageView; }

		VkImageSubresourceRange getSubresourceRange() { return m_subresourceRange; }

		VkImageLayout getLayout() { return m_currentLayout; }

		VkImageAspectFlags getAspect() { return m_aspect; }

		uint32_t getMipLevelCount() { return m_mipLevelCount; }

		VkExtent3D getExtent() { return m_extent; }

		static void copyBufferToImage(vk::Image* dst, vk::Buffer* src, VkDeviceSize size);

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
		VkExtent3D m_extent = {1, 1, 1};
		uint32_t m_mipLevelCount = 1;
		VkSampleCountFlagBits m_samples = VK_SAMPLE_COUNT_1_BIT;
		VkImageTiling m_tiling = VK_IMAGE_TILING_OPTIMAL;
		VkImageUsageFlags m_usage;
		VkImageSubresourceRange m_subresourceRange;
		VkImageLayout m_currentLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
		VkMemoryPropertyFlags m_memoryProperties = 0;
		VkAccessFlags m_accessMask = 0;
	};
	
	class Sampler {
	public:
		Sampler();
		~Sampler();

		operator VkSampler() { return m_sampler; }

		void init();

		void destroy();

	private:
		VkSampler m_sampler = VK_NULL_HANDLE;

		VkSamplerCreateInfo m_createInfo = {
			VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			nullptr,
			0,
			VK_FILTER_LINEAR,
			VK_FILTER_LINEAR,
			VK_SAMPLER_MIPMAP_MODE_LINEAR,
			VK_SAMPLER_ADDRESS_MODE_REPEAT, // outside image bounds just use border color
			VK_SAMPLER_ADDRESS_MODE_REPEAT,
			VK_SAMPLER_ADDRESS_MODE_REPEAT,
			0,
			false,
			1.0f,
			false,
			VK_COMPARE_OP_NEVER,
			-1000,
			1000,
			VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK
		};
	};

	class Surface {
	public:
		Surface(){}

		~Surface();

		operator VkSurfaceKHR() const { return m_surface; }

		void init();

		void setGLFWwindow(GLFWwindow* window) { m_GLFWwindow = window; }

		VkSurfaceKHR getVkSurfaceKHR() const { return m_surface; }

	private:
		bool m_isInit = false;

		VkSurfaceKHR m_surface = VK_NULL_HANDLE;
		GLFWwindow* m_GLFWwindow = nullptr;
	};

	class Swapchain {
	public:
		Swapchain();

		~Swapchain();

		operator VkSwapchainKHR() const { return m_swapchain; }

		void init();

		void update();

		void setSurface(VkSurfaceKHR surface) { m_surface = surface; }
		void setSurface(vk::Surface& surface) { m_surface = surface.getVkSurfaceKHR(); }

		void setWidth(uint32_t width) { m_imageExtent.width = width; }

		void setHeight(uint32_t height) { m_imageExtent.height = height; }

		void setPresentMode(VkPresentModeKHR presentMode) { m_presentMode = presentMode; }

		VkSwapchainKHR getVkSwapchainKHR() const { return m_swapchain; }

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

	struct DescriptorImageInfo {
		Image* pImage;
		Sampler* pSampler;
		VkImageLayout imageLayout;
	};

	struct DescriptorBufferInfo {
		Buffer* pBuffer;
		uint32_t offset;
		uint32_t range;
	};

	struct Descriptor {
		const void*                   pNext;
		VkDescriptorType              type;
		uint32_t                      count = 1;
		VkShaderStageFlags            stages;
		uint32_t                      binding;

		std::vector<DescriptorImageInfo>    imageInfos = {};
		std::vector<DescriptorBufferInfo>   bufferInfos = {};
		std::vector<VkBufferView>           texelBufferViews = {};
	};

	class DescriptorPool; //forward decleration

	class DescriptorSet : public Registerable {
	public:
		DescriptorSet();
		~DescriptorSet();

		operator VkDescriptorSet() const { return m_descriptorSet; }

		void init();

		void allocate();

		void update();

		void destroy();

		void free();

		void addDescriptor(Descriptor descriptor);

		void eraseDescriptor(uint32_t index);

		void eraseDescriptors(uint32_t offset, uint32_t range);

		void setDescriptor(uint32_t index, Descriptor descriptor);

		void setDescriptorPool(const DescriptorPool* descriptorPool);

		Descriptor getDescriptor(uint32_t index);

		VkDescriptorSetLayout getVkDescriptorSetLayout() const { return m_descriptorSetLayout; }

	private:
		VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;
		VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;

		const DescriptorPool* m_pDescriptorPool = nullptr;

		std::vector<Descriptor> m_descriptors = {};

		friend class DescriptorPool;
	};

	class DescriptorPool {
	public:
		DescriptorPool();
		~DescriptorPool();

		operator VkDescriptorPool() const { return m_descriptorPool; }

		void init();

		void update();

		void destroy();

		void addDescriptorSet(DescriptorSet& descriptorSet);

		void setMaxSets(uint32_t maxSets);

		void addPoolSize(VkDescriptorPoolSize poolSize);
		void addPoolSize(VkDescriptorType type, uint32_t count);
		void addPoolSizes(VkDescriptorPoolSize* poolSize, uint32_t poolSizeCount);

	private:
		VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;

		uint32_t m_maxSets = 0;
		std::vector<VkDescriptorPoolSize> m_poolSizes = {};
	};
	
	/*class DescriptorPool {
	public:
		DescriptorPool() {}

		~DescriptorPool();

		operator VkDescriptorPool() { return m_descriptorPool; }

		void init();

		void update();

		void addDescriptorSet();

		void updateDescriptorSet();

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
	};*/

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

		void destroy();

		void update();

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

		void addPushConstantRange(VkPushConstantRange pushConstantRange) { m_pushConstantRanges.push_back(pushConstantRange); }
		
		void delPushConstantRange(int index) { m_pushConstantRanges.erase(m_pushConstantRanges.begin() + index); }

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
		std::vector<VkPushConstantRange> m_pushConstantRanges;

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

	void cmdChangeImageLayout(VkCommandBuffer cmd, VkImage image, VkImageSubresourceRange subresourceRange, VkImageLayout currentLayout, VkImageLayout layout, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask);
	void changeImageLayout(VkImage image, VkImageSubresourceRange subresourceRange, VkImageLayout currentLayout, VkImageLayout layout, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask);

	void queuePresent(VkQueue queue, VkSwapchainKHR swapchain, uint32_t imageIndex);
	void queuePresent(VkQueue queue, VkSwapchainKHR swapchain, uint32_t imageIndex, VkSemaphore waitSemaphore);

	void deviceWaitIdle();
	void allQueuesWaitIdle();

	void createSemaphore(VkSemaphore* semaphore);
	void destroySemaphore(VkSemaphore semaphore);

	void createFence(VkFence* fence);
	void destroyFence(VkFence fence);

	void resetFence(VkFence fence);
	void waitForFence(VkFence fence);

	VkInstance getInstance();

	VkPhysicalDevice getPhysicalDevice();

	VkDevice getDevice();

	uint32_t getQueueFamily();

	struct initInfo {
		const char* applicationName;
		std::vector<const char*> requestedInstanceLayers = {};
		//std::vector<const char*> requestedInstanceExtensions = {}; // TODO implement instanceExtensions
		uint32_t    deviceIndex = 0;
		bool checkDeviceSupport = true;
		std::vector<const char*> requestedDeviceLayers = {};
		std::vector<const char*> requestedDeviceExtensions = {};
		VkPhysicalDeviceFeatures2 features = {};
#ifdef _DEBUG
		bool printDebugInfo = true;
#else
		bool printDebugInfo = false;
#endif
	};

	class RtPipeline {
	public:
		RtPipeline();
		~RtPipeline();

		operator VkPipeline() { return m_pipeline; }

		void init();

		void initShaderBindingTable();

		void destroy();

		void addShader(const VkPipelineShaderStageCreateInfo& shaderStage);

		void delShader(uint32_t index);

		void addGroup(const VkRayTracingShaderGroupCreateInfoKHR& group);

		void delGroup(uint32_t index);

		void addDescriptorSetLayout(VkDescriptorSetLayout setLayout);

		void delDescriptorSetLayout(int index);

		VkPipeline getVkPipeline() { return m_pipeline; }

		VkPipelineLayout getVkPipelineLayout() { return m_pipelineLayout; }

		VkStridedDeviceAddressRegionKHR getRayGenRegion() { return m_rgenRegion; }
		VkStridedDeviceAddressRegionKHR getMissRegion() { return m_missRegion; }
		VkStridedDeviceAddressRegionKHR getHitRegion() { return m_hitRegion; }
		VkStridedDeviceAddressRegionKHR getCallRegion() { return m_callRegion; }

	private:
		bool m_isInit = false;

		VkPipeline       m_pipeline;
		VkPipelineLayout m_pipelineLayout;

		//ShaderBindingTable
		Buffer m_rtSBTBuffer;
		VkStridedDeviceAddressRegionKHR m_rgenRegion = {};
		VkStridedDeviceAddressRegionKHR m_missRegion = {};
		VkStridedDeviceAddressRegionKHR m_hitRegion = {};
		VkStridedDeviceAddressRegionKHR m_callRegion = {};

		std::vector<VkDescriptorSetLayout> m_descriptorSetLayouts;
		std::vector<VkPipelineShaderStageCreateInfo> m_stages;
		std::vector<VkRayTracingShaderGroupCreateInfoKHR> m_shaderGroupes;
	};

	class AccelerationStructure; // forward declaration

	class AccelerationStructureInstance {
	public:
		AccelerationStructureInstance();
		AccelerationStructureInstance(AccelerationStructure& accelerationStructure);
		~AccelerationStructureInstance();

		void setTransform(VkTransformMatrixKHR transform);

		void setCustomIndex(uint32_t customIndex);

		void setMask(uint32_t mask);

		void setShaderBindingTableRecordOffset(uint32_t offset);

		void setFlags(VkGeometryInstanceFlagsKHR flags);
	private:
		VkAccelerationStructureInstanceKHR m_instance;

		friend class AccelerationStructure;
	};

	class AccelerationStructure {
	public:
		AccelerationStructure();
		~AccelerationStructure();

		operator VkAccelerationStructureKHR() { return m_accelerationStructure; }

		void init();

		void destroy();

		void update();

		void setType(VkAccelerationStructureTypeKHR type) { m_type = type; }

		void addGeometry(std::vector<AccelerationStructureInstance>& instances);

		void updateGeometry(std::vector<AccelerationStructureInstance>& instances);

		void addGeometry(Buffer& vertexBuffer, uint32_t vertexStride, Buffer& indexBuffer);

		void addGeometry(float aabbMax[3], float aabbMin[3]);

		VkDeviceAddress getDeviceAddress();

		VkAccelerationStructureKHR getVkAccelerationStructureKHR() { return m_accelerationStructure; }
		VkAccelerationStructureKHR* getVkAccelerationStructureKHRptr() { return &m_accelerationStructure; }

	private:
		Buffer m_buffer;
		VkAccelerationStructureKHR m_accelerationStructure;

		VkAccelerationStructureTypeKHR m_type; // Has to be set by user before initializing
		std::vector<vk::Buffer*> m_additionalBuffers; // Buffers like aabb buffers etc.
		std::vector<VkAccelerationStructureGeometryKHR> m_geometryVector;
		std::vector<VkAccelerationStructureBuildRangeInfoKHR> m_buildRangeInfoVector;
		Buffer m_instanceBuffer;
	};
}

void initVulkan(vk::initInfo info);

void terminateVulkan();

void printStats();
