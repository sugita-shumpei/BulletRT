#ifndef TEST0_TEST0_H
#define TEST0_TEST0_H
#include <BulletRT/Core/BulletRTCore.h>
#include <BulletRT/Utils/VulkanStaging.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <string_view>
#include <ranges>
#include <algorithm>
auto CalcAlignedSize(vk::DeviceSize size, vk::DeviceSize alignment) -> vk::DeviceSize
{
    return ((size + alignment - 1) / alignment) * alignment;
}
class Test0Application
{
public:
    auto Run(int argc, const char **argv) -> int
    {
        m_Argc = argc;
        m_Argv = argv;
        Initialize();
        Terminate();
        return 0;
    }
    ~Test0Application() noexcept
    {
    }
private:
    void Initialize()
    {
        BulletRT::Core::VulkanContext::Initialize();
        InitInstance();
        InitDevice();
        InitCommandPool();
        if (IsDiscrateGpu())
        {
            InitStaging(128 * 1024 * 1024);
        }
        InitMesh();
        InitRenderPass();
    }
    void Terminate()
    {
        FreeRenderPass();
        FreeMesh();
        FreeStaging();
        FreeCommandPool();
        FreeDevice();
        FreeInstance();
        BulletRT::Core::VulkanContext::Terminate();
    }
    void InitInstance();
    void FreeInstance()
    {
        m_VulkanInstance.reset();
    }
    void InitDevice();
    void FreeDevice()
    {
        m_VulkanDevice.reset();
    }
    void InitStaging(vk::DeviceSize deviceSize);
    void FreeStaging()
    {
        m_VulkanStaging.reset();
    }
    void InitCommandPool();
    void FreeCommandPool()
    {
        {
            m_VulkanGCommandPool.reset();
        }
        if (m_VulkanCQueueFamily)
        {
            m_VulkanCCommandPool.reset();
        }
        if (m_VulkanTQueueFamily)
        {
            m_VulkanTCommandPool.reset();
        }
    }
    void InitMesh();
    void FreeMesh(){
        m_VulkanVertMeshBuffer.reset();
        m_VulkanIndxMeshBuffer.reset();
        m_VulkanMeshMemories.clear();
        m_VulkanVertMemoryBuffer.reset();
        m_VulkanIndxMemoryBuffer.reset();
    }
    void InitRenderPass();
    void FreeRenderPass(){
        m_VulkanRenderPass.reset();
    }
private:
    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugUtilsUserCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageTypes,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData)
    {
        std::cout << "[" << vk::to_string(static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(messageSeverity)) << "]";
        std::cout << "[" << vk::to_string(static_cast<vk::DebugUtilsMessageTypeFlagsEXT>(messageTypes)) << "]: ";
        std::cout << pCallbackData->pMessage << std::endl;
        return VK_FALSE;
    }

    static auto FindMemoryTypeIndices(const vk::PhysicalDeviceMemoryProperties &memoryProps,
                                      uint32_t memoryTypeBits,
                                      vk::MemoryPropertyFlags requiredFlags,
                                      vk::MemoryPropertyFlags avoidFlags = vk::MemoryPropertyFlags{}) -> std::vector<uint32_t>
    {
        auto indices = std::vector<uint32_t>();
        for (uint32_t i = 0; i < memoryProps.memoryTypeCount; ++i)
        {
            if ((static_cast<uint32_t>(1) << i) & memoryTypeBits)
            {
                if (((memoryProps.memoryTypes[i].propertyFlags & requiredFlags) == requiredFlags) &&
                    ((memoryProps.memoryTypes[i].propertyFlags & ~avoidFlags) == memoryProps.memoryTypes[i].propertyFlags))
                {
                    indices.push_back(i);
                }
            }
        }
        return indices;
    };

    static auto FindQueueFamilyIndices(const std::vector<vk::QueueFamilyProperties> &queueFamilyProperties,
                                       vk::QueueFlags requiredFlags,
                                       vk::QueueFlags avoidFlags = {}) noexcept -> std::vector<uint32_t>
    {
        auto queueFamilyIndices = std::vector<uint32_t>();
        auto queueFamilyIndex = uint32_t(0);
        for (auto &queueFamilyProp : queueFamilyProperties)
        {
            if ((queueFamilyProp.queueFlags & requiredFlags) &&
                (queueFamilyProp.queueFlags & ~avoidFlags) == queueFamilyProp.queueFlags)
            {
                queueFamilyIndices.push_back(queueFamilyIndex);
            }
            ++queueFamilyIndex;
        }
        return queueFamilyIndices;
    }

    auto GetStagingMemorySize()const noexcept -> vk::DeviceSize { return m_VulkanStaging? m_VulkanStaging->GetSize():0; }
    bool IsDiscrateGpu() const noexcept { return m_VulkanDeviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu; }
    bool SupportVulkan11Features()const noexcept { return m_VulkanDeviceProperties.apiVersion >= VK_API_VERSION_1_1; }
    bool SupportVulkan12Features()const noexcept { return m_VulkanDeviceProperties.apiVersion >= VK_API_VERSION_1_2; }
    bool SupportVulkan13Features()const noexcept { return m_VulkanDeviceProperties.apiVersion >= VK_API_VERSION_1_3; }
private:
    int m_Argc = 0;
    const char **m_Argv = nullptr;
    std::unique_ptr<BulletRT::Core::VulkanInstance> m_VulkanInstance = nullptr;
    std::unique_ptr<BulletRT::Core::VulkanDevice> m_VulkanDevice = nullptr;

    vk::PhysicalDeviceProperties           m_VulkanDeviceProperties = {};
    std::vector<vk::QueueFamilyProperties> m_VulkanQueueFamilyProperties = {};
    vk::PhysicalDeviceMemoryProperties     m_VulkanMemoryProperties = {};

    std::optional<BulletRT::Core::VulkanQueueFamily>   m_VulkanGQueueFamily = std::nullopt;
    std::unique_ptr<BulletRT::Core::VulkanCommandPool> m_VulkanGCommandPool = nullptr;
    std::optional<BulletRT::Core::VulkanQueueFamily>   m_VulkanCQueueFamily = std::nullopt;
    std::unique_ptr<BulletRT::Core::VulkanCommandPool> m_VulkanCCommandPool = nullptr;
    std::optional<BulletRT::Core::VulkanQueueFamily>   m_VulkanTQueueFamily = std::nullopt;
    std::unique_ptr<BulletRT::Core::VulkanCommandPool> m_VulkanTCommandPool = nullptr;
    
    std::unique_ptr<BulletRT::Utils::VulkanStaging> m_VulkanStaging             = nullptr;
    size_t                                          m_VulkanStagingMemoryOffset = 0;

    std::vector<std::unique_ptr<BulletRT::Core::VulkanDeviceMemory>> m_VulkanMeshMemories     = {};
    std::unique_ptr<BulletRT::Core::VulkanBuffer>                    m_VulkanVertMeshBuffer   = nullptr;
    std::unique_ptr<BulletRT::Core::VulkanMemoryBuffer>              m_VulkanVertMemoryBuffer = nullptr;
    std::unique_ptr<BulletRT::Core::VulkanBuffer>                    m_VulkanIndxMeshBuffer   = nullptr;
    std::unique_ptr<BulletRT::Core::VulkanMemoryBuffer>              m_VulkanIndxMemoryBuffer = nullptr;
    std::unique_ptr<BulletRT::Core::VulkanRenderPass>                m_VulkanRenderPass       = nullptr;
    
    GLFWwindow* m_Window = nullptr;
    int m_FbWidth  = 0;
    int m_FbHeight = 0;
};
#endif
