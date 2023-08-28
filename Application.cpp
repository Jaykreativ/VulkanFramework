#define STRINGIFY(x) #x
#define EXPAND(x) STRINGIFY(x)

#ifndef _DEBUG
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif

#include "Application.h"

#include <chrono>

namespace app {
	GLFWwindow* window;

	uint32_t width = 1000;
	uint32_t height = 600;
	const char* TITLE = "Vulkan Application";

	vkRenderer::Buffer vertexBuffer;
	std::vector<Vertex> vertexArray = {
		Vertex({0.5f, -0.5f, 0.5f}),
		Vertex({-0.5f, -0.5f, 0.5f}),
		Vertex({-0.5f, 0.5f, 0.5f}),
		Vertex({0.5f, 0.5f, 0.5f}),
		Vertex({0, 0, -0.5f})
	};

	vkRenderer::Buffer indexBuffer;
	std::vector<uint32_t> indexArray = {
		0, 1, 2,
		0, 2, 3,
		1, 0, 4,
		2, 1, 4,
		3, 2, 4,
		0, 3, 4
	};

	UniformBufferObject ubo = {
		{1, 1, 1, 1},
		glm::mat4(1),
		glm::mat4(1),
		glm::mat4(1),
		{ app::width, app::height }
	};

	vkRenderer::Shader vertShader;
	vkRenderer::Shader fragShader;

	vkRenderer::Surface surface;
	vkRenderer::Swapchain swapchain;

	vkRenderer::Image depthImage;

	vkRenderer::RenderPass renderPass;

	vkRenderer::Framebuffer* framebuffers;

	vkRenderer::DescriptorPool descriptorPool;

	vkRenderer::Buffer uniformBuffer;

	vkRenderer::Pipeline pipeline;

	uint32_t commandBufferCount;
	vkRenderer::CommandBuffer* commandBuffers;

	VkSemaphore imageAvailable;
}

void recordCommandBuffers() {
	for (int i = 0; i < app::swapchain.getImageCount(); i++) {
		VkFramebuffer framebuffer = app::framebuffers[i].getVkFramebuffer();
		vkRenderer::CommandBuffer& commandBuffer = app::commandBuffers[i];
		commandBuffer.begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

		VkRenderPassBeginInfo renderPassBeginInfo;
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext = nullptr;
		renderPassBeginInfo.renderPass = app::renderPass.getVkRenderPass();
		renderPassBeginInfo.framebuffer = framebuffer;
		renderPassBeginInfo.renderArea.offset = { 0,0 };
		renderPassBeginInfo.renderArea.extent = { app::width, app::height };
		VkClearValue colorClearValue = { 0.1f, 0.1f, 0.1f, 1.0f };
		VkClearValue depthStencilClearValue = { 1.0f, 0 };
		std::vector<VkClearValue> clearValues = {
			colorClearValue,
			depthStencilClearValue
		};
		renderPassBeginInfo.clearValueCount = clearValues.size();
		renderPassBeginInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer.getVkCommandBuffer(), &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(commandBuffer.getVkCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, app::pipeline.getVkPipeline());

		VkViewport viewport;
		viewport.x = 0;
		viewport.y = 0;
		viewport.width = app::width;
		viewport.height = app::height;
		viewport.minDepth = 0;
		viewport.maxDepth = 1;
		vkCmdSetViewport(commandBuffer.getVkCommandBuffer(), 0, 1, &viewport);
		VkRect2D scissor;
		scissor.offset = { 0, 0 };
		scissor.extent = { app::width, app::height };
		vkCmdSetScissor(commandBuffer.getVkCommandBuffer(), 0, 1, &scissor);

		uint64_t offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer.getVkCommandBuffer(), 0, 1, &app::vertexBuffer.getVkBuffer(), offsets);
		vkCmdBindIndexBuffer(commandBuffer.getVkCommandBuffer(), app::indexBuffer.getVkBuffer(), 0, VK_INDEX_TYPE_UINT32);

		vkCmdBindDescriptorSets(commandBuffer.getVkCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, app::pipeline.getPipelineLayout(), 0, 1, &app::descriptorPool.getDescriptorSet(0), 0, nullptr);

		vkCmdDrawIndexed(commandBuffer.getVkCommandBuffer(), app::indexArray.size(), 1, 0, 0, 0);

		vkCmdEndRenderPass(commandBuffer.getVkCommandBuffer());

		commandBuffer.end();
	}
}

