#pragma once
#include "VulkanUtils.h"

class Shader {
private:
    void createShaderModule(VkDevice &device, std::vector<char> &code, VkShaderModule &module) {
        VkShaderModuleCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.codeSize = code.size();
        createInfo.pCode = (uint32_t*)code.data();

        VkResult result = vkCreateShaderModule(device, &createInfo, nullptr, &module);
        VK_ASSERT(result);
    }

    VkDevice device;

    const std::string m_path;
    VkShaderModule m_module;

public:
    Shader(const std::string path)
        : m_path(path), device(device)
    {}

    void init(VkDevice& device) {
        auto code = vkUtils::readFile(m_path);
        createShaderModule(device, code, m_module);
    }

    void destroy(VkDevice &device) {
        vkDestroyShaderModule(device, m_module, nullptr);
    }

    VkShaderModule getModule() {
        return m_module;
    }

    const std::string getPath() {
        return m_path;
    }
};
