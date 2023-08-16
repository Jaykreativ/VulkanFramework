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

	const uint32_t WIDTH = 1000;
	const uint32_t HEIGHT = 600;
	const char* TITLE = "Vulkan Application";

	std::vector<Vertex> vertexArray = {
		Vertex({0.5f, -0.5f, -0.5f}),
		Vertex({-0.5f, -0.5f, -0.5f}),
		Vertex({-0.5f, 0.5f, -0.5f}),
		Vertex({0.5f, 0.5f, -0.5f}),
		Vertex({0, 0, 0.5f})
	};
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
		glm::mat4(1)
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
	vkRenderer::RenderPass renderPass = vkRenderer::RenderPass(true);
	renderPass.init();

	vkRenderer::Framebuffer* framebuffers = new vkRenderer::Framebuffer[vkRenderer::getSwapchainImages().size()];
	for (int i = 0; i < vkRenderer::getSwapchainImages().size(); i++) {
		framebuffers[i].addAttachment(vkRenderer::getSwapchainImageViews()[i]);
		framebuffers[i].setRenderPass(renderPass);
		framebuffers[i].setWidth(app::WIDTH);
		framebuffers[i].setHeight(app::HEIGHT);
		framebuffers[i].init();
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

		vertShader.init();
		fragShader.init();

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

		//Set renderPass which is compatible with the one the pipeline is used in
		pipeline.setRenderPass(renderPass);

		//Initialize pipeline
		pipeline.init();
	}

	vkRenderer::Buffer vertexBuffer = vkRenderer::Buffer(app::vertexArray.size() * sizeof(Vertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	vertexBuffer.init(); vertexBuffer.allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	vertexBuffer.uploadData(app::vertexArray.size() * sizeof(Vertex), app::vertexArray.data());
	
	vkRenderer::Buffer indexBuffer = vkRenderer::Buffer(app::indexArray.size() * sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	indexBuffer.init(); indexBuffer.allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	indexBuffer.uploadData(app::indexArray.size() * sizeof(uint32_t), app::indexArray.data());

	VkSemaphore imageAvailable;
	vkRenderer::createSemaphore(&imageAvailable);

	uint32_t countCommandBuffers = vkRenderer::getSwapchainImages().size();//Allocate CommandBuffers for recording
	vkRenderer::CommandBuffer* commandBuffers = new vkRenderer::CommandBuffer[countCommandBuffers];
	for (int i = 0; i < vkRenderer::getSwapchainImages().size(); i++) {
		commandBuffers[i].allocate();
		commandBuffers[i].addWaitSemaphore(imageAvailable, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
	}

	//Record CommandBuffers
	for (int i = 0; i < vkRenderer::getSwapchainImages().size(); i++) {
		const VkFramebuffer& framebuffer = framebuffers[i].getVkFramebuffer();
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

		uint64_t offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer.getVkCommandBuffer(), 0, 1, &vertexBuffer.getVkBuffer(), offsets);
		vkCmdBindIndexBuffer(commandBuffer.getVkCommandBuffer(), indexBuffer.getVkBuffer(), 0, VK_INDEX_TYPE_UINT32);

		vkCmdBindDescriptorSets(commandBuffer.getVkCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.getPipelineLayout(), 0, 1, &descriptorPool.getDescriptorSet(0), 0, nullptr);

		vkCmdDrawIndexed(commandBuffer.getVkCommandBuffer(), app::indexArray.size(), 1, 0, 0, 0);

		vkCmdEndRenderPass(commandBuffer.getVkCommandBuffer());

		commandBuffer.end();
	}
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
		app::ubo.view = glm::lookAt(glm::vec3(0, 1, 0.5f), glm::vec3(0, 0, 0), glm::vec3(0, 0, 1));
		app::ubo.perspective = glm::perspective<float>(30, app::WIDTH / static_cast<float>(app::HEIGHT), 0.1f, 100);

		void* data;  uniformBuffer.map(&data);
		memcpy(data, &app::ubo, sizeof(UniformBufferObject));
		uniformBuffer.unmap();
		
		uint32_t imageIndex = 0;
		vkRenderer::acquireNextImage(imageAvailable, VK_NULL_HANDLE, &imageIndex);

		VkQueue queue;
		commandBuffers[imageIndex].submit(&queue);

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
		framebuffers[i].~Framebuffer();
	}

	renderPass.~RenderPass();

	terminateVulkan(vertShader, fragShader);
	terminateGLFW(app::window);
 }