void resize(GLFWwindow* window, int width, int height) {
	app::width = width;
	app::height = height;
	vkRenderer::deviceWaitIdle();

	delete[] app::commandBuffers;
	delete[] app::framebuffers;

	app::swapchain.setWidth(app::width);
	app::swapchain.setHeight(app::height);
	app::swapchain.update();

	app::depthImage.~Image();
	app::depthImage.setWidth(app::width);
	app::depthImage.setHeight(app::height);
	app::depthImage.init();
	app::depthImage.allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	app::depthImage.initView();

	app::depthImage.changeLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 0, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);

	app::framebuffers = new vkRenderer::Framebuffer[app::commandBufferCount];
	for (int i = 0; i < app::swapchain.getImageCount(); i++) {
		app::framebuffers[i].addAttachment(app::swapchain.getImageView(i));
		app::framebuffers[i].addAttachment(app::depthImage.getVkImageView());
		app::framebuffers[i].setRenderPass(app::renderPass);
		app::framebuffers[i].setWidth(app::width);
		app::framebuffers[i].setHeight(app::height);
		app::framebuffers[i].init();
	}

	app::commandBuffers = new vkRenderer::CommandBuffer[app::commandBufferCount];
	for (int i = 0; i < app::commandBufferCount; i++) {
		app::commandBuffers[i].addWaitSemaphore(app::imageAvailable, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
	}
	recordCommandBuffers();
}

