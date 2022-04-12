#include <BulletRT/Core/BulletRTCore.h>
#include <iostream>
#include <vector>
using namespace BulletRT::Core;
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE
BulletRT::Core::VulkanDeviceFeaturesSet::VulkanDeviceFeaturesSet(const VulkanDeviceFeaturesSet &featureSet) noexcept
{
    if (!featureSet.m_Holders.empty())
    {
        m_IndexMap = featureSet.m_IndexMap;
        m_Holders.reserve(featureSet.m_Holders.size());
        m_TailPNext = nullptr;
        for (auto &holder : featureSet.m_Holders)
        {
            m_Holders.push_back(std::unique_ptr<IVulkanDeviceExtFeatureHolder>(holder->Clone()));
        }
        if (m_Holders.size() > 1)
        {
            for (size_t i = 0; i < m_Holders.size() - 1; ++i)
            {
                m_Holders[i]->Link(m_Holders[i + 1]->GetPointer());
            }
        }
    }
}

BulletRT::Core::VulkanDeviceFeaturesSet::VulkanDeviceFeaturesSet(VulkanDeviceFeaturesSet &&featureSet) noexcept
    : m_Holders{std::move(featureSet.m_Holders)}, m_IndexMap{std::move(featureSet.m_IndexMap)}, m_TailPNext{featureSet.m_TailPNext}
{
    featureSet.m_TailPNext = nullptr;
}

BulletRT::Core::VulkanDeviceFeaturesSet &BulletRT::Core::VulkanDeviceFeaturesSet::operator=(const VulkanDeviceFeaturesSet &featureSet) noexcept
{
    // TODO: return �X�e�[�g�����g�������ɑ}�����܂�
    if (this != &featureSet)
    {

        if (!featureSet.m_Holders.empty())
        {
            m_IndexMap = featureSet.m_IndexMap;
            m_Holders.clear();
            m_Holders.reserve(featureSet.m_Holders.size());
            m_TailPNext = nullptr;
            for (auto &holder : featureSet.m_Holders)
            {
                m_Holders.push_back(std::unique_ptr<IVulkanDeviceExtFeatureHolder>(holder->Clone()));
            }
            if (m_Holders.size() > 1)
            {
                for (size_t i = 0; i < m_Holders.size() - 1; ++i)
                {
                    m_Holders[i]->Link(m_Holders[i + 1]->GetPointer());
                }
            }
        }
    }
    return *this;
}

BulletRT::Core::VulkanDeviceFeaturesSet &BulletRT::Core::VulkanDeviceFeaturesSet::operator=(VulkanDeviceFeaturesSet &&featureSet) noexcept
{
    // TODO: return �X�e�[�g�����g�������ɑ}�����܂�
    if (this != &featureSet)
    {
        m_IndexMap = std::move(featureSet.m_IndexMap);
        m_Holders = std::move(featureSet.m_Holders);
        m_TailPNext = featureSet.m_TailPNext;
        featureSet.m_TailPNext = nullptr;
    }
    return *this;
}

bool BulletRT::Core::VulkanDeviceFeaturesSet::Impl_Contain(vk::StructureType sType) const noexcept
{
    return m_IndexMap.count(sType) > 0;
}

auto BulletRT::Core::VulkanDeviceFeaturesSet::Impl_Read(vk::StructureType sType) const noexcept -> const void *
{
    return Impl_Contain(sType) ? m_Holders.at(m_IndexMap.at(sType))->Read() : nullptr;
}

bool BulletRT::Core::VulkanDeviceFeaturesSet::Impl_Write(vk::StructureType sType, const void *pData) noexcept
{
    if (Impl_Contain(sType))
    {
        m_Holders.at(m_IndexMap.at(sType))->Write(pData);
        return true;
    }
    return false;
}

bool BulletRT::Core::VulkanDeviceFeaturesSet::Impl_Map(vk::StructureType sType, void **ppData) noexcept
{
    if (Impl_Contain(sType))
    {
        auto ptr = m_Holders.at(m_IndexMap.at(sType)).get();
        ptr->Map();
        if (!ppData)
        {
            return false;
        }
        *ppData = ptr->GetPointer();
        return true;
    }
    return false;
}

bool BulletRT::Core::VulkanDeviceFeaturesSet::Impl_Unmap(vk::StructureType sType) noexcept
{
    if (Impl_Contain(sType))
    {
        auto ptr = m_Holders.at(m_IndexMap.at(sType)).get();
        ptr->Unmap();
        return true;
    }
    return false;
}

auto BulletRT::Core::VulkanContext::GetHandle() noexcept -> VulkanContext &
{
    static VulkanContext context;
    return context;
}

void BulletRT::Core::VulkanContext::Initialize() noexcept
{
    auto &handle = GetHandle();
    if (!handle.m_DLL)
    {
        handle.m_DLL = std::make_unique<vk::DynamicLoader>();
        auto vkGetInstanceProcAddr = handle.m_DLL->getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
        VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);
    }
}

void BulletRT::Core::VulkanContext::Terminate() noexcept
{
    GetHandle().m_DLL.reset();
}

bool BulletRT::Core::VulkanContext::IsInitialized() noexcept
{
    return GetHandle().m_DLL != nullptr;
}

auto BulletRT::Core::VulkanInstance::New(const VulkanInstanceBuilder &builder) noexcept -> std::unique_ptr<VulkanInstance>
{
    auto applicationName = builder.GetApplicationName();
    auto engineName = builder.GetEngineName();
    auto applicationInfo = vk::ApplicationInfo()
                               .setApiVersion(builder.GetApiVersion())
                               .setApplicationVersion(builder.GetApplicationVersion())
                               .setEngineVersion(builder.GetEngineVersion())
                               .setPApplicationName(applicationName.c_str())
                               .setPEngineName(engineName.c_str());

    auto enabledExtSet = builder.GetExtensionSet();
    auto enabledExtNamesString = std::vector<std::string>(std::begin(enabledExtSet), std::end(enabledExtSet));
    auto enabledExtNames = std::vector<const char *>();
    enabledExtNames.reserve(enabledExtSet.size());
    for (auto &enabledExtNameString : enabledExtNamesString)
    {
        enabledExtNames.push_back(enabledExtNameString.c_str());
    }

    auto enabledLyrSet = builder.GetLayerSet();
    auto enabledLyrNamesString = std::vector<std::string>(std::begin(enabledLyrSet), std::end(enabledLyrSet));
    auto enabledLyrNames = std::vector<const char *>();
    enabledLyrNames.reserve(enabledLyrSet.size());
    for (auto &enabledLyrNameString : enabledLyrNamesString)
    {
        enabledLyrNames.push_back(enabledLyrNameString.c_str());
    }

    auto debugUtilsMessengerCreateInfos = builder.GetDebugUtilsMessengers();
    auto debugReportCallbackCreateInfo = builder.GetDebugReportCallback();
    auto instanceCreateInfo = vk::InstanceCreateInfo()
                                  .setPApplicationInfo(&applicationInfo)
                                  .setPEnabledExtensionNames(enabledExtNames)
                                  .setPEnabledLayerNames(enabledLyrNames);

    const void *pHead = nullptr;
    if (enabledExtSet.count(VK_EXT_DEBUG_UTILS_EXTENSION_NAME) > 0)
    {
        if (!debugUtilsMessengerCreateInfos.empty())
        {
            if (debugUtilsMessengerCreateInfos.size() > 1)
            {
                for (size_t i = 0; i < debugUtilsMessengerCreateInfos.size() - 1; ++i)
                {
                    debugUtilsMessengerCreateInfos[i].pNext = &debugUtilsMessengerCreateInfos[i + 1];
                }
            }
            pHead = &debugUtilsMessengerCreateInfos.front();
        }
    }
    else
    {
        debugUtilsMessengerCreateInfos.clear();
    }
    if (enabledExtSet.count(VK_EXT_DEBUG_REPORT_EXTENSION_NAME) > 0)
    {
        if (debugReportCallbackCreateInfo)
        {
            debugReportCallbackCreateInfo.value().pNext = pHead;
            pHead = &debugReportCallbackCreateInfo.value();
        }
    }
    else
    {
        debugReportCallbackCreateInfo = std::nullopt;
    }

    instanceCreateInfo.pNext = pHead;

    auto instance = vk::createInstanceUnique(instanceCreateInfo);

    if (instance)
    {
        VULKAN_HPP_DEFAULT_DISPATCHER.init(*instance);

        auto debugUtilsMessengers = std::vector<vk::UniqueDebugUtilsMessengerEXT>();
        auto debugReportCallback = vk::UniqueDebugReportCallbackEXT();

        if (!debugUtilsMessengerCreateInfos.empty())
        {
            for (auto &debugUtilsMessengerCreateInfo : debugUtilsMessengerCreateInfos)
            {
                debugUtilsMessengerCreateInfo.pNext = nullptr;
                debugUtilsMessengers.push_back(instance->createDebugUtilsMessengerEXTUnique(debugUtilsMessengerCreateInfo));
            }
        }
        if (debugReportCallbackCreateInfo)
        {
            debugReportCallbackCreateInfo.value().pNext = nullptr;
            debugReportCallback = instance->createDebugReportCallbackEXTUnique(debugReportCallbackCreateInfo.value());
        }

        auto vulkanInstance = new VulkanInstance();
        vulkanInstance->m_Instance = std::move(instance);
        vulkanInstance->m_DebugUtilsMessengers = std::move(debugUtilsMessengers);
        vulkanInstance->m_DebugReportCallback = std::move(debugReportCallback);
        vulkanInstance->m_ApiVersion = applicationInfo.apiVersion;
        vulkanInstance->m_EngineVersion = applicationInfo.engineVersion;
        vulkanInstance->m_ApplicationVersion = applicationInfo.applicationVersion;
        vulkanInstance->m_ApplicationName = applicationName;
        vulkanInstance->m_EngineName = engineName;
        vulkanInstance->m_DebugUtilsMessengerCreateInfos = std::move(debugUtilsMessengerCreateInfos);
        vulkanInstance->m_DebugReportCallbackCreateInfo = std::move(debugReportCallbackCreateInfo);
        vulkanInstance->m_EnabledExtNameSet = enabledExtSet;
        vulkanInstance->m_EnabledLyrNameSet = enabledLyrSet;
        return std::unique_ptr<VulkanInstance>(vulkanInstance);
    }

    return nullptr;
}

BulletRT::Core::VulkanInstance::~VulkanInstance() noexcept
{
    m_DebugUtilsMessengers.clear();
    m_DebugReportCallback.reset();
    m_Instance.reset();
}

BulletRT::Core::VulkanInstance::VulkanInstance() noexcept : m_Instance{}, m_DebugUtilsMessengers{}, m_DebugReportCallback{}
{
    m_ApiVersion = 0;
    m_ApplicationVersion = 0;
    m_EngineVersion = 0;
    m_ApplicationName = "";
    m_EngineName = "";
    m_EnabledExtNameSet = {};
    m_EnabledLyrNameSet = {};
    m_DebugReportCallbackCreateInfo = std::nullopt;
    m_DebugUtilsMessengerCreateInfos = {};
}

auto BulletRT::Core::VulkanInstanceBuilder::Build() const -> std::unique_ptr<VulkanInstance>
{
    return VulkanInstance::New(*this);
}

auto BulletRT::Core::VulkanDevice::New(const BulletRT::Core::VulkanDeviceBuilder &builder) noexcept -> std::unique_ptr<BulletRT::Core::VulkanDevice>
{
    auto physicalDevice = builder.GetPhysicalDevice();
    auto enabledExtSet = builder.GetExtensionSet();
    auto enabledExtNamesString = std::vector<std::string>(std::begin(enabledExtSet), std::end(enabledExtSet));
    auto enabledExtNames = std::vector<const char *>();
    auto enabledFeatureSet = builder.GetFeaturesSet();
    auto queueFamilySet = builder.GetQueueFamilySet();
    auto deviceQueueCreateInfos = std::vector<vk::DeviceQueueCreateInfo>();

    enabledExtNames.reserve(enabledExtSet.size());
    for (auto &enabledExtNameString : enabledExtNamesString)
    {
        enabledExtNames.push_back(enabledExtNameString.c_str());
    }
    for (auto &[queueFamilyIndex, queueFamily] : queueFamilySet)
    {
        deviceQueueCreateInfos.push_back(
            vk::DeviceQueueCreateInfo()
                .setQueueFamilyIndex(queueFamily.GetQueueFamilyIndex())
                .setQueuePriorities(queueFamily.GetQueueProperties()));
    }

    auto deviceCreateInfo = vk::DeviceCreateInfo()
                                .setQueueCreateInfos(deviceQueueCreateInfos)
                                .setPEnabledExtensionNames(enabledExtNames)
                                .setPNext(enabledFeatureSet.ReadHead());

    auto device = physicalDevice.createDeviceUnique(deviceCreateInfo);
    if (device)
    {
        VULKAN_HPP_DEFAULT_DISPATCHER.init(*device);
        auto vulkanDevice = new BulletRT::Core::VulkanDevice();
        vulkanDevice->m_LogigalDevice = std::move(device);
        vulkanDevice->m_PhysicalDevice = physicalDevice;
        vulkanDevice->m_EnabledExtNameSet = enabledExtSet;
        vulkanDevice->m_EnabledFeaturesSet = enabledFeatureSet;
        vulkanDevice->m_QueueFamilyMap = queueFamilySet;
        vulkanDevice->m_Instance = builder.GetInstance();
        return std::unique_ptr<BulletRT::Core::VulkanDevice>(vulkanDevice);
    }
    return nullptr;
}

BulletRT::Core::VulkanDevice::~VulkanDevice() noexcept
{
    m_LogigalDevice.reset();
}

auto BulletRT::Core::VulkanDevice::WaitForFences(const std::vector<const VulkanFence *> &fences, uint64_t timeOut, VkBool32 waitForAll) -> vk::Result
{
    auto fencesVk = std::vector<vk::Fence>();
    fencesVk.reserve(fences.size());
    for (auto &fence : fences)
    {
        if (fence)
        {
            fencesVk.push_back(fence->GetFenceVk());
        }
    }

    return m_LogigalDevice->waitForFences(fencesVk.size(), fencesVk.data(), waitForAll, timeOut);
}

auto BulletRT::Core::VulkanDevice::EnumerateQueues(uint32_t queueFamilyIndex) const -> std::vector<VulkanQueue>
{
    return BulletRT::Core::VulkanQueue::Enumerate(this, queueFamilyIndex);
}

auto BulletRT::Core::VulkanDevice::AcquireQueueFamily(uint32_t queueFamilyIndex) const -> std::optional<VulkanQueueFamily>
{
    if (SupportQueueFamily(queueFamilyIndex))
    {
        return m_QueueFamilyMap.at(queueFamilyIndex).Build(this);
    }
    return std::nullopt;
}

BulletRT::Core::VulkanDevice::VulkanDevice() noexcept : m_PhysicalDevice(), m_LogigalDevice()
{
    m_EnabledExtNameSet = {};
    m_EnabledFeaturesSet = {};
    m_QueueFamilyMap = {};
    m_Instance = nullptr;
}

auto BulletRT::Core::VulkanDeviceBuilder::Build() const -> std::unique_ptr<BulletRT::Core::VulkanDevice>
{
    return BulletRT::Core::VulkanDevice::New(*this);
}

auto BulletRT::Core::VulkanBuffer::New(const VulkanDevice *device, const VulkanBufferBuilder &builder) -> std::unique_ptr<VulkanBuffer>
{
    auto queueFamilyIndices = builder.GetQueueFamilyIndices();
    auto bufferCreateInfo = vk::BufferCreateInfo()
                                .setSize(builder.GetSize())
                                .setFlags(builder.GetFlags())
                                .setUsage(builder.GetUsage())
                                .setQueueFamilyIndices(queueFamilyIndices)
                                .setSharingMode(builder.GetSharingMode());
    auto buffer = device->GetDeviceVk().createBufferUnique(bufferCreateInfo);
    if (buffer)
    {
        auto vulkanBuffer = new VulkanBuffer();
        vulkanBuffer->m_Device = device;
        vulkanBuffer->m_Buffer = std::move(buffer);
        vulkanBuffer->m_Size = builder.GetSize();
        vulkanBuffer->m_Usage = builder.GetUsage();
        vulkanBuffer->m_Flags = builder.GetFlags();
        vulkanBuffer->m_QueueFamilyIndices = queueFamilyIndices;
        return std::unique_ptr<VulkanBuffer>(vulkanBuffer);
    }
    return nullptr;
}

