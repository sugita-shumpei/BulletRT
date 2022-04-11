#include <Test0.h>
int main(int argc, const char **argv)
{
    auto app = Test0Application();
    return app.Run(argc, argv);
}

void Test0Application::InitInstance()
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
            .setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral)
            .setPfnUserCallback(DebugUtilsUserCallback))
#endif
        .Build());
}

void Test0Application::InitDevice()
{
    auto deviceBuilders = BulletRT::Core::VulkanDevice::Builder::Enumerate(m_VulkanInstance.get());
    auto& deviceBuilder = deviceBuilders.front();
    m_VulkanDeviceProperties = deviceBuilder.GetPhysicalDevice().getProperties();
    m_VulkanQueueFamilyProperties = deviceBuilder.GetPhysicalDevice().getQueueFamilyProperties();
    m_VulkanMemoryProperties = deviceBuilder.GetPhysicalDevice().getMemoryProperties();

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


    if (!SupportVulkan11Features()) {
        throw std::runtime_error("Failed To Support Vulkan 1.1");
    }
    else if(!SupportVulkan12Features()) {
        deviceBuilder.SetExtension(VK_KHR_SPIRV_1_4_EXTENSION_NAME)
            .SetExtension(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME)
            .SetExtension(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME)
            .SetExtension(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME)
            .ResetFeatures<vk::PhysicalDeviceBufferDeviceAddressFeaturesKHR>()
            .ResetFeatures<vk::PhysicalDeviceDescriptorIndexingFeatures>()
            .ResetFeatures<vk::PhysicalDeviceTimelineSemaphoreFeatures>();
    }
    else if (!SupportVulkan13Features()) {
        deviceBuilder.SetExtension(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME)
            .ResetFeatures<vk::PhysicalDeviceSynchronization2Features>();
    }

    if (SupportVulkan12Features()) {
        deviceBuilder.ResetFeatures<vk::PhysicalDeviceVulkan11Features>()
                     .ResetFeatures<vk::PhysicalDeviceVulkan12Features>();
    }
    if (SupportVulkan13Features()) {
        deviceBuilder.ResetFeatures<vk::PhysicalDeviceVulkan13Features>();
    }
    {
        deviceBuilder.SetExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME)
                     .SetExtension(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME)
                     .SetExtension(VK_KHR_RAY_QUERY_EXTENSION_NAME)
                     .SetExtension(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME)
                     .SetExtension(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME)
                     .SetExtension(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME)
                     .SetExtension("VK_KHR_portability_subset")
                     .ResetFeatures<vk::PhysicalDeviceRayTracingPipelineFeaturesKHR>()
                     .ResetFeatures<vk::PhysicalDeviceRayQueryFeaturesKHR>()
                     .ResetFeatures<vk::PhysicalDeviceAccelerationStructureFeaturesKHR>()
                     .SetQueueFamilies(queueFamilyBuilders);
    }

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

void Test0Application::InitStaging(vk::DeviceSize deviceSize)
{
    m_VulkanStaging = BulletRT::Utils::VulkanStaging::New(m_VulkanDevice.get(), deviceSize);
}

void Test0Application::InitCommandPool()
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

