#ifndef TEST0_TEST0_H
#define TEST0_TEST0_H
#include <BulletRT/Core/BulletRTCore.h>
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
    }
    void Terminate()
    {
        FreeStaging();
        FreeCommandPool();
        FreeDevice();
        FreeInstance();
        BulletRT::Core::VulkanContext::Terminate();
    }
    void InitInstance()
    {
        m_VulkanInstance = std::unique_ptr<BulletRT::Core::VulkanInstance>(BulletRT::Core::VulkanInstance::Builder()
                                                                               .SetApiVersion(VK_API_VERSION_1_3)
                                                                               .SetApplicationName("Test0")
                                                                               .SetApplicationVersion(VK_MAKE_API_VERSION(0, 1, 0, 0))
                                                                               .SetEngineName("NO ENGINE")
                                                                               .SetEngineVersion(VK_MAKE_API_VERSION(0, 1, 0, 0))
                                                                               .SetExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)
                                                                               .SetExtension(VK_KHR_SURFACE_EXTENSION_NAME)
#ifdef WIN32
                                                                               .SetExtension("VK_KHR_win32_surface")
#endif
                                                                               .SetLayer("VK_LAYER_KHRONOS_validation")
#ifndef NDEBUG
                                                                               .AddDebugUtilsMessenger(vk::DebugUtilsMessengerCreateInfoEXT()
                                                                                                           .setMessageSeverity(
                                                                                                               vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
                                                                                                               vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
                                                                                                               vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
                                                                                                           .setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral|
                                                                                                                           vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                                                                                                                           vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral)
                                                                                                           .setPfnUserCallback(DebugUtilsUserCallback))
#endif
                                                                               .Build());
    }
    void FreeInstance()
    {
        m_VulkanInstance.reset();
    }
    void InitDevice()
    {
        auto deviceBuilders = BulletRT::Core::VulkanDevice::Builder::Enumerate(m_VulkanInstance.get());
        m_VulkanQueueFamilyProperties = deviceBuilders.front().GetPhysicalDevice().getQueueFamilyProperties();
        m_VulkanMemoryProperties = deviceBuilders.front().GetPhysicalDevice().getMemoryProperties();

        auto gQueFamIndices = FindQueueFamilyIndices(m_VulkanQueueFamilyProperties, vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute | vk::QueueFlagBits::eTransfer);
        auto cQueFamIndices = FindQueueFamilyIndices(m_VulkanQueueFamilyProperties, vk::QueueFlagBits::eCompute | vk::QueueFlagBits::eTransfer, vk::QueueFlagBits::eGraphics);
        auto tQueFamIndices = FindQueueFamilyIndices(m_VulkanQueueFamilyProperties, vk::QueueFlagBits::eTransfer, vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute);

        auto queueFamilyBuilders = std::vector<BulletRT::Core::VulkanQueueFamily::Builder>();
        if (gQueFamIndices.empty())
        {
            throw std::runtime_error("Failed To Find Graphics Queue!");
        }
        {
            queueFamilyBuilders.push_back(BulletRT::Core::VulkanQueueFamily::Builder().SetQueueFamilyIndex(gQueFamIndices.front()).SetQueueCount(1));
        }
        if (!tQueFamIndices.empty())
        {
            queueFamilyBuilders.push_back(BulletRT::Core::VulkanQueueFamily::Builder().SetQueueFamilyIndex(tQueFamIndices.front()).SetQueueCount(1));
        }
        if (!cQueFamIndices.empty())
        {
            queueFamilyBuilders.push_back(BulletRT::Core::VulkanQueueFamily::Builder().SetQueueFamilyIndex(cQueFamIndices.front()).SetQueueCount(1));
        }
        
        auto& deviceBuilder = deviceBuilders.front();
        auto apiVersion = deviceBuilder.GetPhysicalDevice().getProperties().apiVersion;
#if defined(__APPLE__)
        deviceBuilder.SetExtension("VK_KHR_portability_subset");
#endif
        if (apiVersion < VK_API_VERSION_1_1){
            throw std::runtime_error("Failed To Support Vulkan 1.1");
        }
        if (VK_API_VERSION_MINOR(apiVersion)==1){
            deviceBuilder.SetExtension(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
            deviceBuilder.SetExtension(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
            deviceBuilder.SetExtension(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
            deviceBuilder.ResetFeatures<vk::PhysicalDeviceBufferDeviceAddressFeaturesKHR>();
            deviceBuilder.ResetFeatures<vk::PhysicalDeviceDescriptorIndexingFeatures>();
            deviceBuilder.ResetFeatures<vk::PhysicalDeviceTimelineSemaphoreFeatures>();
        }
        if (apiVersion >= VK_API_VERSION_1_2){
            deviceBuilder.ResetFeatures<vk::PhysicalDeviceVulkan11Features>()
                         .ResetFeatures<vk::PhysicalDeviceVulkan12Features>();
        }
        if (apiVersion >= VK_API_VERSION_1_3){
            deviceBuilder.ResetFeatures<vk::PhysicalDeviceVulkan13Features>();
        }
        deviceBuilder.SetExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME)
                     .SetExtension(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME)
                     .SetExtension(VK_KHR_RAY_QUERY_EXTENSION_NAME)
                     .SetExtension(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME)
                     .SetExtension(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME)
                     .SetExtension(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME)
                     .ResetFeatures<vk::PhysicalDeviceRayTracingPipelineFeaturesKHR>()
                     .ResetFeatures<vk::PhysicalDeviceRayQueryFeaturesKHR>()
                     .ResetFeatures<vk::PhysicalDeviceAccelerationStructureFeaturesKHR>()
                     .SetQueueFamilies(queueFamilyBuilders);

        m_VulkanDevice = std::unique_ptr<BulletRT::Core::VulkanDevice>(deviceBuilders.front().Build());

        if (!m_VulkanDevice->SupportExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME))
        {
            throw std::runtime_error("Failed To Support Swapchain!");
        }

        if (!gQueFamIndices.empty())
        {
            m_VulkanGQueueFamily = m_VulkanDevice->AcquireQueueFamily(gQueFamIndices.front());
        }
        if (!cQueFamIndices.empty())
        {
            m_VulkanCQueueFamily = m_VulkanDevice->AcquireQueueFamily(cQueFamIndices.front());
        }
        if (!tQueFamIndices.empty())
        {
            m_VulkanTQueueFamily = m_VulkanDevice->AcquireQueueFamily(tQueFamIndices.front());
        }
    }
    void FreeDevice()
    {
        m_VulkanDevice.reset();
    }
    void InitStaging(vk::DeviceSize deviceSize)
    {
        m_VulkanStagingBuffer = std::unique_ptr<BulletRT::Core::VulkanBuffer>(
            BulletRT::Core::VulkanBuffer::Builder()
                .SetUsage(vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eShaderDeviceAddress)
                .SetSize(deviceSize)
                .SetQueueFamilyIndices({})
                .Build(m_VulkanDevice.get()));
        auto sMemRequirements = m_VulkanStagingBuffer->QueryMemoryRequirements();
        auto sMemTypeIndex = uint32_t(0);
        {
            auto sMemTypeIndices = FindMemoryTypeIndices(m_VulkanMemoryProperties, sMemRequirements.memoryTypeBits,
                                                         vk::MemoryPropertyFlagBits::eHostVisible |
                                                             vk::MemoryPropertyFlagBits::eHostCoherent |
                                                             vk::MemoryPropertyFlagBits::eHostCached);
            if (sMemTypeIndices.empty())
            {
                throw std::runtime_error("Failed To Find Memory Type!");
            }
            sMemTypeIndex = sMemTypeIndices.front();
        }
        m_VulkanStagingMemory = BulletRT::Core::VulkanDeviceMemory::Builder()
                                    .SetAllocationSize(sMemRequirements.size)
                                    .SetMemoryTypeIndex(sMemTypeIndex)
                                    .SetMemoryAllocateFlagsInfo(
                                        vk::MemoryAllocateFlagsInfo().setFlags(vk::MemoryAllocateFlagBits::eDeviceAddress))
                                    .Build(m_VulkanDevice.get());
        m_VulkanStagingMemoryBuffer = BulletRT::Core::VulkanMemoryBuffer::Bind(m_VulkanStagingBuffer.get(), m_VulkanStagingMemory.get());
    }
    void FreeStaging()
    {
        m_VulkanStagingBuffer.reset();
        m_VulkanStagingMemory.reset();
        m_VulkanStagingMemoryBuffer.reset();
    }
    void InitCommandPool()
    {
        {
            m_VulkanGCommandPool = m_VulkanGQueueFamily->NewCommandPool();
        }
        if (m_VulkanCQueueFamily)
        {
            m_VulkanCCommandPool = m_VulkanCQueueFamily->NewCommandPool();
        }
        if (m_VulkanTQueueFamily)
        {
            m_VulkanTCommandPool = m_VulkanTQueueFamily->NewCommandPool();
        }
    }
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
    bool IsDiscrateGpu() const noexcept { return m_VulkanDeviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu; }

private:
    int m_Argc = 0;
    const char **m_Argv = nullptr;
    std::unique_ptr<BulletRT::Core::VulkanInstance> m_VulkanInstance = nullptr;
    std::unique_ptr<BulletRT::Core::VulkanDevice> m_VulkanDevice = nullptr;

    vk::PhysicalDeviceProperties m_VulkanDeviceProperties = {};
    std::vector<vk::QueueFamilyProperties> m_VulkanQueueFamilyProperties = {};
    vk::PhysicalDeviceMemoryProperties m_VulkanMemoryProperties = {};

    std::optional<BulletRT::Core::VulkanQueueFamily> m_VulkanGQueueFamily = std::nullopt;
    std::unique_ptr<BulletRT::Core::VulkanCommandPool> m_VulkanGCommandPool = nullptr;
    std::optional<BulletRT::Core::VulkanQueueFamily> m_VulkanCQueueFamily = std::nullopt;
    std::unique_ptr<BulletRT::Core::VulkanCommandPool> m_VulkanCCommandPool = nullptr;
    std::optional<BulletRT::Core::VulkanQueueFamily> m_VulkanTQueueFamily = std::nullopt;
    std::unique_ptr<BulletRT::Core::VulkanCommandPool> m_VulkanTCommandPool = nullptr;

    std::unique_ptr<BulletRT::Core::VulkanDeviceMemory> m_VulkanStagingMemory = nullptr;
    std::unique_ptr<BulletRT::Core::VulkanBuffer> m_VulkanStagingBuffer = nullptr;
    std::unique_ptr<BulletRT::Core::VulkanMemoryBuffer> m_VulkanStagingMemoryBuffer = nullptr;

    std::vector<std::unique_ptr<BulletRT::Core::VulkanDeviceMemory>> m_VulkanMeshMemories = {};
    std::unique_ptr<BulletRT::Core::VulkanBuffer> m_VulkanVertMeshBuffer = nullptr;
    std::unique_ptr<BulletRT::Core::VulkanMemoryBuffer> m_VulkanVertMemoryBuffer = nullptr;
    std::unique_ptr<BulletRT::Core::VulkanBuffer> m_VulkanIndxMeshBuffer = nullptr;
    std::unique_ptr<BulletRT::Core::VulkanMemoryBuffer> m_VulkanIndxMemoryBuffer = nullptr;
};
#endif