auto BulletRT::Core::VulkanBufferBuilder::Build(const VulkanDevice *device) const noexcept -> std::unique_ptr<VulkanBuffer>
{
    return BulletRT::Core::VulkanBuffer::New(device, *this);
}

BulletRT::Core::VulkanBuffer::~VulkanBuffer() noexcept
{
    m_Buffer.reset();
}

BulletRT::Core::VulkanBuffer::VulkanBuffer() noexcept
{
    m_Buffer = {};
    m_Device = nullptr;
    m_Flags = {};
    m_Size = 0;
    m_Usage = {};
    m_QueueFamilyIndices = {};
}

auto BulletRT::Core::VulkanBuffer::QueryMemoryRequirements() const -> vk::MemoryRequirements
{
    return m_Device->GetDeviceVk().getBufferMemoryRequirements(m_Buffer.get());
}

auto BulletRT::Core::VulkanImage::New(const VulkanDevice *device, const VulkanImageBuilder &builder) -> std::unique_ptr<VulkanImage>
{
    auto queueFamilyIndices = builder.GetQueueFamilyIndices();
    auto imageCreateInfo = vk::ImageCreateInfo()
                               .setImageType(builder.GetImageType())
                               .setFlags(builder.GetFlags())
                               .setFormat(builder.GetFormat())
                               .setExtent(builder.GetExtent())
                               .setMipLevels(builder.GetMipLevels())
                               .setArrayLayers(builder.GetArrayLayers())
                               .setInitialLayout(builder.GetInitialLayout())
                               .setQueueFamilyIndices(queueFamilyIndices)
                               .setSharingMode(builder.GetSharingMode());

    auto image = device->GetDeviceVk().createImageUnique(imageCreateInfo);
    if (image)
    {
        auto vulkanImage = new VulkanImage();
        vulkanImage->m_Device = device;
        vulkanImage->m_Image = std::move(image);
        vulkanImage->m_ImageType = builder.GetImageType();
        vulkanImage->m_Format = builder.GetFormat();
        vulkanImage->m_Extent = builder.GetExtent();
        vulkanImage->m_Usage = builder.GetUsage();
        vulkanImage->m_Flags = builder.GetFlags();
        vulkanImage->m_MipLevels = builder.GetMipLevels();
        vulkanImage->m_ArrayLayers = builder.GetArrayLayers();
        vulkanImage->m_InitialLayout = builder.GetInitialLayout();
        vulkanImage->m_QueueFamilyIndices = queueFamilyIndices;
        return std::unique_ptr<VulkanImage>(vulkanImage);
    }
    return nullptr;
}

auto BulletRT::Core::VulkanImageBuilder::Build(const VulkanDevice *device) const noexcept -> std::unique_ptr<VulkanImage>
{
    return BulletRT::Core::VulkanImage::New(device, *this);
}

BulletRT::Core::VulkanImage::VulkanImage() noexcept
{
    m_Image = {};
    m_Device = nullptr;
    m_Flags = {};
    m_ImageType = vk::ImageType::e2D;
    m_Format = vk::Format::eUndefined;
    m_Extent = vk::Extent3D();
    m_MipLevels = 1;
    m_ArrayLayers = 1;
    m_Samples = vk::SampleCountFlagBits::e1;
    m_Usage = {};
    m_QueueFamilyIndices = {};
    m_InitialLayout = vk::ImageLayout::eUndefined;
}

BulletRT::Core::VulkanImage::~VulkanImage() noexcept
{
    m_Image.reset();
}

auto BulletRT::Core::VulkanDeviceMemory::New(const VulkanDevice *device, const VulkanDeviceMemoryBuilder &builder) -> std::unique_ptr<VulkanDeviceMemory>
{
    auto memoryAllocateFlagsInfo = builder.GetMemoryAllocateFlagsInfo();
    auto memoryDedicatedAllocateInfo = builder.GetMemoryDedicatedAllocateInfo();
    auto memoryAllocateInfo = vk::MemoryAllocateInfo()
                                  .setAllocationSize(builder.GetAllocationSize())
                                  .setMemoryTypeIndex(builder.GetMemoryTypeIndex());

    auto physicalDeviceProperties = device->GetPhysicalDeviceVk().getProperties();

    bool enableDedicatedAllocationKhr = false;
    bool enableDeviceGroupKhr = false;
    if ((physicalDeviceProperties.apiVersion >= VK_API_VERSION_1_1) ||
        device->SupportExtension(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME))
    {
        enableDedicatedAllocationKhr = true;
    }
    if ((physicalDeviceProperties.apiVersion >= VK_API_VERSION_1_1) ||
        device->SupportExtension(VK_KHR_DEVICE_GROUP_EXTENSION_NAME))
    {
        enableDeviceGroupKhr = true;
    }

    const void *pHead = nullptr;
    if (enableDedicatedAllocationKhr)
    {
        if (memoryDedicatedAllocateInfo)
        {
            memoryDedicatedAllocateInfo.value().pNext = pHead;
            pHead = &memoryDedicatedAllocateInfo.value();
        }
    }
    else
    {
        memoryDedicatedAllocateInfo = std::nullopt;
    }
    if (enableDeviceGroupKhr)
    {
        if (memoryAllocateFlagsInfo)
        {
            if (!device->SupportExtension(VK_EXT_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME) &&
                !device->SupportExtension(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME) &&
                physicalDeviceProperties.apiVersion < VK_API_VERSION_1_3)
            {
                memoryAllocateFlagsInfo.value().flags &= ~vk::MemoryAllocateFlagBits::eDeviceAddress;
            }
            memoryAllocateFlagsInfo.value().pNext = pHead;
            pHead = &memoryAllocateFlagsInfo.value();
        }
    }
    else
    {
        memoryAllocateFlagsInfo = std::nullopt;
    }

    memoryAllocateInfo.pNext = pHead;

    auto deviceMemory = device->GetDeviceVk().allocateMemoryUnique(memoryAllocateInfo);

    if (memoryDedicatedAllocateInfo)
    {
        memoryDedicatedAllocateInfo.value().pNext = nullptr;
    }
    if (memoryAllocateFlagsInfo)
    {
        memoryAllocateFlagsInfo.value().pNext = nullptr;
    }

    if (deviceMemory)
    {
        auto vulkanDeviceMemory = new VulkanDeviceMemory();
        vulkanDeviceMemory->m_Device = device;
        vulkanDeviceMemory->m_DeviceMemory = std::move(deviceMemory);
        vulkanDeviceMemory->m_AllocationSize = builder.GetAllocationSize();
        vulkanDeviceMemory->m_MemoryTypeIndex = builder.GetMemoryTypeIndex();
        vulkanDeviceMemory->m_MemoryAllocateFlagsInfo = memoryAllocateFlagsInfo;
        vulkanDeviceMemory->m_MemoryDedicatedAllocateInfo = memoryDedicatedAllocateInfo;
        return std::unique_ptr<VulkanDeviceMemory>(vulkanDeviceMemory);
    }
    return nullptr;
}

BulletRT::Core::VulkanDeviceMemory::VulkanDeviceMemory() noexcept
{
    m_Device = nullptr;
    m_DeviceMemory = {};
    m_AllocationSize = 0;
    m_MemoryTypeIndex = 0;
    m_MemoryAllocateFlagsInfo = std::nullopt;
    m_MemoryDedicatedAllocateInfo = std::nullopt;
}

BulletRT::Core::VulkanDeviceMemory::~VulkanDeviceMemory() noexcept
{
    m_DeviceMemory.reset();
}

auto BulletRT::Core::VulkanDeviceMemory::Map(void **pPData, vk::MemoryMapFlags flags) const -> vk::Result
{
    return m_Device->GetDeviceVk().mapMemory(m_DeviceMemory.get(), 0, m_AllocationSize, flags, pPData);
}

auto BulletRT::Core::VulkanDeviceMemory::Map(void **pPData, vk::DeviceSize size, vk::DeviceSize offset, vk::MemoryMapFlags flags) const -> vk::Result
{
    return m_Device->GetDeviceVk().mapMemory(m_DeviceMemory.get(), offset, size, flags, pPData);
}

void BulletRT::Core::VulkanDeviceMemory::Unmap() const
{
    return m_Device->GetDeviceVk().unmapMemory(m_DeviceMemory.get());
}

auto BulletRT::Core::VulkanQueueFamily::Acquire(const VulkanDevice *device, uint32_t queueFamilyIndex) noexcept -> std::optional<VulkanQueueFamily>
{
    auto queues = BulletRT::Core::VulkanQueue::Enumerate(device, queueFamilyIndex);
    if (!queues.empty())
    {
        auto queueFamily = VulkanQueueFamily();
        queueFamily.m_Device = device;
        queueFamily.m_Queues = queues;
        queueFamily.m_QueueFamilyIndex = queueFamilyIndex;
        return queueFamily;
    }
    return std::nullopt;
}

auto BulletRT::Core::VulkanQueue::Acquire(const VulkanDevice *device, uint32_t queueFamilyIndex, uint32_t queueIndex) -> std::optional<VulkanQueue>
{
    auto queueProperties = device->QueryQueuePriorities(queueFamilyIndex);
    if (queueProperties.size() > queueIndex)
    {
        VulkanQueue vulkanQueue;
        vulkanQueue.m_Device = device;
        vulkanQueue.m_Queue = device->GetDeviceVk().getQueue(queueFamilyIndex, queueIndex);
        vulkanQueue.m_Priority = queueProperties[queueIndex];
        vulkanQueue.m_QueueIndex = queueIndex;
        vulkanQueue.m_QueueFamilyIndex = queueFamilyIndex;
        return vulkanQueue;
    }
    return std::nullopt;
}

auto BulletRT::Core::VulkanQueue::Enumerate(const VulkanDevice *device, uint32_t queueFamilyIndex) -> std::vector<VulkanQueue>
{
    auto queueProperties = device->QueryQueuePriorities(queueFamilyIndex);
    if (queueProperties.size() > 0)
    {
        auto vulkanQueues = std::vector<VulkanQueue>();
        vulkanQueues.reserve(queueProperties.size());
        uint32_t i = 0;
        for (auto &queueProperty : queueProperties)
        {
            VulkanQueue vulkanQueue;
            vulkanQueue.m_Device = device;
            vulkanQueue.m_Queue = device->GetDeviceVk().getQueue(queueFamilyIndex, i);
            vulkanQueue.m_Priority = queueProperty;
            vulkanQueue.m_QueueIndex = i;
            vulkanQueue.m_QueueFamilyIndex = queueFamilyIndex;
            vulkanQueues.push_back(vulkanQueue);
            ++i;
        }
        return vulkanQueues;
    }
    return std::vector<VulkanQueue>();
}

BulletRT::Core::VulkanQueue::VulkanQueue() noexcept
{
    m_Device = nullptr;
    m_Queue = vk::Queue();
    m_Priority = 1.0f;
    m_QueueFamilyIndex = 0;
    m_QueueIndex = 0;
}

auto BulletRT::Core::VulkanQueueFamilyBuilder::Build(const VulkanDevice *device) const -> std::optional<VulkanQueueFamily>
{
    return BulletRT::Core::VulkanQueueFamily::Acquire(device, GetQueueFamilyIndex());
}

auto BulletRT::Core::VulkanQueueFamily::NewCommandPool() const -> std::unique_ptr<VulkanCommandPool>
{
    return BulletRT::Core::VulkanCommandPool::New(GetDevice(), GetQueueFamilyIndex());
}

auto BulletRT::Core::VulkanCommandPool::New(const VulkanDevice *device, uint32_t queueFamilyIndex) noexcept -> std::unique_ptr<VulkanCommandPool>
{
    auto commandPool = device->GetDeviceVk().createCommandPoolUnique(vk::CommandPoolCreateInfo()
                                                                         .setQueueFamilyIndex(queueFamilyIndex)
                                                                         .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer));
    if (commandPool)
    {
        auto vulkanCommandPool = new VulkanCommandPool();
        vulkanCommandPool->m_Device = device;
        vulkanCommandPool->m_CommandPool = std::move(commandPool);
        vulkanCommandPool->m_QueueFamilyIndex = queueFamilyIndex;
        return std::unique_ptr<VulkanCommandPool>(vulkanCommandPool);
    }
    return nullptr;
}

BulletRT::Core::VulkanCommandPool::~VulkanCommandPool() noexcept
{
    m_CommandPool.reset();
}

auto BulletRT::Core::VulkanCommandPool::GetDevice() const noexcept -> const VulkanDevice *
{
    return m_Device;
}

auto BulletRT::Core::VulkanCommandPool::GetCommandPoolVk() const noexcept -> vk::CommandPool
{
    return m_CommandPool.get();
}

auto BulletRT::Core::VulkanCommandPool::GetQueueFamilyIndex() const noexcept -> uint32_t
{
    return m_QueueFamilyIndex;
}

auto BulletRT::Core::VulkanCommandPool::NewCommandBuffer(vk::CommandBufferLevel level) const noexcept -> std::unique_ptr<VulkanCommandBuffer>
{
    return BulletRT::Core::VulkanCommandBuffer::New(this, level);
}

BulletRT::Core::VulkanCommandPool::VulkanCommandPool() noexcept
{
    m_CommandPool = {};
    m_Device = nullptr;
    m_QueueFamilyIndex = 0;
}

auto BulletRT::Core::VulkanCommandBuffer::New(const VulkanCommandPool *commandPool, vk::CommandBufferLevel commandBufferLevel) noexcept -> std::unique_ptr<VulkanCommandBuffer>
{
    auto commandBuffers = commandPool->GetDevice()->GetDeviceVk().allocateCommandBuffersUnique(
        vk::CommandBufferAllocateInfo().setCommandBufferCount(1).setCommandPool(commandPool->GetCommandPoolVk()).setLevel(commandBufferLevel));
    auto vulkanCommandBuffer = new VulkanCommandBuffer();
    vulkanCommandBuffer->m_CommandPool = commandPool;
    vulkanCommandBuffer->m_CommandBuffer = std::move(commandBuffers[0]);
    return std::unique_ptr<VulkanCommandBuffer>(vulkanCommandBuffer);
}

BulletRT::Core::VulkanCommandBuffer::~VulkanCommandBuffer() noexcept
{
    m_CommandBuffer.reset();
}
auto BulletRT::Core::VulkanCommandBuffer::GetCommandBufferVk() const noexcept -> vk::CommandBuffer
{
    return m_CommandBuffer.get();
}

auto BulletRT::Core::VulkanCommandBuffer::GetCommandPool() const noexcept -> const VulkanCommandPool *
{
    return m_CommandPool;
}

BulletRT::Core::VulkanCommandBuffer::VulkanCommandBuffer() noexcept
{
    m_CommandBuffer = {};
    m_CommandPool = nullptr;
}

auto BulletRT::Core::VulkanDeviceMemoryBuilder::Build(const VulkanDevice *device) const -> std::unique_ptr<VulkanDeviceMemory>
{
    return VulkanDeviceMemory::New(device, *this);
}

auto BulletRT::Core::VulkanMemoryBuffer::Bind(const VulkanBuffer *buffer, const VulkanDeviceMemory *memory, vk::DeviceSize memoryOffset) -> std::unique_ptr<VulkanMemoryBuffer>
{
    if (!buffer || !memory)
    {
        return nullptr;
    }
    buffer->GetDevice()->GetDeviceVk().bindBufferMemory(buffer->GetBufferVk(), memory->GetDeviceMemoryVk(), memoryOffset);
    auto memoryBuffer = new VulkanMemoryBuffer();
    memoryBuffer->m_Buffer = buffer;
    memoryBuffer->m_Memory = memory;
    memoryBuffer->m_MemoryOffset = memoryOffset;
    if (SupportDeviceAddress(buffer, memory))
    {
        memoryBuffer->m_DeviceAddress = buffer->GetDevice()->GetDeviceVk().getBufferAddress(vk::BufferDeviceAddressInfo().setBuffer(buffer->GetBufferVk()));
    }
    return std::unique_ptr<VulkanMemoryBuffer>(memoryBuffer);
}

