#define STRINGIFY(x) #x
#define EXPAND(x) STRINGIFY(x)

#ifndef _DEBUG
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif

#include "Application.h"
#include "gtc/matrix_transform.hpp"

#include <chrono>

namespace app {
	GLFWwindow* window;

	const uint32_t WIDTH = 1920;
	const uint32_t HEIGHT = 1061;
	const char* TITLE = "Vulkan Application";

	std::vector<Vertex> vertexArray = {
		Vertex({ 0, -0.5f, 0 }),
		Vertex({ -0.5f, 0.5f, 0 }),
		Vertex({ 0.5f, 0.5f, 0 })
	};
	std::vector<uint32_t> indexArray = {
		0, 1, 2
	};

	UniformBufferObject ubo = {
		{1, 1, 1, 1} //Color
	};
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

	vkRenderer::Shader vertShader = vkRenderer::Shader(projDir + "Shader\\shader.vert.spv");
	vkRenderer::Shader fragShader = vkRenderer::Shader(projDir + "Shader\\shader.frag.spv");

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

	initGLFW(app::window, app::WIDTH, app::HEIGHT, app::TITLE);
	initVulkan(app::window, app::WIDTH, app::HEIGHT, app::TITLE, app::ubo, vertShader, fragShader, app::vertexArray, app::indexArray);
	
#if Test
	/*Initialization*/
	vkRenderer::RenderPass renderPass = vkRenderer::RenderPass();
	renderPass.init();

	VkFramebuffer* framebuffers = new VkFramebuffer[vkRenderer::getSwapchainImages().size()];
	for (int i = 0; i < vkRenderer::getSwapchainImages().size(); i++) {
		std::vector<VkImageView> attachments = {
			vkRenderer::getSwapchainImageViews()[i]
		};
		vkRenderer::createFramebuffer(renderPass.getVkRenderPassRef(), attachments, app::WIDTH, app::HEIGHT, framebuffers[i]);
	}

	vkRenderer::DescriptorPool descriptorPool = vkRenderer::DescriptorPool();