int main() {
	std::string projDir = EXPAND(_PROJDIR);
	projDir.erase(0, 1);
	projDir.erase(projDir.length() - 2);

#if _DEBUG
	std::cout << "Mode: Debug\n";
#else 
	std::cout << "Mode: Release\n";
#endif

	/*uint32_t count = 1;
	const int* array = new int[count] {1};
	int var = 4;
	for (int i = 0; i < count; i++) std::cout << array[i] << "\n";
	std::cout << "\n";
	vkUtils::addArray<int>(array, count, var);
	count++;
	for (int i = 0; i < count; i++) std::cout << array[i] << "\n";

	system("pause");
	return 1;*/

	if (!glfwInit()) std::runtime_error("could not load glfw");


	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_VISIBLE, false);

	app::window = glfwCreateWindow(app::width, app::height, app::TITLE, nullptr, nullptr);

	glfwSetWindowSizeCallback(app::window, resize);

	glfwShowWindow(app::window);

	initVulkan(app::window, app::width, app::height, app::TITLE);

	/*Initialization*/
	app::surface.setGLFWwindow(app::window);
	app::surface.init();

	app::swapchain.setSurface(app::surface);
	app::swapchain.setPresentMode(VK_PRESENT_MODE_MAILBOX_KHR);
	app::swapchain.setWidth(app::width);
	app::swapchain.setHeight(app::height);
	app::swapchain.init();

	{
		app::depthImage.setFormat(VK_FORMAT_D24_UNORM_S8_UINT);
		app::depthImage.setAspect(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
		app::depthImage.setUsage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
		app::depthImage.setInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED);
		app::depthImage.setWidth(app::width);
		app::depthImage.setHeight(app::height);

		app::depthImage.init();
		app::depthImage.allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		app::depthImage.initView();

		app::depthImage.changeLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 0, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);

		VkAttachmentDescription colorAttachmentDescription;
		colorAttachmentDescription.flags = 0;
		colorAttachmentDescription.format = VK_USED_SCREENCOLOR_FORMAT;
		colorAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentDescription depthStencilAttachmentDescription;
		depthStencilAttachmentDescription.flags = 0;
		depthStencilAttachmentDescription.format = VK_FORMAT_D24_UNORM_S8_UINT;
		depthStencilAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
		depthStencilAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthStencilAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthStencilAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthStencilAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthStencilAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthStencilAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference* colorAttachmentReference;
		VkAttachmentReference* depthStencilAttachmentReference;
		{
		VkAttachmentReference tmp1;
		tmp1.attachment = 0;
		tmp1.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachmentReference = &tmp1;

		VkAttachmentReference tmp2;
		tmp2.attachment = 1;
		tmp2.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depthStencilAttachmentReference = &tmp2;

		app::renderPass.addAttachmentDescription(colorAttachmentDescription);
		app::renderPass.addAttachmentDescription(depthStencilAttachmentDescription);
		app::renderPass.addAttachmentReference(&colorAttachmentReference);
		app::renderPass.addAttachmentReference(&depthStencilAttachmentReference);
		}

		VkSubpassDescription description;
		description.flags = 0;
		description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		description.inputAttachmentCount = 0;
		description.pInputAttachments = nullptr;
		description.colorAttachmentCount = 1;
		description.pColorAttachments = colorAttachmentReference;
		description.pResolveAttachments = nullptr;
		description.pDepthStencilAttachment = depthStencilAttachmentReference;
		description.preserveAttachmentCount = 0;
		description.pPreserveAttachments = nullptr;

		app::renderPass.addSubpassDescription(description);

		VkSubpassDependency dependency;
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependency.dependencyFlags = 0;

		app::renderPass.addSubpassDependency(dependency);

		app::renderPass.init();
	}


	app::framebuffers = new vkRenderer::Framebuffer[app::swapchain.getImageCount()];
	for (int i = 0; i < app::swapchain.getImageCount(); i++) {
		app::framebuffers[i].addAttachment(app::swapchain.getImageView(i));
		app::framebuffers[i].addAttachment(app::depthImage.getVkImageView());
		app::framebuffers[i].setRenderPass(app::renderPass);
		app::framebuffers[i].setWidth(app::width);
		app::framebuffers[i].setHeight(app::height);
		app::framebuffers[i].init();
	}

	app::uniformBuffer = vkRenderer::Buffer(sizeof(UniformBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	app::uniformBuffer.init(); app::uniformBuffer.allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	void* data; app::uniformBuffer.map(&data);
	memcpy(data, &app::ubo, sizeof(UniformBufferObject));
	app::uniformBuffer.unmap();

	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = app::uniformBuffer.getVkBuffer();
	bufferInfo.offset = 0;
	bufferInfo.range = app::uniformBuffer.getSize();

	vkRenderer::Descriptor descriptor{};
	descriptor.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptor.stages = VK_SHADER_STAGE_VERTEX_BIT;
	descriptor.binding = 0;
	descriptor.pBufferInfo = &bufferInfo;

	app::descriptorPool.addDescriptorSet();
	app::descriptorPool.addDescriptor(descriptor, 0);

	app::descriptorPool.init();
	
	//Create Pipeline
	{

		app::vertShader.setPath(projDir + "Shader\\shader.vert.spv");
		app::fragShader.setPath(projDir + "Shader\\shader.frag.spv");

		app::vertShader.setStage(VK_SHADER_STAGE_VERTEX_BIT);
		app::fragShader.setStage(VK_SHADER_STAGE_FRAGMENT_BIT);

		app::vertShader.init();
		app::fragShader.init();

		app::pipeline.addShader(app::vertShader.getShaderStage());
		app::pipeline.addShader(app::fragShader.getShaderStage());

		//Add viewport and scissor
		VkViewport viewport;
		viewport.x = 0; viewport.y = 0;
		viewport.width = app::width; viewport.height = app::height;
		viewport.minDepth = 0; viewport.maxDepth = 1;
		app::pipeline.addViewport(viewport);
		VkRect2D scissor;
		scissor.offset = { 0, 0 };
		scissor.extent = { app::width, app::height };
		app::pipeline.addScissor(scissor);

		//Add Vertex input description
		for (int i = 0; i < Vertex::getInputAttributeDescriptions().size(); i++) {
			VkVertexInputAttributeDescription attributeDescription = Vertex::getInputAttributeDescriptions()[i];
			app::pipeline.addVertexInputAttrubuteDescription(attributeDescription);
		}
		app::pipeline.addVertexInputBindingDescription(Vertex::getInputBindingDescription());

		//Add the descriptorSetLayout for the pipelineLayout
		app::pipeline.addDescriptorSetLayout(app::descriptorPool.getDescriptorSetLayout(0));

		//Set renderPass which is compatible with the one the pipeline is used in
		app::pipeline.setRenderPass(app::renderPass);

		app::pipeline.addDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
		app::pipeline.addDynamicState(VK_DYNAMIC_STATE_SCISSOR);

		app::pipeline.enableDepthTest();

		//Initialize pipeline
		app::pipeline.init();
	}

	app::vertexBuffer = vkRenderer::Buffer(app::vertexArray.size() * sizeof(Vertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	app::vertexBuffer.init(); app::vertexBuffer.allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	app::vertexBuffer.uploadData(app::vertexArray.size() * sizeof(Vertex), app::vertexArray.data());
	
	app::indexBuffer = vkRenderer::Buffer(app::indexArray.size() * sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	app::indexBuffer.init(); app::indexBuffer.allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	app::indexBuffer.uploadData(app::indexArray.size() * sizeof(uint32_t), app::indexArray.data());

	vkRenderer::createSemaphore(&app::imageAvailable);

	app::commandBufferCount = app::swapchain.getImageCount();//Allocate CommandBuffers for recording
	app::commandBuffers = new vkRenderer::CommandBuffer[app::commandBufferCount];
	for (int i = 0; i < app::swapchain.getImageCount(); i++) {
		app::commandBuffers[i].allocate();
		app::commandBuffers[i].addWaitSemaphore(app::imageAvailable, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
	}

	recordCommandBuffers();

	printStats();
	
	float a = 0;
	float rotation = 0;

	double deltaTime = 0;
	while (!glfwWindowShouldClose(app::window)) {
		auto startFrame = std::chrono::system_clock::now();
		if (a > 1) a = 0;
		a += 0.5*deltaTime;

		rotation += glm::radians(10.0f)*deltaTime;

		app::ubo.color = glm::vec4(a, 1, 0.5, 1);
		app::ubo.transform = glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0.0f, 0.0f, 1.0f));
		app::ubo.view = glm::lookAt(glm::vec3(0, 1, 0.25f), glm::vec3(0, 0, 0), glm::vec3(0, 0, 1));
		app::ubo.perspective = glm::perspective<float>(glm::radians(120.0f), app::width / static_cast<float>(app::height), 0.1f, 100);
		app::ubo.viewport = glm::vec2(app::width, app::height);

		void* data;  app::uniformBuffer.map(&data);
		memcpy(data, &app::ubo, sizeof(UniformBufferObject));
		app::uniformBuffer.unmap();
		
		uint32_t imageIndex = 0;
		vkRenderer::acquireNextImage(app::swapchain, app::imageAvailable, VK_NULL_HANDLE, &imageIndex);

		VkQueue queue;
		app::commandBuffers[imageIndex].submit(&queue);

		vkRenderer::queuePresent(queue, app::swapchain, imageIndex);

		glfwPollEvents();

		auto endFrame = std::chrono::system_clock::now();
		deltaTime = std::chrono::duration_cast<std::chrono::duration<double>>(endFrame - startFrame).count();
	}

	vkRenderer::allQueuesWaitIdle();

	vkRenderer::destroySemaphore(app::imageAvailable);

	app::uniformBuffer.~Buffer();
	app::vertexBuffer.~Buffer();
	app::indexBuffer.~Buffer();

	delete[] app::commandBuffers;

	app::pipeline.~Pipeline();

	app::vertShader.~Shader();
	app::fragShader.~Shader();

	app::descriptorPool.~DescriptorPool();

	delete[] app::framebuffers;

	app::renderPass.~RenderPass();

	app::depthImage.~Image();

	app::swapchain.~Swapchain();
	app::surface.~Surface();

	terminateVulkan(app::vertShader, app::fragShader);
	
	glfwDestroyWindow(app::window);
	glfwTerminate();

#ifdef _DEBUG
	system("pause");
#endif
 }