BulletRT::Core::VulkanMemoryBuffer::~VulkanMemoryBuffer() noexcept
{
}

auto BulletRT::Core::VulkanMemoryBuffer::Map(void **pPData, vk::DeviceSize size, vk::DeviceSize offset, vk::MemoryMapFlags flags) const -> vk::Result
{
    return m_Memory->Map(pPData, size, m_MemoryOffset + offset, flags);
}

void BulletRT::Core::VulkanMemoryBuffer::Unmap() const
{
    m_Memory->Unmap();
}

bool BulletRT::Core::VulkanMemoryBuffer::SupportDeviceAddress(const VulkanBuffer *buffer, const VulkanDeviceMemory *memory) noexcept
{
    bool supportDeviceAddress = false;
    if (buffer->GetDevice()->SupportExtension(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME))
    {
        if (auto features = buffer->GetDevice()->QueryFeatures<vk::PhysicalDeviceBufferDeviceAddressFeaturesKHR>())
        {
            if (features.value().bufferDeviceAddress)
            {
                supportDeviceAddress = true;
            }
        }
    }
    else
    {
        if (auto features = buffer->GetDevice()->QueryFeatures<vk::PhysicalDeviceVulkan12Features>())
        {
            if (features.value().bufferDeviceAddress)
            {
                supportDeviceAddress = true;
            }
        }
    }
    if (!supportDeviceAddress)
    {
        return false;
    }
    if (!(buffer->GetUsage() & vk::BufferUsageFlagBits::eShaderDeviceAddress))
    {
        return false;
    }
    if (auto &flagsInfo = memory->GetMemoryAllocateFlagsInfo())
    {
        if (!(flagsInfo.value().flags & vk::MemoryAllocateFlagBits::eDeviceAddress))
        {
            return false;
        }
    }
    else
    {
        return false;
    }
    return true;
}

BulletRT::Core::VulkanMemoryBuffer::VulkanMemoryBuffer() noexcept
{
    m_Buffer = nullptr;
    m_Memory = nullptr;
    m_MemoryOffset = 0;
    m_DeviceAddress = std::nullopt;
}

auto BulletRT::Core::VulkanMemoryBuffer::Map(void **pPData, vk::MemoryMapFlags flags) const -> vk::Result
{
    return m_Memory->Map(pPData, m_Buffer->GetSize(), m_MemoryOffset, flags);
}

auto BulletRT::Core::VulkanMemoryImage::Bind(const VulkanImage *image, const VulkanDeviceMemory *memory, vk::DeviceSize memoryOffset) -> std::unique_ptr<VulkanMemoryImage>
{
    if (!image || !memory)
    {
        return nullptr;
    }
    image->GetDevice()->GetDeviceVk().bindImageMemory(image->GetImageVk(), memory->GetDeviceMemoryVk(), memoryOffset);
    auto memoryImage = new VulkanMemoryImage();
    memoryImage->m_Image = image;
    memoryImage->m_Memory = memory;
    memoryImage->m_MemoryOffset = memoryOffset;
    return std::unique_ptr<VulkanMemoryImage>(memoryImage);
}

BulletRT::Core::VulkanMemoryImage::~VulkanMemoryImage() noexcept
{
}

auto BulletRT::Core::VulkanMemoryImage::Map(void **pPData, vk::DeviceSize size, vk::DeviceSize offset, vk::MemoryMapFlags flags) const -> vk::Result
{
    return m_Memory->Map(pPData, size, m_MemoryOffset + offset, flags);
}

void BulletRT::Core::VulkanMemoryImage::Unmap() const
{
    return m_Memory->Unmap();
}

BulletRT::Core::VulkanMemoryImage::VulkanMemoryImage() noexcept
{
    m_Image = nullptr;
    m_Memory = nullptr;
    m_MemoryOffset = 0;
}

auto BulletRT::Core::VulkanFence::New(const VulkanDevice *device, bool isSignaled) -> std::unique_ptr<VulkanFence>
{
    auto fenceVk = device->GetDeviceVk().createFenceUnique(vk::FenceCreateInfo().setFlags(isSignaled ? vk::FenceCreateFlagBits::eSignaled : vk::FenceCreateFlags{}));
    auto fence = new VulkanFence();
    fence->m_Device = device;
    fence->m_Fence = std::move(fenceVk);
    return std::unique_ptr<VulkanFence>(fence);
}

BulletRT::Core::VulkanFence::~VulkanFence() noexcept
{
    m_Fence.reset();
}

BulletRT::Core::VulkanFence::VulkanFence() noexcept
{
    m_Fence = {};
    m_Device = nullptr;
}
auto VulkanFence::Wait(uint64_t timeout) const noexcept -> vk::Result
{
    vk::Fence fence = m_Fence.get();
    return m_Device->GetDeviceVk().waitForFences(1, &fence, VK_TRUE, timeout);
}

auto VulkanFence::QueryStatus() const noexcept -> vk::Result
{
    return m_Device->GetDeviceVk().getFenceStatus(m_Fence.get());
}

auto VulkanDevice::NewFence(bool isSignaled) -> std::unique_ptr<VulkanFence>
{
    return VulkanFence::New(this, isSignaled);
}

auto VulkanPipelineVertexInputStateDesc::GetVulkanPipelineVertexInputStateCreateInfoVk() const noexcept -> vk::PipelineVertexInputStateCreateInfo
{
    return vk::PipelineVertexInputStateCreateInfo()
        .setVertexBindingDescriptions(m_VertexBindingDescriptions)
        .setVertexAttributeDescriptions(m_VertexAttributeDescriptions)
        .setFlags(m_Flags);
}

auto VulkanPipelineVertexInputStateDesc::GetPipelineVertexInputDivisorStateCreateInfoEXT() const noexcept -> std::optional<vk::PipelineVertexInputDivisorStateCreateInfoEXT>
{
    if (!m_VertexBindingDivisors.empty())
    {
        return vk::PipelineVertexInputDivisorStateCreateInfoEXT().setVertexBindingDivisors(m_VertexBindingDivisors);
    }
    else
    {
        return std::nullopt;
    }
}

VulkanPipelineVertexInputStateDesc::VulkanPipelineVertexInputStateDesc() noexcept
{
}

VulkanPipelineVertexInputStateDesc::VulkanPipelineVertexInputStateDesc(const BulletRT::Core::VulkanPipelineVertexInputStateDesc &builder) noexcept
{
    m_Flags = builder.m_Flags;
    m_VertexBindingDescriptions = builder.m_VertexBindingDescriptions;
    m_VertexAttributeDescriptions = builder.m_VertexAttributeDescriptions;
    m_VertexBindingDivisors = builder.m_VertexBindingDivisors;
}

VulkanPipelineVertexInputStateDesc::VulkanPipelineVertexInputStateDesc(BulletRT::Core::VulkanPipelineVertexInputStateDesc &&builder) noexcept
{
    m_Flags = std::move(builder.m_Flags);
    m_VertexBindingDescriptions = std::move(builder.m_VertexBindingDescriptions);
    m_VertexAttributeDescriptions = std::move(builder.m_VertexAttributeDescriptions);
    m_VertexBindingDivisors = std::move(builder.m_VertexBindingDivisors);
}

BulletRT::Core::VulkanPipelineVertexInputStateDesc &VulkanPipelineVertexInputStateDesc::operator=(const BulletRT::Core::VulkanPipelineVertexInputStateDesc &builder) noexcept
{
    if (this != &builder)
    {
        m_Flags = builder.m_Flags;
        m_VertexBindingDescriptions = builder.m_VertexBindingDescriptions;
        m_VertexAttributeDescriptions = builder.m_VertexAttributeDescriptions;
        m_VertexBindingDivisors = builder.m_VertexBindingDivisors;
    }
    return *this;
}

BulletRT::Core::VulkanPipelineVertexInputStateDesc &VulkanPipelineVertexInputStateDesc::operator=(BulletRT::Core::VulkanPipelineVertexInputStateDesc &&builder) noexcept
{
    if (this != &builder)
    {
        m_Flags = std::move(builder.m_Flags);
        m_VertexBindingDescriptions = std::move(builder.m_VertexBindingDescriptions);
        m_VertexAttributeDescriptions = std::move(builder.m_VertexAttributeDescriptions);
        m_VertexBindingDivisors = std::move(builder.m_VertexBindingDivisors);
    }
    return *this;
}

VulkanGraphicsPipelineBuilder::VulkanGraphicsPipelineBuilder() noexcept
{
}

auto VulkanPipelineVertexInputStateDesc::AddVertexBindingDescription(const vk::VertexInputBindingDescription &bindingDesc) noexcept -> BulletRT::Core::VulkanPipelineVertexInputStateDesc &
{
    m_VertexBindingDescriptions.push_back(bindingDesc);
    return *this;
}

auto VulkanPipelineVertexInputStateDesc::SetVertexBindingDescription(size_t idx, const vk::VertexInputBindingDescription &bindingDesc) noexcept -> BulletRT::Core::VulkanPipelineVertexInputStateDesc &
{
    if (m_VertexBindingDescriptions.size() > idx)
    {
        m_VertexBindingDescriptions[idx] = bindingDesc;
    }
    return *this;
}

auto VulkanPipelineVertexInputStateDesc::GetVertexBindingDescriptions() const noexcept -> const std::vector<vk::VertexInputBindingDescription> &
{
    return m_VertexBindingDescriptions;
}

auto VulkanPipelineVertexInputStateDesc::GetVertexBindingDescription(size_t idx) const -> const vk::VertexInputBindingDescription &
{
    return m_VertexBindingDescriptions.at(idx);
}

auto VulkanPipelineVertexInputStateDesc::SetVertexAttributeDescriptions(const std::vector<vk::VertexInputAttributeDescription> &attributeDescs) noexcept -> BulletRT::Core::VulkanPipelineVertexInputStateDesc &
{
    m_VertexAttributeDescriptions = attributeDescs;
    return *this;
}

auto VulkanPipelineVertexInputStateDesc::SetVertexBindingDescriptions(const std::vector<vk::VertexInputBindingDescription> &bindingDescs) noexcept -> BulletRT::Core::VulkanPipelineVertexInputStateDesc &
{
    m_VertexBindingDescriptions = bindingDescs;
    return *this;
}

VulkanGraphicsPipelineBuilder::VulkanGraphicsPipelineBuilder(const BulletRT::Core::VulkanGraphicsPipelineBuilder &builder) noexcept
{
    m_Flags = builder.m_Flags;
    m_Stages = builder.m_Stages;
    m_VertexInputState = builder.m_VertexInputState;
    m_InputAssemblyState = builder.m_InputAssemblyState;
    m_TessellationState = builder.m_TessellationState;
    m_ViewportState = builder.m_ViewportState;
    m_RasterizationState = builder.m_RasterizationState;
    m_MultiSampleState = builder.m_MultiSampleState;
    m_DepthStencilState = builder.m_DepthStencilState;
    m_ColorBlendState = builder.m_ColorBlendState;
    m_DynamicStates = builder.m_DynamicStates;
    m_Layout = builder.m_Layout;
    m_RenderPass = builder.m_RenderPass;
    m_Subpass = builder.m_Subpass;
    m_BasePipelineHandle = builder.m_BasePipelineHandle;
    m_BasePipelineIndex = builder.m_BasePipelineIndex;
}

VulkanGraphicsPipelineBuilder::VulkanGraphicsPipelineBuilder(BulletRT::Core::VulkanGraphicsPipelineBuilder &&builder) noexcept
{
    m_Flags = std::move(builder.m_Flags);
    m_Stages = std::move(builder.m_Stages);
    m_VertexInputState = std::move(builder.m_VertexInputState);
    m_InputAssemblyState = std::move(builder.m_InputAssemblyState);
    m_TessellationState = std::move(builder.m_TessellationState);
    m_ViewportState = std::move(builder.m_ViewportState);
    m_RasterizationState = std::move(builder.m_RasterizationState);
    m_MultiSampleState = std::move(builder.m_MultiSampleState);
    m_DepthStencilState = std::move(builder.m_DepthStencilState);
    m_ColorBlendState = std::move(builder.m_ColorBlendState);
    m_DynamicStates = std::move(builder.m_DynamicStates);
    m_Layout = builder.m_Layout;
    builder.m_Layout = nullptr;
    m_RenderPass = builder.m_RenderPass;
    builder.m_RenderPass = nullptr;
    m_Subpass = std::move(builder.m_Subpass);
    m_BasePipelineHandle = builder.m_BasePipelineHandle;
    builder.m_BasePipelineHandle = nullptr;
    m_BasePipelineIndex = std::move(builder.m_BasePipelineIndex);
}

BulletRT::Core::VulkanGraphicsPipelineBuilder &VulkanGraphicsPipelineBuilder::operator=(const BulletRT::Core::VulkanGraphicsPipelineBuilder &builder) noexcept
{
    if (this != &builder)
    {
        m_Flags = builder.m_Flags;
        m_Stages = builder.m_Stages;
        m_VertexInputState = builder.m_VertexInputState;
        m_InputAssemblyState = builder.m_InputAssemblyState;
        m_TessellationState = builder.m_TessellationState;
        m_ViewportState = builder.m_ViewportState;
        m_RasterizationState = builder.m_RasterizationState;
        m_MultiSampleState = builder.m_MultiSampleState;
        m_DepthStencilState = builder.m_DepthStencilState;
        m_ColorBlendState = builder.m_ColorBlendState;
        m_DynamicStates = builder.m_DynamicStates;
        m_Layout = builder.m_Layout;
        m_RenderPass = builder.m_RenderPass;
        m_Subpass = builder.m_Subpass;
        m_BasePipelineHandle = builder.m_BasePipelineHandle;
        m_BasePipelineIndex = builder.m_BasePipelineIndex;
    }
    return *this;
}

BulletRT::Core::VulkanGraphicsPipelineBuilder &VulkanGraphicsPipelineBuilder::operator=(BulletRT::Core::VulkanGraphicsPipelineBuilder &&builder) noexcept
{
    if (this != &builder)
    {
        m_Flags = std::move(builder.m_Flags);
        m_Stages = std::move(builder.m_Stages);
        m_VertexInputState = std::move(builder.m_VertexInputState);
        m_InputAssemblyState = std::move(builder.m_InputAssemblyState);
        m_TessellationState = std::move(builder.m_TessellationState);
        m_ViewportState = std::move(builder.m_ViewportState);
        m_RasterizationState = std::move(builder.m_RasterizationState);
        m_MultiSampleState = std::move(builder.m_MultiSampleState);
        m_DepthStencilState = std::move(builder.m_DepthStencilState);
        m_ColorBlendState = std::move(builder.m_ColorBlendState);
        m_DynamicStates = std::move(builder.m_DynamicStates);
        m_Layout = builder.m_Layout;
        builder.m_Layout = nullptr;
        m_RenderPass = builder.m_RenderPass;
        builder.m_RenderPass = nullptr;
        m_Subpass = std::move(builder.m_Subpass);
        m_BasePipelineHandle = builder.m_BasePipelineHandle;
        builder.m_BasePipelineHandle = nullptr;
        m_BasePipelineIndex = std::move(builder.m_BasePipelineIndex);
    }
    return *this;
}

auto VulkanGraphicsPipelineBuilder::SetFlags(vk::PipelineCreateFlags flags) noexcept -> BulletRT::Core::VulkanGraphicsPipelineBuilder &
{
    m_Flags = flags;
    return *this;
}

auto VulkanGraphicsPipelineBuilder::GetFlags() const noexcept -> vk::PipelineCreateFlags
{
    return m_Flags;
}

auto VulkanGraphicsPipelineBuilder::SetStages(const std::vector<VulkanPipelineShaderStageDesc> &stages) noexcept -> BulletRT::Core::VulkanGraphicsPipelineBuilder &
{
    m_Stages = stages;
    return *this;
}

auto VulkanGraphicsPipelineBuilder::AddStage(const BulletRT::Core::VulkanPipelineShaderStageDesc &stage) noexcept -> BulletRT::Core::VulkanGraphicsPipelineBuilder &
{
    m_Stages.push_back(stage);
    return *this;
}

