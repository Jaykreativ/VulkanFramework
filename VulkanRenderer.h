#pragma once

#include <iostream>
#include <chrono>

#include "glm.hpp"
#include "VulkanUtils.h"

#include "Vertex.h"

#define PRINT_PHYSICAL_DEVICES true
#define PRINT_QUEUE_FAMILIES  true
#define PRINT_AVAILABLE_DEVICE_EXTENSIONS false
#define PRINT_AVAILABLE_INSTANCE_EXTENSIONS false
#define PRINT_AVAILABLE_INSTANCE_LAYERS false

#define VK_INDEX_OF_USED_PHYSICAL_DEVICE 0
#define VK_PREFERED_QUEUE_FAMILY 0
#define VK_PREFERED_AMOUNT_OF_QUEUES 4
#define VK_USED_SCREENCOLOR_FORMAT VK_FORMAT_B8G8R8A8_UNORM //TODO civ

struct UniformBufferObject {
	glm::vec4 color;
	glm::mat4 transform;
	glm::mat4 view;
	glm::mat4 perspective;
};

namespace vkRenderer {
	void acquireNextImage(VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex);

	void queuePresent(VkQueue queue, uint32_t imageIndex);

	void allQueuesWaitIdle();

	//getter
	const std::vector<VkImage>& getSwapchainImages();
	const std::vector<VkImageView>& getSwapchainImageViews();

	void createSemaphore(VkSemaphore* semaphore);
	void destroySemaphore(VkSemaphore semaphore);

	void createRenderPass(const std::vector<VkAttachmentDescription>& attachmentDescriptions, const std::vector<VkSubpassDescription>& subpassDescriptions, const std::vector<VkSubpassDependency>& subpassDependencies, VkRenderPass& renderPass);
	void destroyRenderPass(VkRenderPass renderPass);

	void createDefaultRenderPass(VkRenderPass& renderPass);

	void createFramebuffer(VkRenderPass& renderPass, const std::vector<VkImageView>& attachments, uint32_t width, uint32_t height, VkFramebuffer& framebuffer);
	void destroyFramebuffer(VkFramebuffer framebuffer);

	void createPipeline(VkDevice& device, VkViewport& viewport, VkRect2D& scissor, VkDescriptorSetLayout& descriptorSetLayout, VkRenderPass& renderPass, VkShaderModule vertShader, VkShaderModule fragShader, VkPipelineLayout& layout, VkPipeline& pipeline);

	class CommandBuffer {
	public:
		CommandBuffer();
		CommandBuffer(bool autoAllocate);
		~CommandBuffer();

		void allocate();

		void begin(VkCommandBufferUsageFlags usageFlags);

		void end();

		void submit();
		void submit(VkQueue* queue);

		void addWaitSemaphore(VkSemaphore waitSemaphore, VkPipelineStageFlags waitDstStageMask) {
			m_waitSemaphores.push_back(waitSemaphore);
			m_waitDstStageMasks.push_back(waitDstStageMask);
		}
		void delWaitSemaphore(int index) {
			m_waitSemaphores.erase(m_waitSemaphores.begin() + index);
			m_waitDstStageMasks.erase(m_waitDstStageMasks.begin() + index);
		}

		const VkCommandBuffer& getVkCommandBuffer() {
			return m_commandBuffer;
		}

	private:
		VkCommandBuffer m_commandBuffer;

		std::vector<VkSemaphore> m_waitSemaphores;
		std::vector<VkPipelineStageFlags> m_waitDstStageMasks;
	};

	class Buffer {
	public:
		Buffer(VkDeviceSize size, VkBufferUsageFlags usage)
			: m_size(size), m_usage(usage)
		{}
		~Buffer();

		void init();

		void allocate(VkMemoryPropertyFlags memoryPropertyFlags);

		void map(void** ptr);
		void map(VkDeviceSize offset, void** ptr);

		void unmap();

		void uploadData(uint32_t size, void* data);

		const VkBuffer& getVkBuffer() {
			return m_buffer;
		}
		const VkDeviceMemory& getDeviceMemory() {
			return m_deviceMemory;
		}
		VkDeviceSize getSize() {
			return m_size;
		}
		VkBufferUsageFlags getUsage() {
			return m_usage;
		}
		VkMemoryPropertyFlags getMemoryPropertyFlags() {
			return m_memoryPropertyFlags;
		}

		static void copyBuffer(VkBuffer dst, VkBuffer src, VkDeviceSize size);

	private:
		bool m_isInit = false;

		VkDeviceSize m_size;
		VkBufferUsageFlags m_usage;
		VkBuffer m_buffer;
		VkMemoryPropertyFlags m_memoryPropertyFlags;
		VkDeviceMemory m_deviceMemory;
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

		void init();

		void addDescriptorSet();

		void addDescriptor(const Descriptor& descriptor, uint32_t setIndex);

