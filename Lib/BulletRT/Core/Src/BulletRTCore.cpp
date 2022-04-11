#include <BulletRT/Core/BulletRTCore.h>
#include <iostream>
#include <vector>
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE
BulletRT::Core::VulkanDeviceFeaturesSet::VulkanDeviceFeaturesSet(const VulkanDeviceFeaturesSet& featureSet) noexcept
{
    if (!featureSet.m_Holders.empty()) {
        m_IndexMap = featureSet.m_IndexMap;
        m_Holders.reserve(featureSet.m_Holders.size());
        m_TailPNext = nullptr;
        for (auto& holder : featureSet.m_Holders) {
            m_Holders.push_back(std::unique_ptr<IVulkanDeviceExtFeatureHolder>(holder->Clone()));
        }
        if (m_Holders.size() > 1) {
            for (size_t i = 0; i < m_Holders.size() - 1; ++i) {
                m_Holders[i]->Link(m_Holders[i + 1]->GetPointer());
            }
        }
    }
}

BulletRT::Core::VulkanDeviceFeaturesSet::VulkanDeviceFeaturesSet(VulkanDeviceFeaturesSet&& featureSet) noexcept
    :m_Holders{std::move(featureSet.m_Holders)}, m_IndexMap{ std::move(featureSet.m_IndexMap) },m_TailPNext{featureSet.m_TailPNext}
{
    featureSet.m_TailPNext = nullptr;
}

BulletRT::Core::VulkanDeviceFeaturesSet& BulletRT::Core::VulkanDeviceFeaturesSet::operator=(const VulkanDeviceFeaturesSet& featureSet) noexcept
{
    // TODO: return ステートメントをここに挿入します
    if (this != &featureSet) {

        if (!featureSet.m_Holders.empty()) {
            m_IndexMap = featureSet.m_IndexMap;
            m_Holders.clear();
            m_Holders.reserve(featureSet.m_Holders.size());
            m_TailPNext = nullptr;
            for (auto& holder : featureSet.m_Holders) {
                m_Holders.push_back(std::unique_ptr<IVulkanDeviceExtFeatureHolder>(holder->Clone()));
            }
            if (m_Holders.size() > 1) {
                for (size_t i = 0; i < m_Holders.size() - 1; ++i) {
                    m_Holders[i]->Link(m_Holders[i + 1]->GetPointer());
                }
            }
        }
    }
    return *this;
}

BulletRT::Core::VulkanDeviceFeaturesSet& BulletRT::Core::VulkanDeviceFeaturesSet::operator=(VulkanDeviceFeaturesSet&& featureSet) noexcept
{
    // TODO: return ステートメントをここに挿入します
    if (this != &featureSet) {
        m_IndexMap = std::move(featureSet.m_IndexMap);
        m_Holders  = std::move(featureSet.m_Holders );
        m_TailPNext = featureSet.m_TailPNext;
        featureSet.m_TailPNext = nullptr;
    }
    return *this;
}

bool BulletRT::Core::VulkanDeviceFeaturesSet::Impl_Contain(vk::StructureType sType) const noexcept
{
    return m_IndexMap.count(sType) > 0;
}

auto BulletRT::Core::VulkanDeviceFeaturesSet::Impl_Read(vk::StructureType sType) const noexcept -> const void*
{
    return Impl_Contain(sType) ? m_Holders.at(m_IndexMap.at(sType))->Read() : nullptr;
}

bool BulletRT::Core::VulkanDeviceFeaturesSet::Impl_Write(vk::StructureType sType, const void* pData) noexcept
{
    if (Impl_Contain(sType)) {
        m_Holders.at(m_IndexMap.at(sType))->Write(pData);
        return true;
    }
    return false;
}

bool BulletRT::Core::VulkanDeviceFeaturesSet::Impl_Map(vk::StructureType sType, void** ppData) noexcept
{
    if (Impl_Contain(sType)) {
        auto ptr = m_Holders.at(m_IndexMap.at(sType)).get();
        ptr->Map();
        if (!ppData) {
            return false;
        }
        *ppData = ptr->GetPointer();
        return true;
    }
    return false;
}