auto VulkanGraphicsPipelineBuilder::SetStage(size_t idx, const BulletRT::Core::VulkanPipelineShaderStageDesc &stage) noexcept -> BulletRT::Core::VulkanGraphicsPipelineBuilder &
{
    if (m_Stages.size() > idx)
    {
        m_Stages[idx] = stage;
    }
    return *this;
}

auto VulkanGraphicsPipelineBuilder::GetStages() const noexcept -> std::vector<VulkanPipelineShaderStageDesc>
{
    return m_Stages;
}

auto VulkanGraphicsPipelineBuilder::GetStage(size_t idx) const -> const BulletRT::Core::VulkanPipelineShaderStageDesc &
{
    return m_Stages.at(idx);
}

auto VulkanGraphicsPipelineBuilder::SetVertexInputState(const BulletRT::Core::VulkanPipelineVertexInputStateDesc &vertexInput) noexcept -> BulletRT::Core::VulkanGraphicsPipelineBuilder &
{
    m_VertexInputState = vertexInput;
    return *this;
}

auto VulkanGraphicsPipelineBuilder::GetVertexInputState() const noexcept -> const std::optional<VulkanPipelineVertexInputStateDesc> &
{
    return m_VertexInputState;
}

auto VulkanGraphicsPipelineBuilder::SetInputAssemblyState(const vk::PipelineInputAssemblyStateCreateInfo &inputAssembly) noexcept -> BulletRT::Core::VulkanGraphicsPipelineBuilder &
{
    m_InputAssemblyState = inputAssembly;
    return *this;
}

auto VulkanGraphicsPipelineBuilder::GetInputAssemblyState() const noexcept -> const std::optional<vk::PipelineInputAssemblyStateCreateInfo> &
{
    return m_InputAssemblyState;
}

auto VulkanGraphicsPipelineBuilder::SetTessellationState(const BulletRT::Core::VulkanPipelineTessellationStateDesc &tessellation) noexcept -> BulletRT::Core::VulkanGraphicsPipelineBuilder &
{
    m_TessellationState = tessellation;
    return *this;
}

auto VulkanGraphicsPipelineBuilder::GetTessellationState() const noexcept -> const std::optional<VulkanPipelineTessellationStateDesc> &
{
    return m_TessellationState;
}

auto VulkanGraphicsPipelineBuilder::SetViewportState(const BulletRT::Core::VulkanPipelineViewportStateDesc &viewport) noexcept -> BulletRT::Core::VulkanGraphicsPipelineBuilder &
{
    m_ViewportState = viewport;
    return *this;
}

auto VulkanGraphicsPipelineBuilder::GetViewportState() const noexcept -> const std::optional<BulletRT::Core::VulkanPipelineViewportStateDesc> &
{
    return m_ViewportState;
}

auto VulkanGraphicsPipelineBuilder::SetRasterizationState(const BulletRT::Core::VulkanPipelineRasterizationStateDesc &rasterization) noexcept -> BulletRT::Core::VulkanGraphicsPipelineBuilder &
{
    m_RasterizationState = rasterization;
    return *this;
}

auto VulkanGraphicsPipelineBuilder::GetRasterizationState() const noexcept -> const std::optional<VulkanPipelineRasterizationStateDesc> &
{
    return m_RasterizationState;
}

auto VulkanGraphicsPipelineBuilder::SetMultiSampleState(const BulletRT::Core::VulkanPipelineMultiSampleStateDesc &multiSample) noexcept -> BulletRT::Core::VulkanGraphicsPipelineBuilder &
{
    m_MultiSampleState = multiSample;
    return *this;
}

auto VulkanGraphicsPipelineBuilder::GetMultiSampleState() const noexcept -> const std::optional<VulkanPipelineMultiSampleStateDesc> &
{
    return m_MultiSampleState;
}

auto VulkanGraphicsPipelineBuilder::GetDepthStencilState(const vk::PipelineDepthStencilStateCreateInfo &depthStencil) noexcept -> BulletRT::Core::VulkanGraphicsPipelineBuilder &
{
    m_DepthStencilState = depthStencil;
    return *this;
}

auto VulkanGraphicsPipelineBuilder::SetDepthStencilState() const noexcept -> const std::optional<vk::PipelineDepthStencilStateCreateInfo> &
{
    return m_DepthStencilState;
}

auto VulkanGraphicsPipelineBuilder::SetColorBlendState(const BulletRT::Core::VulkanPipelineColorBlendStateDesc &colorBlendState) noexcept -> BulletRT::Core::VulkanGraphicsPipelineBuilder &
{
    m_ColorBlendState = colorBlendState;
    return *this;
}

auto VulkanGraphicsPipelineBuilder::GetColorBlendState() const noexcept -> const std::optional<VulkanPipelineColorBlendStateDesc> &
{
    return m_ColorBlendState;
}

auto VulkanGraphicsPipelineBuilder::SetLayout(const BulletRT::Core::VulkanPipelineLayout *layout) noexcept -> BulletRT::Core::VulkanGraphicsPipelineBuilder &
{
    m_Layout = layout;
    return *this;
}

auto VulkanGraphicsPipelineBuilder::GetLayout() const noexcept -> const BulletRT::Core::VulkanPipelineLayout *
{
    return m_Layout;
}

auto VulkanGraphicsPipelineBuilder::SetRenderPass(const BulletRT::Core::VulkanRenderPass *renderPass) noexcept -> BulletRT::Core::VulkanGraphicsPipelineBuilder &
{
    m_RenderPass = renderPass;
    return *this;
}

auto VulkanGraphicsPipelineBuilder::GetRenderPass() const noexcept -> const BulletRT::Core::VulkanRenderPass *
{
    return m_RenderPass;
}

auto VulkanGraphicsPipelineBuilder::SetSubpass(uint32_t subpass) noexcept -> BulletRT::Core::VulkanGraphicsPipelineBuilder &
{
    m_Subpass = subpass;
    return *this;
}

auto VulkanGraphicsPipelineBuilder::GetSubpass() const noexcept -> std::optional<uint32_t>
{
    return m_Subpass;
}

auto VulkanGraphicsPipelineBuilder::SetBasePipelineHandle(const BulletRT::Core::VulkanGraphicsPipeline *basePipeline) noexcept -> BulletRT::Core::VulkanGraphicsPipelineBuilder &
{
    if (basePipeline)
    {
        m_Flags |= vk::PipelineCreateFlagBits::eDerivative;
        if (m_BasePipelineIndex)
        {
            m_BasePipelineIndex = std::nullopt;
        }
        m_BasePipelineHandle = basePipeline;
    }
    return *this;
}

auto VulkanGraphicsPipelineBuilder::GetBasePipelineHandle() const noexcept -> const BulletRT::Core::VulkanGraphicsPipeline *
{
    return m_BasePipelineHandle;
}

auto VulkanGraphicsPipelineBuilder::SetBasePipelineIndex(uint32_t basePipelineIndex) noexcept -> BulletRT::Core::VulkanGraphicsPipelineBuilder &
{
    m_Flags |= vk::PipelineCreateFlagBits::eDerivative;
    if (m_BasePipelineHandle)
    {
        m_BasePipelineHandle = nullptr;
    }
    m_BasePipelineIndex = basePipelineIndex;
    return *this;
}

auto VulkanGraphicsPipelineBuilder::GetBasePipelineIndex() const noexcept -> std::optional<uint32_t>
{
    return m_BasePipelineIndex;
}

VulkanPipelineShaderStageDesc::VulkanPipelineShaderStageDesc() noexcept
{
}

VulkanPipelineShaderStageDesc::VulkanPipelineShaderStageDesc(const BulletRT::Core::VulkanPipelineShaderStageDesc &builder) noexcept
{
    m_Flags = builder.m_Flags;
    m_Stage = builder.m_Stage;
    m_Module = builder.m_Module;
    m_Name = builder.m_Name;
    m_SpecializationDesc = builder.m_SpecializationDesc;
    m_ModuleBuilder = builder.m_ModuleBuilder;
}

VulkanPipelineShaderStageDesc::VulkanPipelineShaderStageDesc(BulletRT::Core::VulkanPipelineShaderStageDesc &&builder) noexcept
{
    m_Flags = std::move(builder.m_Flags);
    m_Stage = std::move(builder.m_Stage);
    m_Module = builder.m_Module;
    builder.m_Module = nullptr;
    m_Name = std::move(builder.m_Name);
    m_SpecializationDesc = std::move(builder.m_SpecializationDesc);
    m_ModuleBuilder = std::move(builder.m_ModuleBuilder);
}

BulletRT::Core::VulkanPipelineShaderStageDesc &VulkanPipelineShaderStageDesc::operator=(const BulletRT::Core::VulkanPipelineShaderStageDesc &builder) noexcept
{
    if (this != &builder)
    {
        m_Flags = builder.m_Flags;
        m_Stage = builder.m_Stage;
        m_Module = builder.m_Module;
        m_Name = builder.m_Name;
        m_SpecializationDesc = builder.m_SpecializationDesc;
        m_ModuleBuilder = builder.m_ModuleBuilder;
    }
    return *this;
}

BulletRT::Core::VulkanPipelineShaderStageDesc &VulkanPipelineShaderStageDesc::operator=(BulletRT::Core::VulkanPipelineShaderStageDesc &&builder) noexcept
{
    if (this != &builder)
    {
        m_Flags = std::move(builder.m_Flags);
        m_Stage = std::move(builder.m_Stage);
        m_Module = builder.m_Module;
        builder.m_Module = nullptr;
        m_Name = std::move(builder.m_Name);
        m_SpecializationDesc = std::move(builder.m_SpecializationDesc);
        m_ModuleBuilder = std::move(builder.m_ModuleBuilder);
    }
    return *this;
}

auto VulkanPipelineShaderStageDesc::SetFlags(vk::PipelineShaderStageCreateFlags flags) noexcept -> BulletRT::Core::VulkanPipelineShaderStageDesc &
{
    m_Flags = flags;
    return *this;
}

auto VulkanPipelineShaderStageDesc::GetFlags() const noexcept -> vk::PipelineShaderStageCreateFlags
{
    return m_Flags;
}

auto VulkanPipelineShaderStageDesc::SetStage(vk::ShaderStageFlagBits stage) noexcept -> BulletRT::Core::VulkanPipelineShaderStageDesc &
{
    m_Stage = stage;
    return *this;
}

auto VulkanPipelineShaderStageDesc::GetStage() const noexcept -> vk::ShaderStageFlagBits
{
    return m_Stage;
}

auto VulkanPipelineShaderStageDesc::SetModule(const BulletRT::Core::VulkanShaderModule *module) noexcept -> BulletRT::Core::VulkanPipelineShaderStageDesc &
{
    if (m_ModuleBuilder)
    {
        m_ModuleBuilder = std::nullopt;
    }
    m_Module = module;
    return *this;
}

auto VulkanPipelineShaderStageDesc::GetModule() const noexcept -> const BulletRT::Core::VulkanShaderModule *
{
    return m_Module;
}

auto VulkanPipelineShaderStageDesc::GetModuleVk() const noexcept -> vk::ShaderModule
{
    return nullptr;
}

auto VulkanPipelineShaderStageDesc::SetName(const std::string &name) noexcept -> BulletRT::Core::VulkanPipelineShaderStageDesc &
{
    m_Name = name;
    return *this;
}

auto VulkanPipelineShaderStageDesc::GetName() const noexcept -> std::string
{
    return m_Name;
}

auto VulkanPipelineShaderStageDesc::SetSpecializationDesc(const BulletRT::Core::VulkanSpecializationDesc &specialization) noexcept -> BulletRT::Core::VulkanPipelineShaderStageDesc &
{
    m_SpecializationDesc = specialization;
    return *this;
}

auto VulkanPipelineShaderStageDesc::GetSpecializationDesc() const noexcept -> const std::optional<VulkanSpecializationDesc> &
{
    return m_SpecializationDesc;
}

auto VulkanPipelineShaderStageDesc::SetShaderModuleBuilder(const BulletRT::Core::VulkanShaderModuleBuilder &builder) noexcept -> BulletRT::Core::VulkanPipelineShaderStageDesc &
{
    if (m_Module)
    {
        m_Module = nullptr;
    }
    m_ModuleBuilder = builder;
    return *this;
}

auto VulkanPipelineShaderStageDesc::GetShaderModuleBuilder() const noexcept -> const std::optional<VulkanShaderModuleBuilder> &
{
    return m_ModuleBuilder;
}

auto VulkanPipelineShaderStageDesc::GetPipelineShaderStageCreateInfoVk() const noexcept -> vk::PipelineShaderStageCreateInfo
{
    return vk::PipelineShaderStageCreateInfo()
        .setFlags(m_Flags)
        .setStage(m_Stage)
        .setModule(GetModuleVk())
        .setPName(m_Name.data())
        .setPSpecializationInfo(nullptr);
}

auto VulkanPipelineShaderStageDesc::GetSpecializationInfoVk() const noexcept -> std::optional<vk::SpecializationInfo>
{
    return m_SpecializationDesc ? std::optional<vk::SpecializationInfo>(m_SpecializationDesc->GetSpecializationInfoVk()) : std::nullopt;
}

auto VulkanPipelineShaderStageDesc::GetShaderModuleCreateInfoVk() const noexcept -> std::optional<vk::ShaderModuleCreateInfo>
{
    return m_ModuleBuilder ? std::optional<vk::ShaderModuleCreateInfo>(m_ModuleBuilder->GetShaderModuleCreateInfoVk()) : std::nullopt;
}

VulkanSpecializationDesc::VulkanSpecializationDesc() noexcept
{
}

VulkanSpecializationDesc::VulkanSpecializationDesc(const BulletRT::Core::VulkanSpecializationDesc &builder) noexcept
{
    m_Entries = builder.m_Entries;
    m_Data = builder.m_Data;
}

VulkanSpecializationDesc::VulkanSpecializationDesc(BulletRT::Core::VulkanSpecializationDesc &&builder) noexcept
{
    m_Entries = std::move(builder.m_Entries);
    m_Data = std::move(builder.m_Data);
}

BulletRT::Core::VulkanSpecializationDesc &VulkanSpecializationDesc::operator=(const BulletRT::Core::VulkanSpecializationDesc &builder) noexcept
{
    if (this != &builder)
    {
        m_Entries = builder.m_Entries;
        m_Data = builder.m_Data;
    }
    return *this;
}

BulletRT::Core::VulkanSpecializationDesc &VulkanSpecializationDesc::operator=(BulletRT::Core::VulkanSpecializationDesc &&builder) noexcept
{
    if (this != &builder)
    {
        m_Entries = std::move(builder.m_Entries);
        m_Data = std::move(builder.m_Data);
    }
    return *this;
}

auto VulkanSpecializationDesc::SetEntryCount(size_t entryCount) noexcept -> BulletRT::Core::VulkanSpecializationDesc &
{
    m_Entries.resize(entryCount);
    return *this;
}

auto VulkanSpecializationDesc::SetEntries(const std::vector<vk::SpecializationMapEntry> &entries) noexcept -> BulletRT::Core::VulkanSpecializationDesc &
{
    m_Entries = entries;
    return *this;
}

auto VulkanSpecializationDesc::AddEntry(const vk::SpecializationMapEntry &entry) noexcept -> BulletRT::Core::VulkanSpecializationDesc &
{
    m_Entries.push_back(entry);
    return *this;
}

auto VulkanSpecializationDesc::SetEntry(size_t idx, const vk::SpecializationMapEntry &entry) noexcept -> BulletRT::Core::VulkanSpecializationDesc &
{
    if (idx < m_Entries.size())
    {
        m_Entries[idx] = entry;
    }
    return *this;
}

auto VulkanSpecializationDesc::GetEntries() const noexcept -> const std::vector<vk::SpecializationMapEntry> &
{
    return m_Entries;
}

auto VulkanSpecializationDesc::GetEntry(size_t idx) const -> const vk::SpecializationMapEntry &
{
    return m_Entries.at(idx);
}

auto VulkanSpecializationDesc::GetSpecializationInfoVk() const noexcept -> vk::SpecializationInfo
{
    return vk::SpecializationInfo().setPData(m_Data.data()).setDataSize(m_Data.size()).setMapEntries(m_Entries);
}