	vkRenderer::Buffer uniformBuffer = vkRenderer::Buffer(sizeof(UniformBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	uniformBuffer.init(); uniformBuffer.allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	void* data; uniformBuffer.map(&data);
	memcpy(data, &app::ubo, sizeof(UniformBufferObject));
	uniformBuffer.unmap();

	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = uniformBuffer.getVkBuffer();
	bufferInfo.offset = 0;
	bufferInfo.range = uniformBuffer.getSize();

	vkRenderer::Descriptor descriptor{};
	descriptor.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptor.stages = VK_SHADER_STAGE_VERTEX_BIT;
	descriptor.binding = 0;
	descriptor.pBufferInfo = &bufferInfo;

	descriptorPool.addDescriptorSet();
	descriptorPool.addDescriptor(descriptor, 0);

	descriptorPool.init();

	vkRenderer::Pipeline pipeline = vkRenderer::Pipeline();
	{//Add shaders
		
		vertShader.setStage(VK_SHADER_STAGE_VERTEX_BIT);
		fragShader.setStage(VK_SHADER_STAGE_FRAGMENT_BIT);

		pipeline.addShader(vertShader.getShaderStage());
		pipeline.addShader(fragShader.getShaderStage());

		//Add viewport and scissor
		VkViewport viewport;
		viewport.x = 0; viewport.y = 0;
		viewport.width = app::WIDTH; viewport.height = app::HEIGHT;
		viewport.minDepth = 0; viewport.maxDepth = 1;
		pipeline.addViewport(viewport);
		VkRect2D scissor;
		scissor.offset = { 0, 0 };
		scissor.extent = { app::WIDTH, app::HEIGHT };
		pipeline.addScissor(scissor);

		//Add Vertex input description
		for (int i = 0; i < Vertex::getInputAttributeDescriptions().size(); i++) {
			VkVertexInputAttributeDescription attributeDescription = Vertex::getInputAttributeDescriptions()[i];
			pipeline.addVertexInputAttrubuteDescription(attributeDescription);
		}
		pipeline.addVertexInputBindingDescription(Vertex::getInputBindingDescription());

		//Add the descriptorSetLayout for the pipelineLayout
		pipeline.addDescriptorSetLayout(descriptorPool.getDescriptorSetLayout(0));

		//Initialize pipeline
		pipeline.init();
	}

	vkRenderer::Buffer vertexBuffer = vkRenderer::Buffer(app::vertexArray.size() * sizeof(Vertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	vertexBuffer.init(); vertexBuffer.allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	vertexBuffer.uploadData(app::vertexArray.size() * sizeof(Vertex), app::vertexArray.data());
	
	vkRenderer::Buffer indexBuffer = vkRenderer::Buffer(app::indexArray.size() * sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	indexBuffer.init(); indexBuffer.allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	indexBuffer.uploadData(app::indexArray.size() * sizeof(uint32_t), app::indexArray.data());

	uint32_t countCommandBuffers = vkRenderer::getSwapchainImages().size();//Allocate CommandBuffers for recording
	vkRenderer::CommandBuffer* commandBuffers = new vkRenderer::CommandBuffer[countCommandBuffers];
	for (int i = 0; i < vkRenderer::getSwapchainImages().size(); i++) {
		commandBuffers[i].allocate();
	}

	//Record CommandBuffers
	for (int i = 0; i < vkRenderer::getSwapchainImages().size(); i++) {
		VkFramebuffer& framebuffer = framebuffers[i];
		vkRenderer::CommandBuffer& commandBuffer = commandBuffers[i];
		commandBuffer.begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

		VkRenderPassBeginInfo renderPassBeginInfo;
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext = nullptr;
		renderPassBeginInfo.renderPass = renderPass.getVkRenderPass();
		renderPassBeginInfo.framebuffer = framebuffer;
		renderPassBeginInfo.renderArea.offset = { 0,0};
		renderPassBeginInfo.renderArea.extent = { app::WIDTH, app::HEIGHT };
		VkClearValue colorClearValue = { 0.1f, 0.1f, 0.1f, 1.0f };
		VkClearValue depthStencilClearValue = { 1.0f, 0 };
		std::vector<VkClearValue> clearValues = {
			colorClearValue,
			depthStencilClearValue
		};
		renderPassBeginInfo.clearValueCount = clearValues.size();
		renderPassBeginInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer.getVkCommandBuffer(), &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(commandBuffer.getVkCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.getVkPipeline());

		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer.getVkCommandBuffer(), 0, 1, &vertexBuffer.getVkBuffer(), offsets);
		vkCmdBindIndexBuffer(commandBuffer.getVkCommandBuffer(), indexBuffer.getVkBuffer(), 0, VK_INDEX_TYPE_UINT32);

		vkCmdBindDescriptorSets(commandBuffer.getVkCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.getPipelineLayout(), 0, 1, &descriptorPool.getDescriptorSet(0), 0, nullptr);

		vkCmdDrawIndexed(commandBuffer.getVkCommandBuffer(), app::indexArray.size(), 1, 0, 0, 0);

		vkCmdEndRenderPass(commandBuffer.getVkCommandBuffer());

		commandBuffer.end();
	}

	VkSemaphore imageAvailable;
	vkRenderer::createSemaphore(&imageAvailable);
#endif

	//printStats();
	
	float a = 0;

	double deltaTime = 0;
	while (!glfwWindowShouldClose(app::window)) {
		auto startFrame = std::chrono::system_clock::now();
		if (a > 1) a = 0;
		a += 0.5*deltaTime;

		app::ubo.color = glm::vec4(a, 1, 0.5, 1);
		app::ubo.transform = glm::rotate(glm::mat4(1.0f), a * glm::radians(360.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		void* data;  uniformBuffer.map(&data);
		memcpy(data, &app::ubo, sizeof(UniformBufferObject));
		uniformBuffer.unmap();
		
		uint32_t imageIndex = 0;
		vkRenderer::acquireNextImage(imageAvailable, VK_NULL_HANDLE, &imageIndex);

		VkSubmitInfo submitInfo;
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &imageAvailable;
		VkPipelineStageFlags waitDstStageMask[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.pWaitDstStageMask = waitDstStageMask;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffers[imageIndex].getVkCommandBuffer();
		submitInfo.signalSemaphoreCount = 0;
		submitInfo.pSignalSemaphores = nullptr;

		VkQueue queue = vkUtils::queueHandler::getQueue();
		VkResult result = vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
		VK_ASSERT(result);

		vkRenderer::queuePresent(queue, imageIndex);

		glfwPollEvents();

		auto endFrame = std::chrono::system_clock::now();
		deltaTime = std::chrono::duration_cast<std::chrono::duration<double>>(endFrame - startFrame).count();
	}

	vkRenderer::allQueuesWaitIdle();

	vkRenderer::destroySemaphore(imageAvailable);

	uniformBuffer.~Buffer();
	vertexBuffer.~Buffer();
	indexBuffer.~Buffer();

	delete[] commandBuffers;

	pipeline.~Pipeline();

	vertShader.~Shader();
	fragShader.~Shader();

	descriptorPool.~DescriptorPool();

	for (int i = 0; i < vkRenderer::getSwapchainImages().size(); i++) {
		vkRenderer::destroyFramebuffer(framebuffers[i]);
	}

	renderPass.~RenderPass();

	terminateVulkan(vertShader, fragShader);
	terminateGLFW(app::window);
 }