bool BulletRT::Core::VulkanDeviceFeaturesSet::Impl_Unmap(vk::StructureType sType) noexcept
{
    if (Impl_Contain(sType)) {
        auto ptr = m_Holders.at(m_IndexMap.at(sType)).get();
        ptr->Unmap();
        return true;
    }
    return false;
}

auto BulletRT::Core::VulkanContext::GetHandle() noexcept -> VulkanContext&
{
    static VulkanContext context;
    return context;
}

void BulletRT::Core::VulkanContext::Initialize() noexcept
{
    auto& handle = GetHandle();
    if (!handle.m_DLL) {
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

auto BulletRT::Core::VulkanInstance::New(const VulkanInstanceBuilder& builder) noexcept -> std::unique_ptr<VulkanInstance>
{
    auto applicationName = builder.GetApplicationName();
    auto engineName      = builder.GetEngineName();
    auto applicationInfo = vk::ApplicationInfo()
        .setApiVersion(builder.GetApiVersion())
        .setApplicationVersion(builder.GetApplicationVersion())
        .setEngineVersion(builder.GetEngineVersion())
        .setPApplicationName(applicationName.c_str())
        .setPEngineName(engineName.c_str());

    auto enabledExtSet         = builder.GetExtensionSet();
    auto enabledExtNamesString = std::vector<std::string>(std::begin(enabledExtSet), std::end(enabledExtSet));
    auto enabledExtNames       = std::vector<const char*>();
    enabledExtNames.reserve(enabledExtSet.size());
    for (auto& enabledExtNameString : enabledExtNamesString) {
        enabledExtNames.push_back(enabledExtNameString.c_str());
    }

    auto enabledLyrSet         = builder.GetLayerSet();
    auto enabledLyrNamesString = std::vector<std::string>(std::begin(enabledLyrSet), std::end(enabledLyrSet));
    auto enabledLyrNames       = std::vector<const char*>();
    enabledLyrNames.reserve(enabledLyrSet.size());
    for (auto& enabledLyrNameString : enabledLyrNamesString) {
        enabledLyrNames.push_back(enabledLyrNameString.c_str());
    }

    auto debugUtilsMessengerCreateInfos = builder.GetDebugUtilsMessengers();
    auto debugReportCallbackCreateInfo  = builder.GetDebugReportCallback();
    auto instanceCreateInfo = vk::InstanceCreateInfo()
        .setPApplicationInfo(&applicationInfo)
        .setPEnabledExtensionNames(enabledExtNames)
        .setPEnabledLayerNames(enabledLyrNames);

    const void* pHead = nullptr;
    if (enabledExtSet.count(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)  > 0) {
        if (!debugUtilsMessengerCreateInfos.empty()) {
            if (debugUtilsMessengerCreateInfos.size() > 1) {
                for (size_t i = 0; i < debugUtilsMessengerCreateInfos.size() - 1; ++i)
                {
                    debugUtilsMessengerCreateInfos[i].pNext = &debugUtilsMessengerCreateInfos[i + 1];
                }
            }
            pHead = &debugUtilsMessengerCreateInfos.front();
        }
    }
    else {
        debugUtilsMessengerCreateInfos.clear();
    }
    if (enabledExtSet.count(VK_EXT_DEBUG_REPORT_EXTENSION_NAME) > 0) {
        if (debugReportCallbackCreateInfo) {
            debugReportCallbackCreateInfo.value().pNext = pHead;
            pHead = &debugReportCallbackCreateInfo.value();
        }
    }
    else {
        debugReportCallbackCreateInfo = std::nullopt;
    }

    instanceCreateInfo.pNext = pHead;

    auto instance             = vk::createInstanceUnique(instanceCreateInfo);

    if (instance) {
        VULKAN_HPP_DEFAULT_DISPATCHER.init(*instance);

        auto debugUtilsMessengers = std::vector<vk::UniqueDebugUtilsMessengerEXT>();
        auto debugReportCallback  = vk::UniqueDebugReportCallbackEXT();

        if (!debugUtilsMessengerCreateInfos.empty()) {
            for (auto& debugUtilsMessengerCreateInfo : debugUtilsMessengerCreateInfos)
            {
                debugUtilsMessengerCreateInfo.pNext = nullptr;
                debugUtilsMessengers.push_back(instance->createDebugUtilsMessengerEXTUnique(debugUtilsMessengerCreateInfo));
            }
        }
        if (debugReportCallbackCreateInfo) {
            debugReportCallbackCreateInfo.value().pNext = nullptr;
            debugReportCallback = instance->createDebugReportCallbackEXTUnique(debugReportCallbackCreateInfo.value());
        }

        auto vulkanInstance = new VulkanInstance();
        vulkanInstance->m_Instance             = std::move(instance);
        vulkanInstance->m_DebugUtilsMessengers = std::move(debugUtilsMessengers);
        vulkanInstance->m_DebugReportCallback  = std::move(debugReportCallback);
        vulkanInstance->m_ApiVersion           = applicationInfo.apiVersion;
        vulkanInstance->m_EngineVersion        = applicationInfo.engineVersion;
        vulkanInstance->m_ApplicationVersion   = applicationInfo.applicationVersion;
        vulkanInstance->m_ApplicationName      = applicationName;
        vulkanInstance->m_EngineName           = engineName;
        vulkanInstance->m_DebugUtilsMessengerCreateInfos = std::move(debugUtilsMessengerCreateInfos);
        vulkanInstance->m_DebugReportCallbackCreateInfo  = std::move(debugReportCallbackCreateInfo);
        vulkanInstance->m_EnabledExtNameSet    = enabledExtSet;
        vulkanInstance->m_EnabledLyrNameSet    = enabledLyrSet;
        return std::unique_ptr<VulkanInstance>(vulkanInstance);
    }

    return nullptr;
}

BulletRT::Core::VulkanInstance::~VulkanInstance() noexcept {
    m_DebugUtilsMessengers.clear();
    m_DebugReportCallback.reset();
    m_Instance.reset();
}

BulletRT::Core::VulkanInstance::VulkanInstance() noexcept :m_Instance{}, m_DebugUtilsMessengers{}, m_DebugReportCallback{}
{
    m_ApiVersion = 0;
    m_ApplicationVersion = 0;
    m_EngineVersion = 0;
    m_ApplicationName = "";
    m_EngineName = "";
    m_EnabledExtNameSet = {};
    m_EnabledLyrNameSet = {};
    m_DebugReportCallbackCreateInfo  = std::nullopt;
    m_DebugUtilsMessengerCreateInfos = {};
}

auto BulletRT::Core::VulkanInstanceBuilder::Build() const -> std::unique_ptr<VulkanInstance>
{
    return VulkanInstance::New(*this);
}

auto BulletRT::Core::VulkanDevice::New(const BulletRT::Core::VulkanDeviceBuilder& builder) noexcept -> std::unique_ptr<BulletRT::Core::VulkanDevice>
{
    auto physicalDevice        = builder.GetPhysicalDevice();
    auto enabledExtSet         = builder.GetExtensionSet();
    auto enabledExtNamesString = std::vector<std::string>(std::begin(enabledExtSet), std::end(enabledExtSet));
    auto enabledExtNames       = std::vector<const char*>();
    auto enabledFeatureSet     = builder.GetFeaturesSet();
    auto queueFamilySet        = builder.GetQueueFamilySet();
    auto deviceQueueCreateInfos= std::vector<vk::DeviceQueueCreateInfo>();

    enabledExtNames.reserve(enabledExtSet.size());
    for (auto& enabledExtNameString : enabledExtNamesString) {
        enabledExtNames.push_back(enabledExtNameString.c_str());
    }
    for (auto& [queueFamilyIndex, queueFamily] : queueFamilySet) {
        deviceQueueCreateInfos.push_back(
            vk::DeviceQueueCreateInfo()
            .setQueueFamilyIndex(queueFamily.GetQueueFamilyIndex())
            .setQueuePriorities( queueFamily.GetQueueProperties())
        );
    }

    auto deviceCreateInfo = vk::DeviceCreateInfo()
        .setQueueCreateInfos(deviceQueueCreateInfos)
        .setPEnabledExtensionNames(enabledExtNames)
        .setPNext(enabledFeatureSet.ReadHead());

    auto device = physicalDevice.createDeviceUnique(deviceCreateInfo);
    if (device) {
        VULKAN_HPP_DEFAULT_DISPATCHER.init(*device);
        auto vulkanDevice = new BulletRT::Core::VulkanDevice();
        vulkanDevice->m_LogigalDevice      = std::move(device);
        vulkanDevice->m_PhysicalDevice     = physicalDevice;
        vulkanDevice->m_EnabledExtNameSet  = enabledExtSet;
        vulkanDevice->m_EnabledFeaturesSet = enabledFeatureSet;
        vulkanDevice->m_QueueFamilyMap     = queueFamilySet;
        vulkanDevice->m_Instance           = builder.GetInstance();
        return std::unique_ptr<BulletRT::Core::VulkanDevice>(vulkanDevice);
    }
    return nullptr;
}

BulletRT::Core::VulkanDevice::~VulkanDevice() noexcept
{
    m_LogigalDevice.reset();
}

auto BulletRT::Core::VulkanDevice::WaitForFences(const std::vector<const VulkanFence*>& fences, uint64_t timeOut, VkBool32 waitForAll) -> vk::Result
{
    auto fencesVk = std::vector<vk::Fence>();
    fencesVk.reserve(fences.size());
    for (auto& fence:fences){
        if (fence){
            fencesVk.push_back(fence->GetFenceVk());
        }
    }
    
    return m_LogigalDevice->waitForFences(fencesVk.size(),fencesVk.data(),waitForAll,timeOut);
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

BulletRT::Core::VulkanDevice::VulkanDevice() noexcept:m_PhysicalDevice(), m_LogigalDevice()
{
    m_EnabledExtNameSet  = {};
    m_EnabledFeaturesSet = {};
    m_QueueFamilyMap     = {};
    m_Instance           = nullptr;
}

auto BulletRT::Core::VulkanDeviceBuilder::Build() const -> std::unique_ptr<BulletRT::Core::VulkanDevice>
{
    return BulletRT::Core::VulkanDevice::New(*this);
}

auto BulletRT::Core::VulkanBuffer::New(const VulkanDevice* device, const VulkanBufferBuilder& builder) -> std::unique_ptr<VulkanBuffer>
{
    auto queueFamilyIndices = builder.GetQueueFamilyIndices();
    auto bufferCreateInfo = vk::BufferCreateInfo()
        .setSize(builder.GetSize())
        .setFlags(builder.GetFlags())
        .setUsage(builder.GetUsage())
        .setQueueFamilyIndices(queueFamilyIndices)
        .setSharingMode(builder.GetSharingMode());
    auto buffer = device->GetDeviceVk().createBufferUnique(bufferCreateInfo);
    if (buffer) {
        auto vulkanBuffer = new VulkanBuffer();
        vulkanBuffer->m_Device = device;
        vulkanBuffer->m_Buffer = std::move(buffer);
        vulkanBuffer->m_Size   = builder.GetSize();
        vulkanBuffer->m_Usage  = builder.GetUsage();
        vulkanBuffer->m_Flags  = builder.GetFlags();
        vulkanBuffer->m_QueueFamilyIndices = queueFamilyIndices;
        return std::unique_ptr<VulkanBuffer>(vulkanBuffer);
    }
    return nullptr;
}

auto BulletRT::Core::VulkanBufferBuilder::Build(const VulkanDevice* device) const noexcept -> std::unique_ptr<VulkanBuffer>
{
    return BulletRT::Core::VulkanBuffer::New(device,*this);
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

auto BulletRT::Core::VulkanBuffer::QueryMemoryRequirements()const->vk::MemoryRequirements {
    return m_Device->GetDeviceVk().getBufferMemoryRequirements(m_Buffer.get());
}

auto BulletRT::Core::VulkanImage::New(const VulkanDevice* device, const VulkanImageBuilder& builder) -> std::unique_ptr<VulkanImage>
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
    if (image) {
        auto vulkanImage = new VulkanImage();
        vulkanImage->m_Device = device;
        vulkanImage->m_Image  = std::move(image);
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

auto BulletRT::Core::VulkanImageBuilder::Build(const VulkanDevice* device) const noexcept -> std::unique_ptr<VulkanImage>
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

auto BulletRT::Core::VulkanDeviceMemory::New(const VulkanDevice* device, const VulkanDeviceMemoryBuilder& builder) -> std::unique_ptr<VulkanDeviceMemory>
{
    auto memoryAllocateFlagsInfo      = builder.GetMemoryAllocateFlagsInfo();
    auto memoryDedicatedAllocateInfo  = builder.GetMemoryDedicatedAllocateInfo();
    auto memoryAllocateInfo           = vk::MemoryAllocateInfo()
        .setAllocationSize(builder.GetAllocationSize())
        .setMemoryTypeIndex(builder.GetMemoryTypeIndex());

    auto physicalDeviceProperties     = device->GetPhysicalDeviceVk().getProperties();

    bool enableDedicatedAllocationKhr = false;
    bool enableDeviceGroupKhr         = false;
    if ((physicalDeviceProperties.apiVersion >= VK_API_VERSION_1_1) ||
         device->SupportExtension(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME)) {
        enableDedicatedAllocationKhr  = true;
    }
    if ((physicalDeviceProperties.apiVersion >= VK_API_VERSION_1_1) ||
        device->SupportExtension(VK_KHR_DEVICE_GROUP_EXTENSION_NAME)) {
        enableDeviceGroupKhr          = true;
    }

    const void* pHead = nullptr;
    if (enableDedicatedAllocationKhr) {
        if (memoryDedicatedAllocateInfo) {
            memoryDedicatedAllocateInfo.value().pNext = pHead;
            pHead = &memoryDedicatedAllocateInfo.value();
        }
    }
    else {
        memoryDedicatedAllocateInfo = std::nullopt;
    }
    if (enableDeviceGroupKhr) {
        if (memoryAllocateFlagsInfo) {
            if (!device->SupportExtension(VK_EXT_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME)&&
                !device->SupportExtension(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME)&&
                 physicalDeviceProperties.apiVersion < VK_API_VERSION_1_3){
                memoryAllocateFlagsInfo.value().flags &= ~vk::MemoryAllocateFlagBits::eDeviceAddress;
            }
            memoryAllocateFlagsInfo.value().pNext = pHead;
            pHead = &memoryAllocateFlagsInfo.value();
        }
    }
    else {
        memoryAllocateFlagsInfo = std::nullopt;
    }

    memoryAllocateInfo.pNext = pHead;


    auto deviceMemory = device->GetDeviceVk().allocateMemoryUnique(memoryAllocateInfo);

    if (memoryDedicatedAllocateInfo) {
        memoryDedicatedAllocateInfo.value().pNext = nullptr;
    }
    if (memoryAllocateFlagsInfo) {
        memoryAllocateFlagsInfo.value().pNext = nullptr;
    }

    if (deviceMemory) {
        auto vulkanDeviceMemory = new VulkanDeviceMemory();
        vulkanDeviceMemory->m_Device = device;
        vulkanDeviceMemory->m_DeviceMemory = std::move(deviceMemory);
        vulkanDeviceMemory->m_AllocationSize  = builder.GetAllocationSize();
        vulkanDeviceMemory->m_MemoryTypeIndex = builder.GetMemoryTypeIndex();
        vulkanDeviceMemory->m_MemoryAllocateFlagsInfo     = memoryAllocateFlagsInfo;
        vulkanDeviceMemory->m_MemoryDedicatedAllocateInfo = memoryDedicatedAllocateInfo;
        return std::unique_ptr<VulkanDeviceMemory>(vulkanDeviceMemory);
    }
    return nullptr;
}

BulletRT::Core::VulkanDeviceMemory:: VulkanDeviceMemory() noexcept
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

auto BulletRT::Core::VulkanDeviceMemory::Map(void** pPData, vk::MemoryMapFlags flags) const -> vk::Result
{
    return m_Device->GetDeviceVk().mapMemory(m_DeviceMemory.get(),0, m_AllocationSize,flags,pPData);
}

auto BulletRT::Core::VulkanDeviceMemory::Map(void** pPData, vk::DeviceSize size, vk::DeviceSize offset, vk::MemoryMapFlags flags) const -> vk::Result
{
    return m_Device->GetDeviceVk().mapMemory(m_DeviceMemory.get(),offset, size,flags,pPData);
}

void BulletRT::Core::VulkanDeviceMemory::Unmap() const
{
    return m_Device->GetDeviceVk().unmapMemory(m_DeviceMemory.get());
}

auto BulletRT::Core::VulkanQueueFamily::Acquire(const VulkanDevice* device, uint32_t queueFamilyIndex) noexcept -> std::optional<VulkanQueueFamily>
{
    auto queues = BulletRT::Core::VulkanQueue::Enumerate(device, queueFamilyIndex);
    if (!queues.empty()) {
        auto queueFamily = VulkanQueueFamily();
        queueFamily.m_Device = device;
        queueFamily.m_Queues = queues;
        queueFamily.m_QueueFamilyIndex = queueFamilyIndex;
        return queueFamily;
    }
    return std::nullopt;
}

auto BulletRT::Core::VulkanQueue::Acquire(const VulkanDevice* device, uint32_t queueFamilyIndex, uint32_t queueIndex) -> std::optional<VulkanQueue>
{
    auto queueProperties = device->QueryQueuePriorities(queueFamilyIndex);
    if (queueProperties.size() > queueIndex) {
        VulkanQueue vulkanQueue;
        vulkanQueue.m_Device     = device;
        vulkanQueue.m_Queue      = device->GetDeviceVk().getQueue(queueFamilyIndex, queueIndex);
        vulkanQueue.m_Priority   = queueProperties[queueIndex];
        vulkanQueue.m_QueueIndex = queueIndex;
        vulkanQueue.m_QueueFamilyIndex = queueFamilyIndex;
        return vulkanQueue;
    }
    return std::nullopt;
}

auto BulletRT::Core::VulkanQueue::Enumerate(const VulkanDevice* device, uint32_t queueFamilyIndex) -> std::vector<VulkanQueue>
{
    auto queueProperties = device->QueryQueuePriorities(queueFamilyIndex);
    if (queueProperties.size() > 0) {
        auto vulkanQueues = std::vector<VulkanQueue>();
        vulkanQueues.reserve(queueProperties.size());
        uint32_t i = 0;
        for (auto& queueProperty : queueProperties) {
            VulkanQueue vulkanQueue;
            vulkanQueue.m_Device     = device;
            vulkanQueue.m_Queue      = device->GetDeviceVk().getQueue(queueFamilyIndex, i);
            vulkanQueue.m_Priority   = queueProperty;
            vulkanQueue.m_QueueIndex = i;
            vulkanQueue.m_QueueFamilyIndex = queueFamilyIndex;
            vulkanQueues.push_back(vulkanQueue);
            ++i;
        }
        return vulkanQueues;
    }
    return std::vector<VulkanQueue>();
}


BulletRT::Core::VulkanQueue::VulkanQueue()noexcept
{
    m_Device = nullptr;
    m_Queue  = vk::Queue();
    m_Priority = 1.0f;
    m_QueueFamilyIndex = 0;
    m_QueueIndex = 0;
}

auto BulletRT::Core::VulkanQueueFamilyBuilder::Build(const VulkanDevice* device) const -> std::optional<VulkanQueueFamily>
{
    return BulletRT::Core::VulkanQueueFamily::Acquire(device, GetQueueFamilyIndex());
}

auto BulletRT::Core::VulkanQueueFamily::NewCommandPool()const->std::unique_ptr<VulkanCommandPool>
{
    return BulletRT::Core::VulkanCommandPool::New(GetDevice(), GetQueueFamilyIndex());
}

auto BulletRT::Core::VulkanCommandPool::New(const VulkanDevice* device, uint32_t queueFamilyIndex) noexcept -> std::unique_ptr<VulkanCommandPool>
{
    auto commandPool = device->GetDeviceVk().createCommandPoolUnique(vk::CommandPoolCreateInfo()
        .setQueueFamilyIndex(queueFamilyIndex)
        .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
    );
    if (commandPool) {
        auto vulkanCommandPool                = new VulkanCommandPool();
        vulkanCommandPool->m_Device           = device;
        vulkanCommandPool->m_CommandPool      = std::move(commandPool);
        vulkanCommandPool->m_QueueFamilyIndex = queueFamilyIndex;
        return std::unique_ptr<VulkanCommandPool>(vulkanCommandPool);
    }
    return nullptr;
}

BulletRT::Core::VulkanCommandPool::~VulkanCommandPool() noexcept
{
    m_CommandPool.reset();
}

auto BulletRT::Core::VulkanCommandPool::GetDevice() const noexcept -> const VulkanDevice*
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
    m_CommandPool      = {};
    m_Device           = nullptr;
    m_QueueFamilyIndex = 0;
}

auto BulletRT::Core::VulkanCommandBuffer::New(const VulkanCommandPool* commandPool, vk::CommandBufferLevel commandBufferLevel) noexcept -> std::unique_ptr<VulkanCommandBuffer>
{
    auto commandBuffers = commandPool->GetDevice()->GetDeviceVk().allocateCommandBuffersUnique(
        vk::CommandBufferAllocateInfo().setCommandBufferCount(1).setCommandPool(commandPool->GetCommandPoolVk()).setLevel(commandBufferLevel)
    );
    auto vulkanCommandBuffer = new VulkanCommandBuffer();
    vulkanCommandBuffer->m_CommandPool   = commandPool;
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

auto BulletRT::Core::VulkanCommandBuffer::GetCommandPool() const noexcept -> const VulkanCommandPool*
{
    return m_CommandPool;
}

BulletRT::Core::VulkanCommandBuffer::VulkanCommandBuffer() noexcept
{
    m_CommandBuffer = {};
    m_CommandPool = nullptr;
}

auto BulletRT::Core::VulkanDeviceMemoryBuilder::Build(const VulkanDevice* device) const -> std::unique_ptr<VulkanDeviceMemory>
{
    return VulkanDeviceMemory::New(device,*this);
}

auto BulletRT::Core::VulkanMemoryBuffer::Bind(const VulkanBuffer* buffer, const VulkanDeviceMemory* memory, vk::DeviceSize memoryOffset) -> std::unique_ptr<VulkanMemoryBuffer>
{
    if (!buffer || !memory) {
        return nullptr;
    }
    buffer->GetDevice()->GetDeviceVk().bindBufferMemory(buffer->GetBufferVk(), memory->GetDeviceMemoryVk(), memoryOffset);
    auto memoryBuffer = new VulkanMemoryBuffer();
    memoryBuffer->m_Buffer        = buffer;
    memoryBuffer->m_Memory        = memory;
    memoryBuffer->m_MemoryOffset  = memoryOffset;
    auto memoryAllocationFlagsInfo= memory->GetMemoryAllocateFlagsInfo();
    if (memoryAllocationFlagsInfo.has_value()){
        if (memoryAllocationFlagsInfo.value().flags&vk::MemoryAllocateFlagBits::eDeviceAddress){
            memoryBuffer->m_DeviceAddress = buffer->GetDevice()->GetDeviceVk().getBufferAddress(vk::BufferDeviceAddressInfo().setBuffer(buffer->GetBufferVk()));
        }
    }
    return std::unique_ptr<VulkanMemoryBuffer>(memoryBuffer);
}

BulletRT::Core::VulkanMemoryBuffer::~VulkanMemoryBuffer() noexcept
{
}

auto BulletRT::Core::VulkanMemoryBuffer::Map(void** pPData, vk::DeviceSize size, vk::DeviceSize offset, vk::MemoryMapFlags flags) const -> vk::Result
{
    return m_Memory->Map(pPData,size,m_MemoryOffset+offset,flags);
}

void BulletRT::Core::VulkanMemoryBuffer::Unmap() const
{
    m_Memory->Unmap();
}

BulletRT::Core::VulkanMemoryBuffer::VulkanMemoryBuffer() noexcept
{
    m_Buffer = nullptr;
    m_Memory = nullptr;
    m_MemoryOffset = 0;
    m_DeviceAddress = std::nullopt;
}

auto BulletRT::Core::VulkanMemoryBuffer::Map(void **pPData, vk::MemoryMapFlags flags) const -> vk::Result { 
    return m_Memory->Map(pPData,m_Buffer->GetSize(),m_MemoryOffset,flags);
}


auto BulletRT::Core::VulkanMemoryImage::Bind(const VulkanImage* image, const VulkanDeviceMemory* memory, vk::DeviceSize memoryOffset) -> std::unique_ptr<VulkanMemoryImage>
{
    if (!image || !memory) {
        return nullptr;
    }
    image->GetDevice()->GetDeviceVk().bindImageMemory(image->GetImageVk(), memory->GetDeviceMemoryVk(), memoryOffset);    
    auto memoryImage = new VulkanMemoryImage();
    memoryImage->m_Image  = image;
    memoryImage->m_Memory = memory;
    memoryImage->m_MemoryOffset = memoryOffset;
    return std::unique_ptr<VulkanMemoryImage>(memoryImage);
}

BulletRT::Core::VulkanMemoryImage::~VulkanMemoryImage() noexcept
{
}

auto BulletRT::Core::VulkanMemoryImage::Map(void** pPData, vk::DeviceSize size, vk::DeviceSize offset, vk::MemoryMapFlags flags) const -> vk::Result
{
    return m_Memory->Map(pPData, size, m_MemoryOffset + offset, flags);
}

void BulletRT::Core::VulkanMemoryImage::Unmap() const
{
    return m_Memory->Unmap();
}

BulletRT::Core::VulkanMemoryImage::VulkanMemoryImage() noexcept
{
    m_Image  = nullptr;
    m_Memory = nullptr;
    m_MemoryOffset = 0;
}

auto BulletRT::Core::VulkanFence::New(const VulkanDevice* device, bool isSignaled) -> std::unique_ptr<VulkanFence>
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


using namespace BulletRT::Core;

auto VulkanFence::Wait(uint64_t timeout) const noexcept -> vk::Result {
    vk::Fence fence = m_Fence.get();
    return m_Device->GetDeviceVk().waitForFences(1,&fence,VK_TRUE,timeout);
}


auto VulkanFence::QueryStatus() const noexcept -> vk::Result { 
    return m_Device->GetDeviceVk().getFenceStatus(m_Fence.get());
}


auto VulkanDevice::NewFence(bool isSignaled) -> std::unique_ptr<VulkanFence> { 
    return VulkanFence::New(this,isSignaled);
}