VulkanShaderModuleBuilder::VulkanShaderModuleBuilder() noexcept
{
}

VulkanShaderModuleBuilder::VulkanShaderModuleBuilder(const BulletRT::Core::VulkanShaderModuleBuilder &builder) noexcept
{
    m_Flags = builder.m_Flags;
    m_Codes = builder.m_Codes;
}

VulkanShaderModuleBuilder::VulkanShaderModuleBuilder(BulletRT::Core::VulkanShaderModuleBuilder &&builder) noexcept
{
    m_Flags = std::move(builder.m_Flags);
    m_Codes = std::move(builder.m_Codes);
}

BulletRT::Core::VulkanShaderModuleBuilder &VulkanShaderModuleBuilder::operator=(const BulletRT::Core::VulkanShaderModuleBuilder &builder) noexcept
{
    if (this != &builder)
    {
        m_Flags = builder.m_Flags;
        m_Codes = builder.m_Codes;
    }
    return *this;
}

BulletRT::Core::VulkanShaderModuleBuilder &VulkanShaderModuleBuilder::operator=(BulletRT::Core::VulkanShaderModuleBuilder &&builder) noexcept
{
    if (this != &builder)
    {
        m_Flags = std::move(builder.m_Flags);
        m_Codes = std::move(builder.m_Codes);
    }
    return *this;
}

auto VulkanShaderModuleBuilder::SetFlags(vk::ShaderModuleCreateFlags flags) noexcept -> BulletRT::Core::VulkanShaderModuleBuilder &
{
    m_Flags = flags;
    return *this;
}

auto VulkanShaderModuleBuilder::SetCodes(const std::vector<uint32_t> &codes) noexcept -> BulletRT::Core::VulkanShaderModuleBuilder &
{
    m_Codes = codes;
    return *this;
}

auto VulkanShaderModuleBuilder::GetCodes() const noexcept -> const std::vector<uint32_t> &
{
    return m_Codes;
}

auto VulkanShaderModuleBuilder::GetPCode() const noexcept -> const uint32_t *
{
    return m_Codes.data();
}

auto VulkanShaderModuleBuilder::GetCodeSize() const noexcept -> uint32_t
{
    return static_cast<uint32_t>(m_Codes.size());
}

auto VulkanShaderModuleBuilder::GetShaderModuleCreateInfoVk() const noexcept -> vk::ShaderModuleCreateInfo
{
    return vk::ShaderModuleCreateInfo().setFlags(m_Flags).setCode(m_Codes);
}

auto VulkanShaderModule::New(const BulletRT::Core::VulkanDevice *device, const BulletRT::Core::VulkanShaderModule::Builder &builder) -> std::unique_ptr<VulkanShaderModule>
{
    auto shaderModule = device->GetDeviceVk().createShaderModuleUnique(builder.GetShaderModuleCreateInfoVk());
    if (shaderModule)
    {
        auto vulkanShaderModule = std::unique_ptr<VulkanShaderModule>(new VulkanShaderModule());
        vulkanShaderModule->m_Device = device;
        vulkanShaderModule->m_ShaderModule = std::move(shaderModule);
        vulkanShaderModule->m_Flags = builder.GetFlags();
        vulkanShaderModule->m_Codes = builder.GetCodes();
        return vulkanShaderModule;
    }
    return nullptr;
}

VulkanShaderModule::~VulkanShaderModule() noexcept
{
    m_ShaderModule.reset();
}

auto VulkanShaderModule::GetDevice() const noexcept -> const BulletRT::Core::VulkanDevice *
{
    return m_Device;
}

auto VulkanShaderModule::GetDeviceVk() const noexcept -> vk::Device
{
    return m_Device ? m_Device->GetDeviceVk() : nullptr;
}

auto VulkanShaderModule::GetShaderModuleVk() const noexcept -> vk::ShaderModule
{
    return m_ShaderModule.get();
}

auto VulkanShaderModule::GetFlags() const noexcept -> vk::ShaderModuleCreateFlags
{
    return m_Flags;
}

auto VulkanShaderModule::GetCodes() const noexcept -> const std::vector<uint32_t> &
{
    return m_Codes;
}

auto VulkanShaderModule::GetPCode() const noexcept -> const uint32_t *
{
    return m_Codes.data();
}

auto VulkanShaderModule::GetCodeSize() const noexcept -> uint32_t
{
    return static_cast<uint32_t>(m_Codes.size());
}

VulkanShaderModule::VulkanShaderModule() noexcept
{
}

auto VulkanPipelineTessellationStateDesc::SetFlags(vk::PipelineTessellationStateCreateFlags flags) noexcept -> BulletRT::Core::VulkanPipelineTessellationStateDesc &
{
    m_Flags = flags;
    return *this;
}

auto VulkanPipelineTessellationStateDesc::GetFlags() const noexcept -> vk::PipelineTessellationStateCreateFlags
{
    return m_Flags;
}

auto VulkanPipelineTessellationStateDesc::SetPatchControlPoints(uint32_t patchControlPoints) noexcept -> BulletRT::Core::VulkanPipelineTessellationStateDesc &
{
    m_PatchControlPoints = patchControlPoints;
    return *this;
}

auto VulkanPipelineTessellationStateDesc::GetPatchControlPoints() const noexcept -> uint32_t
{
    return m_PatchControlPoints;
}

auto VulkanPipelineTessellationStateDesc::SetDomainOrigin(const vk::TessellationDomainOrigin &origin) noexcept -> BulletRT::Core::VulkanPipelineTessellationStateDesc &
{
    m_DomainOrigin = origin;
    return *this;
}

auto VulkanPipelineTessellationStateDesc::GetDomainOrigin() const noexcept -> const std::optional<vk::TessellationDomainOrigin> &
{
    return m_DomainOrigin;
}

auto VulkanPipelineTessellationStateDesc::GetPipelineTessellationStateInfoVk() const noexcept -> vk::PipelineTessellationStateCreateInfo
{
    return vk::PipelineTessellationStateCreateInfo().setFlags(m_Flags).setPatchControlPoints(m_PatchControlPoints);
}

VulkanPipelineTessellationStateDesc::VulkanPipelineTessellationStateDesc() noexcept
{
}

VulkanPipelineTessellationStateDesc::VulkanPipelineTessellationStateDesc(const BulletRT::Core::VulkanPipelineTessellationStateDesc &tessellation) noexcept
{
    m_Flags = tessellation.m_Flags;
    m_DomainOrigin = tessellation.m_DomainOrigin;
    m_PatchControlPoints = tessellation.m_PatchControlPoints;
}

VulkanPipelineTessellationStateDesc::VulkanPipelineTessellationStateDesc(BulletRT::Core::VulkanPipelineTessellationStateDesc &&tessellation) noexcept
{
    m_Flags = std::move(tessellation.m_Flags);
    m_DomainOrigin = std::move(tessellation.m_DomainOrigin);
    m_PatchControlPoints = std::move(tessellation.m_PatchControlPoints);
}

BulletRT::Core::VulkanPipelineTessellationStateDesc &VulkanPipelineTessellationStateDesc::operator=(const BulletRT::Core::VulkanPipelineTessellationStateDesc &tessellation) noexcept
{
    if (this != &tessellation)
    {
        m_Flags = tessellation.m_Flags;
        m_DomainOrigin = tessellation.m_DomainOrigin;
        m_PatchControlPoints = tessellation.m_PatchControlPoints;
    }
    return *this;
}

BulletRT::Core::VulkanPipelineTessellationStateDesc &VulkanPipelineTessellationStateDesc::operator=(BulletRT::Core::VulkanPipelineTessellationStateDesc &&tessellation) noexcept
{
    if (this != &tessellation)
    {
        m_Flags = std::move(tessellation.m_Flags);
        m_DomainOrigin = std::move(tessellation.m_DomainOrigin);
        m_PatchControlPoints = std::move(tessellation.m_PatchControlPoints);
    }
    return *this;
}

auto VulkanPipelineViewportStateDesc::SetNegativeOneToOne(vk::Bool32 negativeOneToOne) noexcept -> BulletRT::Core::VulkanPipelineViewportStateDesc &
{
    m_NegativeOneToOne = negativeOneToOne;
    return *this;
}

auto VulkanPipelineViewportStateDesc::GetNegativeOneToOne() const noexcept -> std::optional<vk::Bool32>
{
    return m_NegativeOneToOne;
}

auto VulkanPipelineViewportStateDesc::GetPipelineViewportStateCreateInfoVk() const noexcept -> vk::PipelineViewportStateCreateInfo
{
    return vk::PipelineViewportStateCreateInfo()
        .setFlags(m_Flags)
        .setViewports(m_Viewports)
        .setScissors(m_Scissors);
}

VulkanPipelineViewportStateDesc::VulkanPipelineViewportStateDesc() noexcept
{
}

VulkanPipelineViewportStateDesc::VulkanPipelineViewportStateDesc(const BulletRT::Core::VulkanPipelineViewportStateDesc &builder) noexcept
{
    m_Flags = builder.m_Flags;
    m_Viewports = builder.m_Viewports;
    m_Scissors = builder.m_Scissors;
    m_NegativeOneToOne = builder.m_NegativeOneToOne;
}

VulkanPipelineViewportStateDesc::VulkanPipelineViewportStateDesc(BulletRT::Core::VulkanPipelineViewportStateDesc &&builder) noexcept
{
    m_Flags = std::move(builder.m_Flags);
    m_Viewports = std::move(builder.m_Viewports);
    m_Scissors = std::move(builder.m_Scissors);
    m_NegativeOneToOne = std::move(builder.m_NegativeOneToOne);
}

BulletRT::Core::VulkanPipelineViewportStateDesc &VulkanPipelineViewportStateDesc::operator=(const BulletRT::Core::VulkanPipelineViewportStateDesc &builder) noexcept
{
    if (this != &builder)
    {
        m_Flags = builder.m_Flags;
        m_Viewports = builder.m_Viewports;
        m_Scissors = builder.m_Scissors;
        m_NegativeOneToOne = builder.m_NegativeOneToOne;
    }
    return *this;
}

BulletRT::Core::VulkanPipelineViewportStateDesc &VulkanPipelineViewportStateDesc::operator=(BulletRT::Core::VulkanPipelineViewportStateDesc &&builder) noexcept
{
    if (this != &builder)
    {
        m_Flags = std::move(builder.m_Flags);
        m_Viewports = std::move(builder.m_Viewports);
        m_Scissors = std::move(builder.m_Scissors);
        m_NegativeOneToOne = std::move(builder.m_NegativeOneToOne);
    }
    return *this;
}

auto VulkanPipelineViewportStateDesc::SetFlags(vk::PipelineViewportStateCreateFlags flags) noexcept -> BulletRT::Core::VulkanPipelineViewportStateDesc &
{
    m_Flags = flags;
    return *this;
}

auto VulkanPipelineViewportStateDesc::GetFlags() const noexcept -> vk::PipelineViewportStateCreateFlags
{
    return m_Flags;
}

auto VulkanPipelineViewportStateDesc::SetViewportCount(uint32_t viewportCount) noexcept -> BulletRT::Core::VulkanPipelineViewportStateDesc &
{
    m_Viewports.resize(viewportCount);
    return *this;
}

auto VulkanPipelineViewportStateDesc::SetViewports(const std::vector<vk::Viewport> &viewports) noexcept -> BulletRT::Core::VulkanPipelineViewportStateDesc &
{
    m_Viewports = viewports;
    return *this;
}

auto VulkanPipelineViewportStateDesc::AddViewport(const vk::Viewport &viewport) noexcept -> BulletRT::Core::VulkanPipelineViewportStateDesc &
{
    m_Viewports.push_back(viewport);
    return *this;
}

auto VulkanPipelineViewportStateDesc::SetViewport(size_t idx, const vk::Viewport &viewport) noexcept -> BulletRT::Core::VulkanPipelineViewportStateDesc &
{
    if (idx < m_Viewports.size())
    {
        m_Viewports[idx] = viewport;
    }
    return *this;
}

auto VulkanPipelineViewportStateDesc::GetViewports() const noexcept -> const std::vector<vk::Viewport> &
{
    return m_Viewports;
}

auto VulkanPipelineViewportStateDesc::GetViewport(size_t idx) const -> const vk::Viewport &
{
    return m_Viewports.at(idx);
}

auto VulkanPipelineViewportStateDesc::SetScissorCount(uint32_t scissorCount) noexcept -> BulletRT::Core::VulkanPipelineViewportStateDesc &
{
    m_Scissors.resize(scissorCount);
    return *this;
}

auto VulkanPipelineViewportStateDesc::SetScissors(const std::vector<vk::Rect2D> &scissors) noexcept -> BulletRT::Core::VulkanPipelineViewportStateDesc &
{
    m_Scissors = scissors;
    return *this;
}

auto VulkanPipelineViewportStateDesc::AddScissor(const vk::Rect2D &scissor) noexcept -> BulletRT::Core::VulkanPipelineViewportStateDesc &
{
    m_Scissors.push_back(scissor);
    return *this;
}

auto VulkanPipelineViewportStateDesc::SetScissor(size_t idx, const vk::Rect2D &scissor) noexcept -> BulletRT::Core::VulkanPipelineViewportStateDesc &
{
    if (idx < m_Scissors.size())
    {
        m_Scissors[idx] = scissor;
    }
    return *this;
}

auto VulkanPipelineViewportStateDesc::GetScissors() const noexcept -> const std::vector<vk::Rect2D> &
{
    return m_Scissors;
}

auto VulkanPipelineViewportStateDesc::GetScissor(size_t idx) const -> const vk::Rect2D &
{
    return m_Scissors.at(idx);
}

VulkanPipelineRasterizationStateDesc::VulkanPipelineRasterizationStateDesc() noexcept {}

VulkanPipelineRasterizationStateDesc::VulkanPipelineRasterizationStateDesc(const BulletRT::Core::VulkanPipelineRasterizationStateDesc &builder) noexcept
{
    m_Flags = builder.m_Flags;
    m_DepthClampEnable = builder.m_DepthClampEnable;
    m_RasterizerDiscardEnable = builder.m_RasterizerDiscardEnable;
    m_PolygonMode = builder.m_PolygonMode;
    m_CullMode = builder.m_CullMode;
    m_FrontFace = builder.m_FrontFace;
    m_DepthBiasEnable = builder.m_DepthBiasEnable;
    m_DepthBiasConstantFactor = builder.m_DepthBiasConstantFactor;
    m_DepthBiasClamp = builder.m_DepthBiasClamp;
    m_DepthBiasSlopeFactor = builder.m_DepthBiasSlopeFactor;
    m_LineWidth = builder.m_LineWidth;
}

VulkanPipelineRasterizationStateDesc::VulkanPipelineRasterizationStateDesc(BulletRT::Core::VulkanPipelineRasterizationStateDesc &&builder) noexcept
{
    m_Flags = std::move(builder.m_Flags);
    m_DepthClampEnable = std::move(builder.m_DepthClampEnable);
    m_RasterizerDiscardEnable = std::move(builder.m_RasterizerDiscardEnable);
    m_PolygonMode = std::move(builder.m_PolygonMode);
    m_CullMode = std::move(builder.m_CullMode);
    m_FrontFace = std::move(builder.m_FrontFace);
    m_DepthBiasEnable = std::move(builder.m_DepthBiasEnable);
    m_DepthBiasConstantFactor = std::move(builder.m_DepthBiasConstantFactor);
    m_DepthBiasClamp = std::move(builder.m_DepthBiasClamp);
    m_DepthBiasSlopeFactor = std::move(builder.m_DepthBiasSlopeFactor);
    m_LineWidth = std::move(builder.m_LineWidth);
}