void Test0Application::InitMesh()
{

    auto triVertData = std::vector<float>{ 1.0f,0.0f,0.0f,-1.0f,0.0f,0.0f,0.0f,1.0f,0.0f };
    auto triIndxData = std::vector<uint32_t>{ 0,1,2 };

    auto triVertSize = triVertData.size() * sizeof(triVertData[0]);
    auto triVertOffs = size_t(0);
    auto triIndxSize = triIndxData.size() * sizeof(triIndxData[0]);
    auto triIndxOffs = triVertOffs;
    
    auto bufferUsageExt = vk::BufferUsageFlags{};

    if (IsDiscrateGpu()) {
        bufferUsageExt |= vk::BufferUsageFlagBits::eTransferDst;
    }
    if (m_VulkanDevice->SupportExtension(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME)) {
        bufferUsageExt |= vk::BufferUsageFlagBits::eStorageBuffer;
        bufferUsageExt |= vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR;
        bufferUsageExt |= vk::BufferUsageFlagBits::eShaderDeviceAddress;
    }

    m_VulkanVertMeshBuffer = BulletRT::Core::VulkanBuffer::Builder()
        .SetUsage(vk::BufferUsageFlagBits::eVertexBuffer | bufferUsageExt)
        .SetSize(triVertSize)
        .Build(m_VulkanDevice.get());

    m_VulkanIndxMeshBuffer = BulletRT::Core::VulkanBuffer::Builder()
        .SetUsage(vk::BufferUsageFlagBits::eIndexBuffer | bufferUsageExt)
        .SetSize(triIndxSize)
        .Build(m_VulkanDevice.get());

    auto vMemRequirements = m_VulkanVertMeshBuffer->QueryMemoryRequirements();
    auto iMemRequirements = m_VulkanIndxMeshBuffer->QueryMemoryRequirements();

    auto memPropRequired  = IsDiscrateGpu() ? vk::MemoryPropertyFlagBits::eDeviceLocal : vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
    auto memPropAvoided   = IsDiscrateGpu() ? vk::MemoryPropertyFlagBits::eHostVisible : vk::MemoryPropertyFlags{};

    auto vMemTypeIndices = FindMemoryTypeIndices(m_VulkanMemoryProperties, vMemRequirements.memoryTypeBits, memPropRequired, memPropAvoided);
    auto iMemTypeIndices = FindMemoryTypeIndices(m_VulkanMemoryProperties, iMemRequirements.memoryTypeBits, memPropRequired, memPropAvoided);

    if ( vMemTypeIndices.empty()||iMemTypeIndices.empty()) {
        throw std::runtime_error("Failed To Find Memory Type!");
    }

    auto vMemTypeIndex = vMemTypeIndices.front();
    auto iMemTypeIndex = iMemTypeIndices.front();

    if (vMemTypeIndex == iMemTypeIndex) {
        auto vertBufferOffset = 0;
        auto vertBufferSize   = vMemRequirements.size;
        auto indxBufferOffset = CalcAlignedSize(vertBufferSize, iMemRequirements.alignment);
        auto indxBufferSize   = iMemRequirements.size;
        auto allocationSize   = indxBufferOffset + indxBufferSize;

        m_VulkanMeshMemories.reserve(1);
        m_VulkanMeshMemories.push_back(BulletRT::Core::VulkanDeviceMemory::Builder()
            .SetMemoryTypeIndex(vMemTypeIndex)
            .SetAllocationSize(allocationSize)
            .SetMemoryAllocateFlagsInfo(vk::MemoryAllocateFlagsInfo().setFlags(vk::MemoryAllocateFlagBits::eDeviceAddress))
            .Build(m_VulkanDevice.get())
        );
        m_VulkanVertMemoryBuffer = BulletRT::Core::VulkanMemoryBuffer::Bind(
            m_VulkanVertMeshBuffer.get(), m_VulkanMeshMemories.front().get(), vertBufferOffset
        );
        m_VulkanIndxMemoryBuffer = BulletRT::Core::VulkanMemoryBuffer::Bind(
            m_VulkanIndxMeshBuffer.get(), m_VulkanMeshMemories.front().get(), indxBufferOffset
        );
    }
    else {
        m_VulkanMeshMemories.reserve(2);
        m_VulkanMeshMemories.push_back(BulletRT::Core::VulkanDeviceMemory::Builder()
            .SetMemoryTypeIndex(vMemTypeIndex)
            .SetAllocationSize(vMemRequirements.size)
            .SetMemoryAllocateFlagsInfo(vk::MemoryAllocateFlagsInfo().setFlags(vk::MemoryAllocateFlagBits::eDeviceAddress))
            .Build(m_VulkanDevice.get())
        );
        m_VulkanMeshMemories.push_back(BulletRT::Core::VulkanDeviceMemory::Builder()
            .SetMemoryTypeIndex(iMemTypeIndex)
            .SetAllocationSize(iMemRequirements.size)
            .SetMemoryAllocateFlagsInfo(vk::MemoryAllocateFlagsInfo().setFlags(vk::MemoryAllocateFlagBits::eDeviceAddress))
            .Build(m_VulkanDevice.get())
        );
        m_VulkanVertMemoryBuffer = BulletRT::Core::VulkanMemoryBuffer::Bind(
            m_VulkanVertMeshBuffer.get(), m_VulkanMeshMemories[0].get(), 0
        );
        m_VulkanIndxMemoryBuffer = BulletRT::Core::VulkanMemoryBuffer::Bind(
            m_VulkanIndxMeshBuffer.get(), m_VulkanMeshMemories[1].get(), 0
        );
    }

    if (!IsDiscrateGpu()) {
        if (vMemTypeIndex == iMemTypeIndex) {
            void* pMapedData;
            if (m_VulkanMeshMemories.front()->Map(&pMapedData) == vk::Result::eSuccess) {
                std::memcpy((char*)pMapedData + m_VulkanVertMemoryBuffer->GetMemoryOffset(), triVertData.data(), triVertSize);
                std::memcpy((char*)pMapedData + m_VulkanIndxMemoryBuffer->GetMemoryOffset(), triIndxData.data(), triIndxSize);
                m_VulkanMeshMemories.front()->Unmap();
            }
        }
        else {
            void* pMapedData;
            if (m_VulkanVertMemoryBuffer->Map(&pMapedData) == vk::Result::eSuccess) {
                std::memcpy((char*)pMapedData, triVertData.data(), triVertSize);
                m_VulkanVertMemoryBuffer->Unmap();
            }
            if (m_VulkanIndxMemoryBuffer->Map(&pMapedData) == vk::Result::eSuccess) {
                std::memcpy((char*)pMapedData, triIndxData.data(), triIndxSize);
                m_VulkanIndxMemoryBuffer->Unmap();
            }
        }
        return;
    }

    m_VulkanStaging->Upload({
          {triVertData.data(),triVertSize,triVertOffs},
          {triIndxData.data(),triIndxSize,triIndxOffs}
    });

    auto copyCommand = m_VulkanGCommandPool->NewCommandBuffer(vk::CommandBufferLevel::ePrimary);
    copyCommand->GetCommandBufferVk().begin(vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
    copyCommand->GetCommandBufferVk().copyBuffer(m_VulkanStaging->GetBufferVk(), m_VulkanVertMeshBuffer->GetBufferVk(), vk::BufferCopy().setSize(triVertSize).setSrcOffset(triVertOffs));
    copyCommand->GetCommandBufferVk().copyBuffer(m_VulkanStaging->GetBufferVk(), m_VulkanIndxMeshBuffer->GetBufferVk(), vk::BufferCopy().setSize(triIndxSize).setSrcOffset(triIndxOffs));
    copyCommand->GetCommandBufferVk().end();

    auto copyCommandVk = copyCommand->GetCommandBufferVk();

    auto gQueue = m_VulkanGQueueFamily->GetQueues().front();
    auto gFence = m_VulkanDevice->NewFence();
    gQueue.GetQueueVk().submit(vk::SubmitInfo().setCommandBuffers(copyCommandVk), gFence->GetFenceVk());

    if (gFence->QueryStatus() != vk::Result::eSuccess)
    {
        gFence->Wait(UINT64_MAX);
    }
    gFence.reset();
    copyCommand.reset();
}