		const VkDescriptorPool& getDescriptorPool() {
			return m_descriptorPool;
		}

		const VkDescriptorSet& getDescriptorSet(int index) {
			return m_pDescriptorSets[index];
		}

		const VkDescriptorSetLayout& getDescriptorSetLayout(int index) {
			return m_pDescriptorSetLayouts[index];
		}

	private:
		bool m_isInit = false;

		VkDescriptorPool m_descriptorPool;
		uint32_t m_descriptorSetCount = 0;
		VkDescriptorSetLayout* m_pDescriptorSetLayouts = nullptr;
		VkDescriptorSet* m_pDescriptorSets = nullptr;

		std::vector<VkDescriptorPoolSize> m_poolSizes;
		std::vector<VkDescriptorSetLayoutCreateInfo> m_setLayoutCreateInfos;
		std::vector<std::vector<VkDescriptorSetLayoutBinding>> m_setLayoutCreateInfoBindings;
		std::vector<VkWriteDescriptorSet> m_writeDescriptorSets;
		std::vector<uint32_t> m_writeDescriptorSetIndices;



	};

	class Shader {
	public:
		Shader(std::string path);
		~Shader();

		void init();

		void setStage(VkShaderStageFlagBits stage) {
			m_shaderStage.stage = stage;
		}
		VkPipelineShaderStageCreateInfo getShaderStage() {
			return m_shaderStage;
		}

		VkShaderModule getModule() {
			return m_module;
		}

		void setPath(std::string path) {
			m_path = path;
		}
		std::string getPath() {
			return m_path;
		}

	private:
		bool m_isInit = false;

		std::string m_path;
		VkShaderModule m_module;

		VkPipelineShaderStageCreateInfo m_shaderStage{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0};
	};

	class RenderPass {
	public:
		RenderPass();
		RenderPass(bool generateDefault);
		~RenderPass();

		void init();

		const VkRenderPass& getVkRenderPass() {
			return m_renderPass;
		}
		VkRenderPass& getVkRenderPassRef() {
			return m_renderPass;
		}

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

		void init();

		void addAttachment(VkImageView attachment) {
			m_attachments.push_back(attachment);
		}
		void delAttachment(int index) {
			m_attachments.erase(m_attachments.begin() + index);
		}

		void setRenderPass(VkRenderPass renderPass) {
			m_renderPass = renderPass;
		}
		void setRenderPass(vkRenderer::RenderPass& renderPass){
			setRenderPass(renderPass.getVkRenderPass());
		}

		void setWidth(uint32_t width) {
			m_width = width;
		}
		void setHeight(uint32_t height) {
			m_height = height;
		}

		VkFramebuffer getVkFramebuffer() {
			return m_framebuffer;
		}

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

		void init();

		void addShader(const VkPipelineShaderStageCreateInfo& shaderStage);
		void delShader(int index);

		void addVertexInputBindingDescription(const VkVertexInputBindingDescription& vertexInputBindingDescription);
		void delVertexInputBindingDescription(int index);

		void addVertexInputAttrubuteDescription(const VkVertexInputAttributeDescription& vertexInputAttributeDescription);
		void delVertexInputAttrubuteDescription(int index);

		void addDescriptorSetLayout(VkDescriptorSetLayout setLayout) {
			m_setLayouts.push_back(setLayout);
		}
		void delDescriptorSetLayout(int index) {
			m_setLayouts.erase(m_setLayouts.begin()+index);
		}

		void addViewport(const VkViewport& viewport);
		void delViewport(int index);
		
		void addScissor(const VkRect2D& scissor);
		void delScissor(int index);

		void setRenderPass(VkRenderPass renderPass) {
			m_renderPass = renderPass;
		}
		void setRenderPass(vkRenderer::RenderPass& renderPass) {
			setRenderPass(renderPass.getVkRenderPass());
		}

		const VkPipeline& getVkPipeline() {
			return m_pipeline;
		}

		const VkPipelineLayout& getPipelineLayout() {
			return m_pipelineLayout;
		}

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

	void allocateCommandBuffers(uint32_t countCommandBuffers, VkCommandBuffer* commandBuffers);

	void createVertexBuffer(std::vector<Vertex>& vertexArray, VkDeviceMemory& deviceMemory, VkBuffer& vertexBuffer);
}

void initGLFW(GLFWwindow*& window, int width, int height, const char* title);

void terminateGLFW(GLFWwindow* window);

void initVulkan(GLFWwindow* window, uint32_t width, uint32_t height, const char* applicationName, UniformBufferObject &ubo, vkRenderer::Shader &vertShader, vkRenderer::Shader &fragShader, std::vector<Vertex> &vertexArray, std::vector<uint32_t> &indexArray);

void terminateVulkan(vkRenderer::Shader &vertShader, vkRenderer::Shader &fragShader);

void printStats();

void updateUniform(UniformBufferObject &ubo);

void drawFrame();