BulletRT::Core::VulkanPipelineRasterizationStateDesc &VulkanPipelineRasterizationStateDesc::operator=(const BulletRT::Core::VulkanPipelineRasterizationStateDesc &builder) noexcept
{
    if (this != &builder)
    {
        m_Flags = builder.m_Flags;
        m_DepthClampEnable = builder.m_DepthClampEnable;
        m_RasterizerDiscardEnable = builder.m_RasterizerDiscardEnable;
        m_PolygonMode = builder.m_PolygonMode;
        m_CullMode = builder.m_CullMode;
        m_FrontFace = builder.m_FrontFace;
        m_DepthBiasEnable = builder.m_DepthBiasEnable;
        m_DepthBiasConstantFactor = builder.m_DepthBiasConstantFactor;
        m_DepthBiasClamp = builder.m_DepthBiasClamp;
        m_DepthBiasSlopeFactor = builder.m_DepthBiasSlopeFactor;
        m_LineWidth = builder.m_LineWidth;
    }
    return *this;
}

BulletRT::Core::VulkanPipelineRasterizationStateDesc &VulkanPipelineRasterizationStateDesc::operator=(BulletRT::Core::VulkanPipelineRasterizationStateDesc &&builder) noexcept
{
    if (this != &builder)
    {
        m_Flags = std::move(builder.m_Flags);
        m_DepthClampEnable = std::move(builder.m_DepthClampEnable);
        m_RasterizerDiscardEnable = std::move(builder.m_RasterizerDiscardEnable);
        m_PolygonMode = std::move(builder.m_PolygonMode);
        m_CullMode = std::move(builder.m_CullMode);
        m_FrontFace = std::move(builder.m_FrontFace);
        m_DepthBiasEnable = std::move(builder.m_DepthBiasEnable);
        m_DepthBiasConstantFactor = std::move(builder.m_DepthBiasConstantFactor);
        m_DepthBiasClamp = std::move(builder.m_DepthBiasClamp);
        m_DepthBiasSlopeFactor = std::move(builder.m_DepthBiasSlopeFactor);
        m_LineWidth = std::move(builder.m_LineWidth);
    }
    return *this;
}

auto VulkanPipelineRasterizationStateDesc::GetFlags() const noexcept -> vk::PipelineRasterizationStateCreateFlags
{
    return m_Flags;
}

auto VulkanPipelineRasterizationStateDesc::SetFlags(vk::PipelineRasterizationStateCreateFlags flags) noexcept -> BulletRT::Core::VulkanPipelineRasterizationStateDesc &
{
    m_Flags = flags;
    return *this;
}

auto VulkanPipelineRasterizationStateDesc::SetDepthClampEnable(vk::Bool32 depthClampEnable) noexcept -> BulletRT::Core::VulkanPipelineRasterizationStateDesc &
{
    m_DepthBiasClamp = depthClampEnable;
    return *this;
}

auto VulkanPipelineRasterizationStateDesc::GetDepthClampEnable() const noexcept -> vk::Bool32
{
    return m_DepthClampEnable;
}

auto VulkanPipelineRasterizationStateDesc::SetRasterizerDiscardEnable(vk::Bool32 rasterizerDiscardEnable) noexcept -> BulletRT::Core::VulkanPipelineRasterizationStateDesc &
{
    m_RasterizerDiscardEnable = rasterizerDiscardEnable;
    return *this;
}

auto VulkanPipelineRasterizationStateDesc::GetRasterizerDiscardEnable() const noexcept -> vk::Bool32
{
    return m_RasterizerDiscardEnable;
}

auto VulkanPipelineRasterizationStateDesc::SetPolygonMode(vk::PolygonMode polygonMode) noexcept -> BulletRT::Core::VulkanPipelineRasterizationStateDesc &
{
    m_PolygonMode = polygonMode;
    return *this;
}

auto VulkanPipelineRasterizationStateDesc::GetPolygonMode() const noexcept -> vk::PolygonMode
{
    return m_PolygonMode;
}

auto VulkanPipelineRasterizationStateDesc::GetCullMode() const noexcept -> vk::CullModeFlags
{
    return m_CullMode;
}

auto VulkanPipelineRasterizationStateDesc::SetCullMode(vk::CullModeFlags cullMode) noexcept -> BulletRT::Core::VulkanPipelineRasterizationStateDesc &
{
    m_CullMode = cullMode;
    return *this;
}

auto VulkanPipelineRasterizationStateDesc::GetFrontFace() const noexcept -> vk::FrontFace
{
    return m_FrontFace;
}

auto VulkanPipelineRasterizationStateDesc::SetFrontFace(vk::FrontFace frontFace) noexcept -> BulletRT::Core::VulkanPipelineRasterizationStateDesc &
{
    m_FrontFace = frontFace;
    return *this;
}

auto VulkanPipelineRasterizationStateDesc::SetDepthBiasEnable(vk::Bool32 depthBiasEnable) noexcept -> BulletRT::Core::VulkanPipelineRasterizationStateDesc &
{
    m_DepthBiasEnable = depthBiasEnable;
    return *this;
}

auto VulkanPipelineRasterizationStateDesc::GetDepthBiasEnable() const noexcept -> vk::Bool32
{
    return m_DepthBiasEnable;
}

auto VulkanPipelineRasterizationStateDesc::SetDepthBiasConstantFactor(float depthBiasConstantFactor) noexcept -> BulletRT::Core::VulkanPipelineRasterizationStateDesc &
{
    m_DepthBiasConstantFactor = depthBiasConstantFactor;
    return *this;
}

auto VulkanPipelineRasterizationStateDesc::GetDepthBiasConstantFactor() const noexcept -> float
{
    return m_DepthBiasConstantFactor;
}

auto VulkanPipelineRasterizationStateDesc::SetDepthBiasClamp(float depthBiasClamp) noexcept -> BulletRT::Core::VulkanPipelineRasterizationStateDesc &
{
    m_DepthBiasClamp = depthBiasClamp;
    return *this;
}

auto VulkanPipelineRasterizationStateDesc::GetDepthBiasClamp() const noexcept -> float
{
    return m_DepthBiasClamp;
}

auto VulkanPipelineRasterizationStateDesc::SetDepthBiasSlopeFactor(float depthBiasSlopeFactor) noexcept -> BulletRT::Core::VulkanPipelineRasterizationStateDesc &
{
    m_DepthBiasSlopeFactor = depthBiasSlopeFactor;
    return *this;
}

auto VulkanPipelineRasterizationStateDesc::GetDepthBiasSlopeFactor() const noexcept -> float
{
    return m_DepthBiasSlopeFactor;
}

auto VulkanPipelineRasterizationStateDesc::SetLineWidth(float lineWidth) noexcept -> BulletRT::Core::VulkanPipelineRasterizationStateDesc &
{
    m_LineWidth = lineWidth;
    return *this;
}

auto VulkanPipelineRasterizationStateDesc::GetLineWidth() const noexcept -> float
{
    return m_LineWidth;
}

VulkanPipelineMultiSampleStateDesc::VulkanPipelineMultiSampleStateDesc() noexcept
{
}

VulkanPipelineMultiSampleStateDesc::VulkanPipelineMultiSampleStateDesc(const BulletRT::Core::VulkanPipelineMultiSampleStateDesc &builder) noexcept
{
    m_Flags = builder.m_Flags;
    m_RasterizationSamples = builder.m_RasterizationSamples;
    m_SampleShadingEnable = builder.m_SampleShadingEnable;
    m_MinSampleShading = builder.m_MinSampleShading;
    m_SampleMasks = builder.m_SampleMasks;
    m_AlphaToCoverageEnable = builder.m_AlphaToCoverageEnable;
    m_AlphaToOneEnable = builder.m_AlphaToOneEnable;
}

VulkanPipelineMultiSampleStateDesc::VulkanPipelineMultiSampleStateDesc(BulletRT::Core::VulkanPipelineMultiSampleStateDesc &&builder) noexcept
{
    m_Flags = std::move(builder.m_Flags);
    m_RasterizationSamples = std::move(builder.m_RasterizationSamples);
    m_SampleShadingEnable = std::move(builder.m_SampleShadingEnable);
    m_MinSampleShading = std::move(builder.m_MinSampleShading);
    m_SampleMasks = std::move(builder.m_SampleMasks);
    m_AlphaToCoverageEnable = std::move(builder.m_AlphaToCoverageEnable);
    m_AlphaToOneEnable = std::move(builder.m_AlphaToOneEnable);
}

BulletRT::Core::VulkanPipelineMultiSampleStateDesc &VulkanPipelineMultiSampleStateDesc::operator=(const BulletRT::Core::VulkanPipelineMultiSampleStateDesc &builder) noexcept
{
    if (this != &builder)
    {
        m_Flags = builder.m_Flags;
        m_RasterizationSamples = builder.m_RasterizationSamples;
        m_SampleShadingEnable = builder.m_SampleShadingEnable;
        m_MinSampleShading = builder.m_MinSampleShading;
        m_SampleMasks = builder.m_SampleMasks;
        m_AlphaToCoverageEnable = builder.m_AlphaToCoverageEnable;
        m_AlphaToOneEnable = builder.m_AlphaToOneEnable;
    }
    return *this;
}

BulletRT::Core::VulkanPipelineMultiSampleStateDesc &VulkanPipelineMultiSampleStateDesc::operator=(BulletRT::Core::VulkanPipelineMultiSampleStateDesc &&builder) noexcept
{
    if (this != &builder)
    {
        m_Flags = builder.m_Flags;
        m_RasterizationSamples = builder.m_RasterizationSamples;
        m_SampleShadingEnable = builder.m_SampleShadingEnable;
        m_MinSampleShading = builder.m_MinSampleShading;
        m_SampleMasks = builder.m_SampleMasks;
        m_AlphaToCoverageEnable = builder.m_AlphaToCoverageEnable;
        m_AlphaToOneEnable = builder.m_AlphaToOneEnable;
    }
    return *this;
}

auto VulkanPipelineMultiSampleStateDesc::GetFlags() const noexcept -> vk::PipelineMultisampleStateCreateFlags
{
    return m_Flags;
}

auto VulkanPipelineMultiSampleStateDesc::SetFlags(vk::PipelineMultisampleStateCreateFlags flags) noexcept -> BulletRT::Core::VulkanPipelineMultiSampleStateDesc &
{
    m_Flags = flags;
    return *this;
}

auto VulkanPipelineMultiSampleStateDesc::GetRasterizationSamples() const noexcept -> vk::SampleCountFlagBits
{
    return m_RasterizationSamples;
}

auto VulkanPipelineMultiSampleStateDesc::SetRasterizationSamples(vk::SampleCountFlagBits rasterizationSamples) noexcept -> BulletRT::Core::VulkanPipelineMultiSampleStateDesc &
{
    m_RasterizationSamples = rasterizationSamples;
    return *this;
}

auto VulkanPipelineMultiSampleStateDesc::GetSampleShadingEnable() const noexcept -> vk::Bool32
{
    return m_SampleShadingEnable;
}

auto VulkanPipelineMultiSampleStateDesc::SetSampleShadingEnable(vk::Bool32 sampleShadingEnable) noexcept -> BulletRT::Core::VulkanPipelineMultiSampleStateDesc &
{
    m_SampleShadingEnable = sampleShadingEnable;
    return *this;
}

auto VulkanPipelineMultiSampleStateDesc::GetMinSampleShading() const noexcept -> float
{
    return m_MinSampleShading;
}

auto VulkanPipelineMultiSampleStateDesc::SetMinSampleShading(float minSampleShading) noexcept -> BulletRT::Core::VulkanPipelineMultiSampleStateDesc &
{
    m_MinSampleShading = minSampleShading;
    return *this;
}

auto VulkanPipelineMultiSampleStateDesc::GetAlphaToCoverageEnable() const noexcept -> vk::Bool32
{
    return m_AlphaToCoverageEnable;
}

auto VulkanPipelineMultiSampleStateDesc::SetAlphaToCoverageEnable(vk::Bool32 alphaToCoverageEnable) noexcept -> BulletRT::Core::VulkanPipelineMultiSampleStateDesc &
{
    m_AlphaToCoverageEnable = alphaToCoverageEnable;
    return *this;
}

auto VulkanPipelineMultiSampleStateDesc::GetAlphaToOneEnable() const noexcept -> vk::Bool32
{
    return m_AlphaToOneEnable;
}

auto VulkanPipelineMultiSampleStateDesc::SetAlphaToOneEnable(vk::Bool32 alphaToOneEnable) noexcept -> BulletRT::Core::VulkanPipelineMultiSampleStateDesc &
{
    m_AlphaToOneEnable = alphaToOneEnable;
    return *this;
}

VulkanPipelineColorBlendStateDesc::VulkanPipelineColorBlendStateDesc() noexcept
{
}

VulkanPipelineColorBlendStateDesc::VulkanPipelineColorBlendStateDesc(const BulletRT::Core::VulkanPipelineColorBlendStateDesc &builder) noexcept
{
    m_Flags = builder.m_Flags;
    m_LogicOpEnable = builder.m_LogicOpEnable;
    m_LogicOp = builder.m_LogicOp;
    m_Attachments = builder.m_Attachments;
    m_BlendConstants[0] = builder.m_BlendConstants[0];
    m_BlendConstants[1] = builder.m_BlendConstants[1];
    m_BlendConstants[2] = builder.m_BlendConstants[2];
    m_BlendConstants[3] = builder.m_BlendConstants[3];
}

VulkanPipelineColorBlendStateDesc::VulkanPipelineColorBlendStateDesc(BulletRT::Core::VulkanPipelineColorBlendStateDesc &&builder) noexcept
{
    m_Flags = std::move(builder.m_Flags);
    m_LogicOpEnable = std::move(builder.m_LogicOpEnable);
    m_LogicOp = std::move(builder.m_LogicOp);
    m_Attachments = std::move(builder.m_Attachments);
    m_BlendConstants[0] = std::move(builder.m_BlendConstants[0]);
    m_BlendConstants[1] = std::move(builder.m_BlendConstants[1]);
    m_BlendConstants[2] = std::move(builder.m_BlendConstants[2]);
    m_BlendConstants[3] = std::move(builder.m_BlendConstants[3]);
}

BulletRT::Core::VulkanPipelineColorBlendStateDesc &VulkanPipelineColorBlendStateDesc::operator=(const BulletRT::Core::VulkanPipelineColorBlendStateDesc &builder) noexcept
{
    if (this != &builder)
    {
        m_Flags = builder.m_Flags;
        m_LogicOpEnable = builder.m_LogicOpEnable;
        m_LogicOp = builder.m_LogicOp;
        m_Attachments = builder.m_Attachments;
        m_BlendConstants[0] = builder.m_BlendConstants[0];
        m_BlendConstants[1] = builder.m_BlendConstants[1];
        m_BlendConstants[2] = builder.m_BlendConstants[2];
        m_BlendConstants[3] = builder.m_BlendConstants[3];
    }
    return *this;
}

BulletRT::Core::VulkanPipelineColorBlendStateDesc &VulkanPipelineColorBlendStateDesc::operator=(BulletRT::Core::VulkanPipelineColorBlendStateDesc &&builder) noexcept
{
    if (this != &builder)
    {
        m_Flags = std::move(builder.m_Flags);
        m_LogicOpEnable = std::move(builder.m_LogicOpEnable);
        m_LogicOp = std::move(builder.m_LogicOp);
        m_Attachments = std::move(builder.m_Attachments);
        m_BlendConstants[0] = std::move(builder.m_BlendConstants[0]);
        m_BlendConstants[1] = std::move(builder.m_BlendConstants[1]);
        m_BlendConstants[2] = std::move(builder.m_BlendConstants[2]);
        m_BlendConstants[3] = std::move(builder.m_BlendConstants[3]);
    }
    return *this;
}

auto VulkanPipelineColorBlendStateDesc::GetFlags() const noexcept -> vk::PipelineColorBlendStateCreateFlags
{
    return m_Flags;
}

auto VulkanPipelineColorBlendStateDesc::SetFlags(vk::PipelineColorBlendStateCreateFlags flags) noexcept -> BulletRT::Core::VulkanPipelineColorBlendStateDesc &
{
    m_Flags = flags;
    return *this;
}

auto VulkanPipelineColorBlendStateDesc::SetLogicOpEnable(vk::Bool32 logicOpEnable) noexcept -> BulletRT::Core::VulkanPipelineColorBlendStateDesc &
{
    m_LogicOpEnable = logicOpEnable;
    return *this;
}

auto VulkanPipelineColorBlendStateDesc::GetLogicOpEnable() const noexcept -> vk::Bool32
{
    return m_LogicOpEnable;
}

auto VulkanPipelineColorBlendStateDesc::SetAttachmentCount(uint32_t attachmentCount) noexcept -> BulletRT::Core::VulkanPipelineColorBlendStateDesc &
{
    m_Attachments.resize(attachmentCount);
    return *this;
}

auto VulkanPipelineColorBlendStateDesc::GetAttachmentCount() const noexcept -> uint32_t
{
    return static_cast<uint32_t>(m_Attachments.size());
}

auto VulkanPipelineColorBlendStateDesc::AddAttachment(const vk::PipelineColorBlendAttachmentState &attachment) noexcept -> BulletRT::Core::VulkanPipelineColorBlendStateDesc &
{
    m_Attachments.push_back(attachment);
    return *this;
}

auto VulkanPipelineColorBlendStateDesc::SetAttachment(size_t idx, const vk::PipelineColorBlendAttachmentState &attachment) noexcept -> BulletRT::Core::VulkanPipelineColorBlendStateDesc &
{
    if (idx < m_Attachments.size())
    {
        m_Attachments[idx] = attachment;
    }
    return *this;
}

auto VulkanPipelineColorBlendStateDesc::GetAttachment(size_t idx) const -> const vk::PipelineColorBlendAttachmentState &
{
    return m_Attachments.at(idx);
}

auto VulkanPipelineColorBlendStateDesc::SetAttachments(const std::vector<vk::PipelineColorBlendAttachmentState> &attachments) noexcept -> BulletRT::Core::VulkanPipelineColorBlendStateDesc &
{
    m_Attachments = attachments;
    return *this;
}

auto VulkanPipelineColorBlendStateDesc::GetAttachments() const noexcept -> const std::vector<vk::PipelineColorBlendAttachmentState> &
{
    return m_Attachments;
}

auto VulkanPipelineColorBlendStateDesc::SetBlendConstants(const std::array<float, 4> &blendConstants) noexcept -> BulletRT::Core::VulkanPipelineColorBlendStateDesc &
{
    m_BlendConstants[0] = blendConstants[0];
    m_BlendConstants[1] = blendConstants[1];
    m_BlendConstants[2] = blendConstants[2];
    m_BlendConstants[3] = blendConstants[3];
    return *this;
}

auto VulkanPipelineColorBlendStateDesc::GetBlendConstants() const noexcept -> std::array<float, 4>
{
    return std::array<float, 4>{m_BlendConstants[0], m_BlendConstants[1], m_BlendConstants[2], m_BlendConstants[3]};
}

auto VulkanPipelineColorBlendStateDesc::SetBlendConstant(size_t idx, float blendConstant) noexcept -> BulletRT::Core::VulkanPipelineColorBlendStateDesc &
{
    if (idx < 4)
    {
        m_BlendConstants[idx] = blendConstant;
    }
    return *this;
}

auto VulkanPipelineColorBlendStateDesc::GetBlendConstant(size_t idx) const -> float
{
    return m_BlendConstants[idx];
}

VulkanSubpassDesc::VulkanSubpassDesc() noexcept
{
}

VulkanSubpassDesc::VulkanSubpassDesc(const BulletRT::Core::VulkanSubpassDesc &builder) noexcept
{
    m_Flags = builder.m_Flags;
    m_PipelineBindPoint = builder.m_PipelineBindPoint;
    m_InputAttachments = builder.m_InputAttachments;
    m_ColorAttachments = builder.m_ColorAttachments;
    m_ResolveAttachments = builder.m_ResolveAttachments;
    m_DepthStencilAttachment = builder.m_DepthStencilAttachment;
    m_PreserveAttachments = builder.m_PreserveAttachments;
}

VulkanSubpassDesc::VulkanSubpassDesc(BulletRT::Core::VulkanSubpassDesc &&builder) noexcept
{
    m_Flags = std::move(builder.m_Flags);
    m_PipelineBindPoint = std::move(builder.m_PipelineBindPoint);
    m_InputAttachments = std::move(builder.m_InputAttachments);
    m_ColorAttachments = std::move(builder.m_ColorAttachments);
    m_ResolveAttachments = std::move(builder.m_ResolveAttachments);
    m_DepthStencilAttachment = std::move(builder.m_DepthStencilAttachment);
    m_PreserveAttachments = std::move(builder.m_PreserveAttachments);
}

BulletRT::Core::VulkanSubpassDesc &VulkanSubpassDesc::operator=(const BulletRT::Core::VulkanSubpassDesc &builder) noexcept
{
    if (this != &builder)
    {

        m_Flags = builder.m_Flags;
        m_PipelineBindPoint = builder.m_PipelineBindPoint;
        m_InputAttachments = builder.m_InputAttachments;
        m_ColorAttachments = builder.m_ColorAttachments;
        m_ResolveAttachments = builder.m_ResolveAttachments;
        m_DepthStencilAttachment = builder.m_DepthStencilAttachment;
        m_PreserveAttachments = builder.m_PreserveAttachments;
    }
    return *this;
}

BulletRT::Core::VulkanSubpassDesc &VulkanSubpassDesc::operator=(BulletRT::Core::VulkanSubpassDesc &&builder) noexcept
{
    if (this != &builder)
    {
        m_Flags = std::move(builder.m_Flags);
        m_PipelineBindPoint = std::move(builder.m_PipelineBindPoint);
        m_InputAttachments = std::move(builder.m_InputAttachments);
        m_ColorAttachments = std::move(builder.m_ColorAttachments);
        m_ResolveAttachments = std::move(builder.m_ResolveAttachments);
        m_DepthStencilAttachment = std::move(builder.m_DepthStencilAttachment);
        m_PreserveAttachments = std::move(builder.m_PreserveAttachments);
    }
    return *this;
}

auto VulkanSubpassDesc::GetFlags() const noexcept -> vk::SubpassDescriptionFlags
{
    return m_Flags;
}

auto VulkanSubpassDesc::SetFlags(vk::SubpassDescriptionFlags flags) noexcept -> BulletRT::Core::VulkanSubpassDesc &
{
    m_Flags = flags;
    return *this;
}

auto VulkanSubpassDesc::SetPipelineBindPoint(vk::PipelineBindPoint pipelineBindPoint) noexcept -> BulletRT::Core::VulkanSubpassDesc &
{
    m_PipelineBindPoint = pipelineBindPoint;
    return *this;
}

auto VulkanSubpassDesc::GetPipelineBindPoint() const noexcept -> vk::PipelineBindPoint
{
    return m_PipelineBindPoint;
}

auto VulkanSubpassDesc::SetInputAttachmentCount(uint32_t inputAttachmentCount) noexcept -> BulletRT::Core::VulkanSubpassDesc &
{
    m_InputAttachments.resize(inputAttachmentCount);
    return *this;
}

auto VulkanSubpassDesc::GetInputAttachmentCount() const noexcept -> uint32_t
{
    return static_cast<uint32_t>(m_InputAttachments.size());
}

auto VulkanSubpassDesc::GetInputAttachments() const noexcept -> const std::vector<vk::AttachmentReference> &
{
    return m_InputAttachments;
}

auto VulkanSubpassDesc::SetInputAttachments(const std::vector<vk::AttachmentReference> &attachments) noexcept -> BulletRT::Core::VulkanSubpassDesc &
{
    m_InputAttachments = attachments;
    return *this;
}

auto VulkanSubpassDesc::AddInputAttachment(const vk::AttachmentReference &attachmentReference) noexcept -> BulletRT::Core::VulkanSubpassDesc &
{
    m_InputAttachments.push_back(attachmentReference);
    return *this;
}

auto VulkanSubpassDesc::SetInputAttachment(size_t idx, const vk::AttachmentReference &attachmentReference) noexcept -> BulletRT::Core::VulkanSubpassDesc &
{
    if (idx < m_InputAttachments.size())
    {
        m_InputAttachments[idx] = attachmentReference;
    }
    return *this;
}

auto VulkanSubpassDesc::GetInputAttachment(size_t idx) const noexcept -> std::optional<vk::AttachmentReference>
{
    return m_InputAttachments.size() > idx ? std::optional<vk::AttachmentReference>(m_InputAttachments[idx]) : std::nullopt;
}

auto VulkanSubpassDesc::SetColorAttachmentCount(uint32_t colorAttachmentCount) noexcept -> BulletRT::Core::VulkanSubpassDesc &
{
    m_ColorAttachments.resize(colorAttachmentCount);
    if (!m_ResolveAttachments.empty()){
        m_ResolveAttachments.resize(colorAttachmentCount);
    }
    return *this;
}

auto VulkanSubpassDesc::GetColorAttachmentCount() const noexcept -> uint32_t
{
    return static_cast<uint32_t>(m_ColorAttachments.size());
}

auto VulkanSubpassDesc::GetColorAttachments() const noexcept -> const std::vector<vk::AttachmentReference> &
{
    return m_ColorAttachments;
}

auto VulkanSubpassDesc::SetColorAttachments(const std::vector<vk::AttachmentReference> &attachments) noexcept -> BulletRT::Core::VulkanSubpassDesc &
{
    m_ColorAttachments = attachments;
    return *this;
}

auto VulkanSubpassDesc::SetResolveAttachments(const std::vector<vk::AttachmentReference> &attachments) noexcept -> BulletRT::Core::VulkanSubpassDesc &
{
    m_ResolveAttachments = attachments;
    m_ColorAttachments.resize(attachments.size());
    return *this;
}

auto VulkanSubpassDesc::GetResolveAttachments() const noexcept -> const std::vector<vk::AttachmentReference> &
{
    return m_ResolveAttachments;
}

auto VulkanSubpassDesc::SetResolveEnable(bool resolveEnable) noexcept -> BulletRT::Core::VulkanSubpassDesc &
{
    if (resolveEnable)
    {
        m_ResolveAttachments.resize(m_ColorAttachments.size());
    }
    else
    {
        m_ResolveAttachments = std::vector<vk::AttachmentReference>();
    }
    return *this;
}

auto VulkanSubpassDesc::GetResolveEnable() const noexcept -> bool
{
    return !m_ResolveAttachments.empty();
}

auto VulkanSubpassDesc::SetDepthStencilAttachment(const vk::AttachmentReference &attachment) noexcept -> BulletRT::Core::VulkanSubpassDesc &
{
    m_DepthStencilAttachment = attachment;
    return *this;
}

auto VulkanSubpassDesc::GetDepthStencilAttachment() const noexcept -> std::optional<vk::AttachmentReference>
{
    return m_DepthStencilAttachment;
}

auto VulkanSubpassDesc::SetPreserveAttachmentCount(uint32_t attachmentCount) noexcept -> BulletRT::Core::VulkanSubpassDesc &
{
    m_PreserveAttachments.resize(attachmentCount);
    return *this;
}

auto VulkanSubpassDesc::GetPreserveAttachmentCount() const noexcept -> uint32_t
{
    return static_cast<uint32_t>(m_PreserveAttachments.size());
}

auto VulkanSubpassDesc::SetPreserveAttachments(const std::vector<uint32_t> &preserveAttachments) noexcept -> BulletRT::Core::VulkanSubpassDesc &
{
    m_PreserveAttachments = preserveAttachments;
    return *this;
}

auto VulkanSubpassDesc::GetPreserveAttachments() const noexcept -> const std::vector<uint32_t> &
{
    return m_PreserveAttachments;
}

auto VulkanSubpassDesc::AddPreserveAttachment(uint32_t preserveAttachment) noexcept -> BulletRT::Core::VulkanSubpassDesc &
{
    m_PreserveAttachments.push_back(preserveAttachment);
    return *this;
}

auto VulkanSubpassDesc::SetPreserveAttachment(uint32_t idx, uint32_t preserveAttachment) noexcept -> BulletRT::Core::VulkanSubpassDesc &
{
    if (idx < m_PreserveAttachments.size())
    {
        m_PreserveAttachments[idx] = preserveAttachment;
    }
    return *this;
}

auto VulkanSubpassDesc::GetPreserveAttachment(uint32_t idx) const noexcept -> std::optional<uint32_t>
{
    return m_PreserveAttachments.at(idx);
}

auto VulkanSubpassDesc::GetSubpassDescriptionVk() const noexcept -> vk::SubpassDescription { 
    auto subpassDesc = vk::SubpassDescription()
        .setFlags(m_Flags)
        .setPipelineBindPoint(m_PipelineBindPoint)
        .setInputAttachments(m_InputAttachments)
        .setColorAttachments(m_ColorAttachments)
        .setPreserveAttachments(m_PreserveAttachments);
    if (!m_ResolveAttachments.empty())
    {
        subpassDesc.setResolveAttachments(m_ResolveAttachments);
    }
    if (m_DepthStencilAttachment){
        subpassDesc.setPDepthStencilAttachment(&m_DepthStencilAttachment.value());
    }
    return subpassDesc;
}

auto VulkanSubpassDesc::AddColorAttachment(const vk::AttachmentReference &reference) noexcept -> BulletRT::Core::VulkanSubpassDesc & { 
    if (!m_ResolveAttachments.empty()){
        m_ResolveAttachments.resize(m_ColorAttachments.size()+1);
    }
    m_ColorAttachments.push_back(reference);
    return *this;
}

auto VulkanSubpassDesc::SetColorAttachment(size_t idx, const vk::AttachmentReference &reference) noexcept -> BulletRT::Core::VulkanSubpassDesc & { 
    if (m_ColorAttachments.size() > idx){
        m_ColorAttachments[idx] = reference;
    }
    return *this;
}

auto VulkanSubpassDesc::GetColorAttachment(size_t idx) const noexcept -> std::optional<vk::AttachmentReference> { 
    if (m_ColorAttachments.size() > idx){
        return m_ColorAttachments[idx];
    }
    return std::nullopt;
}



VulkanRenderPassBuilder::VulkanRenderPassBuilder() noexcept
{
}

VulkanRenderPassBuilder::VulkanRenderPassBuilder(const BulletRT::Core::VulkanRenderPassBuilder &builder) noexcept
{
    m_Flags = builder.m_Flags;
    m_Attachments = builder.m_Attachments;
    m_Subpasses = builder.m_Subpasses;
    m_Dependencies = builder.m_Dependencies;
}

VulkanRenderPassBuilder::VulkanRenderPassBuilder(BulletRT::Core::VulkanRenderPassBuilder &&builder) noexcept
{
    m_Flags = std::move(builder.m_Flags);
    m_Attachments = std::move(builder.m_Attachments);
    m_Subpasses = std::move(builder.m_Subpasses);
    m_Dependencies = std::move(builder.m_Dependencies);
}

BulletRT::Core::VulkanRenderPassBuilder &VulkanRenderPassBuilder::operator=(const BulletRT::Core::VulkanRenderPassBuilder &builder) noexcept
{
    if (this != &builder)
    {
        m_Flags = builder.m_Flags;
        m_Attachments = builder.m_Attachments;
        m_Subpasses = builder.m_Subpasses;
        m_Dependencies = builder.m_Dependencies;
    }
    return *this;
}

BulletRT::Core::VulkanRenderPassBuilder &VulkanRenderPassBuilder::operator=(BulletRT::Core::VulkanRenderPassBuilder &&builder) noexcept
{
    if (this != &builder)
    {
        m_Flags = std::move(builder.m_Flags);
        m_Attachments = std::move(builder.m_Attachments);
        m_Subpasses = std::move(builder.m_Subpasses);
        m_Dependencies = std::move(builder.m_Dependencies);
    }
    return *this;
}

auto VulkanRenderPassBuilder::GetSubpassDescriptionVks() const noexcept -> std::vector<vk::SubpassDescription> { 
    std::vector<vk::SubpassDescription> subpassDescs;
    subpassDescs.reserve(m_Subpasses.size());
    for (auto& subpass:m_Subpasses){
        subpassDescs.push_back(subpass.GetSubpassDescriptionVk());
    }
    return subpassDescs;
}

auto VulkanRenderPassBuilder::GetSubpasses() const noexcept -> const std::vector<VulkanSubpassDesc> & { 
    return m_Subpasses;
}

auto VulkanRenderPassBuilder::SetDependencies(const std::vector<vk::SubpassDependency> &dependencies) noexcept -> BulletRT::Core::VulkanRenderPassBuilder & { 
    m_Dependencies = dependencies; return *this;
}

auto VulkanRenderPassBuilder::GetDependencies() const noexcept -> const std::vector<vk::SubpassDependency> & { 
    return m_Dependencies;
}

auto VulkanRenderPassBuilder::GetFlags() const noexcept -> vk::RenderPassCreateFlags { 
    return m_Flags;
}

auto VulkanRenderPassBuilder::GetAttachments() const noexcept -> const std::vector<vk::AttachmentDescription> & { 
    return m_Attachments;
}

auto VulkanRenderPassBuilder::SetFlags(vk::RenderPassCreateFlags flags) noexcept -> BulletRT::Core::VulkanRenderPassBuilder & { 
    m_Flags = flags ; return *this;
}

auto VulkanRenderPassBuilder::SetAttachmentCount(uint32_t attachmentCount) noexcept -> BulletRT::Core::VulkanRenderPassBuilder & { 
    m_Attachments.resize(attachmentCount); return *this;
}

auto VulkanRenderPassBuilder::GetAttachmentCount() const noexcept -> uint32_t { 
    return static_cast<uint32_t>(m_Attachments.size());
}

auto VulkanRenderPassBuilder::SetAttachments(const std::vector<vk::AttachmentDescription> &attachments) noexcept -> BulletRT::Core::VulkanRenderPassBuilder & { 
    m_Attachments = attachments; return *this;
}

auto VulkanRenderPassBuilder::AddAttachment(const vk::AttachmentDescription &attachment) noexcept -> BulletRT::Core::VulkanRenderPassBuilder & { 
    m_Attachments.push_back(attachment); return *this;
}

auto VulkanRenderPassBuilder::SetAttachment(size_t idx, const vk::AttachmentDescription &attachment) noexcept -> BulletRT::Core::VulkanRenderPassBuilder & { 
    if (idx < m_Attachments.size()){
        m_Attachments[idx] = attachment;
    }
    return *this;
}

auto VulkanRenderPassBuilder::GetAttachment(size_t idx) const -> std::optional<vk::AttachmentDescription> {

    if (idx < m_Attachments.size()){
        return m_Attachments[idx];
    }
    return std::nullopt;
}

auto VulkanRenderPassBuilder::SetSubpassCount(uint32_t subpassCount) noexcept -> BulletRT::Core::VulkanRenderPassBuilder & { 
    m_Subpasses.resize(subpassCount); return *this;
}

auto VulkanRenderPassBuilder::GetSubpassCount() const noexcept -> uint32_t { 
    return static_cast<uint32_t>(m_Subpasses.size());
}

auto VulkanRenderPassBuilder::SetSubpasses(const std::vector<VulkanSubpassDesc> &subpasses) noexcept -> BulletRT::Core::VulkanRenderPassBuilder & { 
    m_Subpasses =subpasses; return *this;
}

auto VulkanRenderPassBuilder::AddSubpass(const BulletRT::Core::VulkanSubpassDesc &subpass) noexcept -> BulletRT::Core::VulkanRenderPassBuilder & { 
    m_Subpasses.push_back(subpass); return *this;
}

auto VulkanRenderPassBuilder::SetSubpass(size_t idx, const BulletRT::Core::VulkanSubpassDesc &subpass) noexcept -> BulletRT::Core::VulkanRenderPassBuilder & { 
    if (idx < m_Subpasses.size()){
        m_Subpasses[idx] = subpass;
    }
    return *this;
}

auto VulkanRenderPassBuilder::GetSubpass(size_t idx) const noexcept -> std::optional<VulkanSubpassDesc> { 
    if (idx < m_Subpasses.size()){
        return m_Subpasses[idx];
    }else{
        return std::nullopt;
    }
}

auto VulkanRenderPassBuilder::SetDependencyCount(uint32_t dependencyCount) noexcept -> BulletRT::Core::VulkanRenderPassBuilder & { 
    m_Dependencies.resize(dependencyCount); return *this;
}

auto VulkanRenderPassBuilder::GetDependencyCount() const noexcept -> uint32_t { 
    return static_cast<uint32_t>(m_Dependencies.size());
}

auto VulkanRenderPassBuilder::AddDependency(const vk::SubpassDependency &dependency) noexcept -> BulletRT::Core::VulkanRenderPassBuilder & { 
    m_Dependencies.push_back(dependency); return *this;
}

auto VulkanRenderPassBuilder::SetDependency(size_t idx, const vk::SubpassDependency &dependency) noexcept -> BulletRT::Core::VulkanRenderPassBuilder & { 
    if (idx < m_Dependencies.size()){
        m_Dependencies[idx] = dependency;
    }
    return *this;
}

auto VulkanRenderPassBuilder::GetDependency(size_t idx) const noexcept -> std::optional<vk::SubpassDependency> { 
    if (idx < m_Dependencies.size()){
        return m_Dependencies[idx];
    }else{
        return std::nullopt;
    }
}

auto VulkanRenderPassBuilder::GetSrcSubpass(size_t subpassDependIdx) const -> std::optional<VulkanSubpassDesc> { 
    if (m_Dependencies.size() > subpassDependIdx){
        if (m_Subpasses.size() > m_Dependencies[subpassDependIdx].srcSubpass){
            return m_Subpasses[m_Dependencies[subpassDependIdx].srcSubpass];
        }
    }
    return std::nullopt;

}

auto VulkanRenderPassBuilder::GetDstSubpass(size_t subpassDependIdx) const -> std::optional<VulkanSubpassDesc> {
    if (m_Dependencies.size() > subpassDependIdx){
        if (m_Subpasses.size() > m_Dependencies[subpassDependIdx].dstSubpass){
            return m_Subpasses[m_Dependencies[subpassDependIdx].dstSubpass];
        }
    }
    return std::nullopt;

}

auto VulkanRenderPassBuilder::GetInputAttachments(size_t subpassIdx) const noexcept -> std::vector<vk::AttachmentDescription> {
    if (m_Subpasses.size() > subpassIdx){
        auto& inputAttachments = m_Subpasses[subpassIdx].GetInputAttachments();
        auto attachs = std::vector<vk::AttachmentDescription>();
        attachs.reserve(inputAttachments.size());
        for (auto& attachment:inputAttachments){
            attachs.push_back(m_Attachments[attachment.attachment]);
        }
        return attachs;
    }
    return std::vector<vk::AttachmentDescription>();
}

auto VulkanRenderPassBuilder::GetColorAttachments(size_t subpassIdx) const noexcept -> std::vector<vk::AttachmentDescription> {
    if (m_Subpasses.size() > subpassIdx){
        auto& colorAttachments = m_Subpasses[subpassIdx].GetColorAttachments();
        auto attachs = std::vector<vk::AttachmentDescription>();
        attachs.reserve(colorAttachments.size());
        for (auto& attachment:colorAttachments){
            attachs.push_back(m_Attachments[attachment.attachment]);
        }
        return attachs;
    }
    return std::vector<vk::AttachmentDescription>();
}

auto VulkanRenderPassBuilder::GetResolveAttachments(size_t subpassIdx) const noexcept -> std::vector<vk::AttachmentDescription> {
    if (m_Subpasses.size() > subpassIdx){
        auto& resolveAttachments = m_Subpasses[subpassIdx].GetResolveAttachments();
        auto attachs = std::vector<vk::AttachmentDescription>();
        attachs.reserve(resolveAttachments.size());
        for (auto& attachment:resolveAttachments){
            attachs.push_back(m_Attachments[attachment.attachment]);
        }
        return attachs;
    }
    return std::vector<vk::AttachmentDescription>();
}

auto VulkanRenderPassBuilder::GetDepthStencilAttachment(size_t subpassIdx) const noexcept -> std::optional<vk::AttachmentDescription> {
    if (m_Subpasses.size() > subpassIdx){
        auto depthStencilAttachment = m_Subpasses[subpassIdx].GetDepthStencilAttachment();
        if (depthStencilAttachment){
            return m_Attachments[depthStencilAttachment->attachment];
        }
    }
    return std::optional<vk::AttachmentDescription>();
}

auto VulkanRenderPassBuilder::GetPreserveAttachments(size_t subpassIdx) const noexcept -> std::vector<vk::AttachmentDescription> {
    if (m_Subpasses.size() > subpassIdx){
        auto preserveAttachments = m_Subpasses[subpassIdx].GetPreserveAttachments();
        auto attachs = std::vector<vk::AttachmentDescription>();
        attachs.reserve(preserveAttachments.size());
        for (auto& attachment:preserveAttachments){
            attachs.push_back(m_Attachments[attachment]);
        }
        return attachs;
    }
    return std::vector<vk::AttachmentDescription>();
}

auto VulkanRenderPassBuilder::Build(const BulletRT::Core::VulkanDevice *device) const -> std::unique_ptr<BulletRT::Core::VulkanRenderPass> { 
    return VulkanRenderPass::New(device, *this);
}








auto VulkanRenderPass::New(const BulletRT::Core::VulkanDevice *device, const BulletRT::Core::VulkanRenderPassBuilder &builder) -> std::unique_ptr<VulkanRenderPass> { 
    auto subpassDescriptionVks  = builder.GetSubpassDescriptionVks();
    auto renderPassCreateInfoVk = vk::RenderPassCreateInfo()
        .setFlags(builder.GetFlags())
        .setAttachments(builder.GetAttachments())
        .setSubpasses(subpassDescriptionVks)
        .setDependencies(builder.GetDependencies());
    auto renderPass = device->GetDeviceVk().createRenderPassUnique(
        renderPassCreateInfoVk
    );
    if (renderPass){
        auto vulkanRenderPass = std::unique_ptr<VulkanRenderPass>(new VulkanRenderPass());
        vulkanRenderPass->m_Flags = builder.GetFlags();
        vulkanRenderPass->m_Device= device;
        vulkanRenderPass->m_RenderPass = std::move(renderPass);
        vulkanRenderPass->m_Subpasses = builder.GetSubpasses();
        vulkanRenderPass->m_Dependencies = builder.GetDependencies();
        vulkanRenderPass->m_Attachments = builder.GetAttachments();
        return vulkanRenderPass;
    }
    return nullptr;
}

VulkanRenderPass::~VulkanRenderPass() noexcept { 
    m_RenderPass.reset();
}

auto VulkanRenderPass::GetDevice() const noexcept -> const BulletRT::Core::VulkanDevice * { 
    return m_Device;
}

auto VulkanRenderPass::GetDeviceVk() const noexcept -> vk::Device { 
    return m_Device ? m_Device->GetDeviceVk() : nullptr;
}

auto VulkanRenderPass::GetRenderPassVk() const noexcept -> vk::RenderPass { 
    return m_RenderPass.get();
}

auto VulkanRenderPass::GetFlags() const noexcept -> vk::RenderPassCreateFlags { 
    return m_Flags;
}

auto VulkanRenderPass::GetAttachmentCount() const noexcept -> uint32_t { 
    return static_cast<uint32_t>(m_Attachments.size());
}

auto VulkanRenderPass::GetAttachments() const noexcept -> const std::vector<vk::AttachmentDescription> & {
    return m_Attachments;
}

auto VulkanRenderPass::GetAttachment(size_t idx) const -> std::optional<vk::AttachmentDescription> { 
    return m_Attachments.size() > idx ? std::optional<vk::AttachmentDescription>(m_Attachments[idx]):std::nullopt;
}

auto VulkanRenderPass::GetSubpassCount() const noexcept -> uint32_t { 
    return static_cast<uint32_t>(m_Subpasses.size());
}

auto VulkanRenderPass::GetSubpasses() const noexcept -> const std::vector<VulkanSubpassDesc> & { 
    return m_Subpasses;
}

auto VulkanRenderPass::GetSubpass(size_t idx) const noexcept -> std::optional<VulkanSubpassDesc> { 
    return m_Subpasses.size() > idx ? std::optional<VulkanSubpassDesc>(m_Subpasses[idx]):std::nullopt;
}

auto VulkanRenderPass::GetDependencyCount() const noexcept -> uint32_t { 
    return static_cast<uint32_t>(m_Subpasses.size());
}

auto VulkanRenderPass::GetDependencies() const noexcept -> const std::vector<vk::SubpassDependency> & { 
    return m_Dependencies;
}

auto VulkanRenderPass::GetDependency(size_t idx) const noexcept -> std::optional<vk::SubpassDependency> { 
    return m_Dependencies.size() > idx ? std::optional<vk::SubpassDependency>(m_Dependencies.at(idx)):std::nullopt;
}

auto VulkanRenderPass::GetSrcSubpass(size_t subpassDependIdx) const -> std::optional<VulkanSubpassDesc> { 
    if (m_Dependencies.size() > subpassDependIdx){
        if (m_Subpasses.size() > m_Dependencies[subpassDependIdx].srcSubpass){
            return m_Subpasses[m_Dependencies[subpassDependIdx].srcSubpass];
        }
    }
    return std::nullopt;
}

auto VulkanRenderPass::GetDstSubpass(size_t subpassDependIdx) const -> std::optional<VulkanSubpassDesc> { 
    if (m_Dependencies.size() > subpassDependIdx){
        if (m_Subpasses.size() > m_Dependencies[subpassDependIdx].dstSubpass){
            return m_Subpasses[m_Dependencies[subpassDependIdx].dstSubpass];
        }
    }
    return std::nullopt;

}

auto VulkanRenderPass::GetInputAttachments(size_t subpassIdx) const noexcept -> std::vector<vk::AttachmentDescription> { 
    if (m_Subpasses.size() > subpassIdx){
        auto& inputAttachments = m_Subpasses[subpassIdx].GetInputAttachments();
        auto attachs = std::vector<vk::AttachmentDescription>();
        attachs.reserve(inputAttachments.size());
        for (auto& attachment:inputAttachments){
            attachs.push_back(m_Attachments[attachment.attachment]);
        }
        return attachs;
    }
    return std::vector<vk::AttachmentDescription>();
}

auto VulkanRenderPass::GetColorAttachments(size_t subpassIdx) const noexcept -> std::vector<vk::AttachmentDescription> { 
    if (m_Subpasses.size() > subpassIdx){
        auto& colorAttachments = m_Subpasses[subpassIdx].GetColorAttachments();
        auto attachs = std::vector<vk::AttachmentDescription>();
        attachs.reserve(colorAttachments.size());
        for (auto& attachment:colorAttachments){
            attachs.push_back(m_Attachments[attachment.attachment]);
        }
        return attachs;
    }
    return std::vector<vk::AttachmentDescription>();
}

auto VulkanRenderPass::GetResolveAttachments(size_t subpassIdx) const noexcept -> std::vector<vk::AttachmentDescription> { 
    if (m_Subpasses.size() > subpassIdx){
        auto& resolveAttachments = m_Subpasses[subpassIdx].GetResolveAttachments();
        auto attachs = std::vector<vk::AttachmentDescription>();
        attachs.reserve(resolveAttachments.size());
        for (auto& attachment:resolveAttachments){
            attachs.push_back(m_Attachments[attachment.attachment]);
        }
        return attachs;
    }
    return std::vector<vk::AttachmentDescription>();
}

auto VulkanRenderPass::GetDepthStencilAttachment(size_t subpassIdx) const noexcept -> std::optional<vk::AttachmentDescription> { 
    if (m_Subpasses.size() > subpassIdx){
        auto depthStencilAttachment = m_Subpasses[subpassIdx].GetDepthStencilAttachment();
        if (depthStencilAttachment){
            return m_Attachments[depthStencilAttachment->attachment];
        }
    }
    return std::optional<vk::AttachmentDescription>();
}

auto VulkanRenderPass::GetPreserveAttachments(size_t subpassIdx) const noexcept -> std::vector<vk::AttachmentDescription> { 
    if (m_Subpasses.size() > subpassIdx){
        auto preserveAttachments = m_Subpasses[subpassIdx].GetPreserveAttachments();
        auto attachs = std::vector<vk::AttachmentDescription>();
        attachs.reserve(preserveAttachments.size());
        for (auto& attachment:preserveAttachments){
            attachs.push_back(m_Attachments[attachment]);
        }
        return attachs;
    }
    return std::vector<vk::AttachmentDescription>();
}


VulkanRenderPass::VulkanRenderPass() noexcept { 
    
}
