#include <BulletRT/Utils/VulkanStaging.h>
static auto FindMemoryTypeIndices(const vk::PhysicalDeviceMemoryProperties& memoryProps,
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
auto BulletRT::Utils::VulkanStaging::New(const BulletRT::Core::VulkanDevice* device, vk::DeviceSize size) -> std::unique_ptr<VulkanStaging>
{
    auto vulkanMemoryProperties = device->GetPhysicalDeviceVk().getMemoryProperties();
    auto vulkanStagingBuffer    = std::unique_ptr<BulletRT::Core::VulkanBuffer>(
        BulletRT::Core::VulkanBuffer::Builder()
        .SetUsage(vk::BufferUsageFlagBits::eTransferSrc)
        .SetSize(size)
        .SetQueueFamilyIndices({})
        .Build(device));
    auto sMemRequirements = vulkanStagingBuffer->QueryMemoryRequirements();
    auto sMemTypeIndex = uint32_t(0);
    {
        auto sMemTypeIndices = FindMemoryTypeIndices(vulkanMemoryProperties, sMemRequirements.memoryTypeBits,
            vk::MemoryPropertyFlagBits::eHostVisible |
            vk::MemoryPropertyFlagBits::eHostCoherent |
            vk::MemoryPropertyFlagBits::eHostCached);
        if (sMemTypeIndices.empty())
        {
            return nullptr;
        }
        sMemTypeIndex = sMemTypeIndices.front();
    }
    auto vulkanStagingMemory = BulletRT::Core::VulkanDeviceMemory::Builder()
        .SetAllocationSize(sMemRequirements.size)
        .SetMemoryTypeIndex(sMemTypeIndex)
        .Build(device);
    if (!vulkanStagingMemory) {
        return nullptr;
    }
    auto vulkanStagingMemoryBuffer = BulletRT::Core::VulkanMemoryBuffer::Bind(vulkanStagingBuffer.get(), vulkanStagingMemory.get(), 0);
    if (!vulkanStagingMemoryBuffer) {
        return nullptr;
    }
    auto memoryBuffer = new VulkanStaging();
    memoryBuffer->m_Buffer = std::move(vulkanStagingBuffer);
    memoryBuffer->m_Memory = std::move(vulkanStagingMemory);
    memoryBuffer->m_MemoryBuffer = std::move(vulkanStagingMemoryBuffer);
	return std::unique_ptr<VulkanStaging>(memoryBuffer);
}

BulletRT::Utils::VulkanStaging::~VulkanStaging() noexcept
{
	m_Buffer.reset();
	m_Memory.reset();
	m_MemoryBuffer.reset();
}

auto BulletRT::Utils::VulkanStaging::Map(void** pPData, vk::MemoryMapFlags flags) const -> vk::Result
{
	return m_Memory->Map(pPData,flags);
}

auto BulletRT::Utils::VulkanStaging::Map(void** pPData, vk::DeviceSize size, vk::DeviceSize offset, vk::MemoryMapFlags flags) const -> vk::Result
{
    return m_Memory->Map(pPData,size, offset, flags);
}

void BulletRT::Utils::VulkanStaging::Unmap() const
{
    m_Memory->Unmap();
}

auto BulletRT::Utils::VulkanStaging::Upload(const std::vector<VulkanStagingUploadDesc>& descs) const -> vk::Result
{
    std::vector<VulkanStagingUploadDesc> executeDescs;
    executeDescs.reserve(descs.size());
    size_t minRange = SIZE_MAX;
    size_t maxRange = 0;
    for (auto& desc : descs) {
        if ((desc.offset <= GetSize())&&( desc.offset + desc.sizeInBytes <= GetSize())) {
            executeDescs.push_back(desc);
            minRange = std::min(minRange, desc.offset);
            maxRange = std::max(maxRange, desc.offset + desc.sizeInBytes);
        }
    }
    size_t sizeInBytes = maxRange - minRange;
    void* pMappedData;
    auto res = Map(&pMappedData, sizeInBytes, minRange);
    if (res == vk::Result::eSuccess) {
        for (auto& desc : executeDescs) {
            std::memcpy((char*)pMappedData + desc.offset - minRange, desc.pData, desc.sizeInBytes);
        }
        Unmap();
    }
    return res;
}

auto BulletRT::Utils::VulkanStaging::GetBuffer() const noexcept -> const BulletRT::Core::VulkanBuffer*
{
	return m_Buffer.get();
}

auto BulletRT::Utils::VulkanStaging::GetBufferVk() const noexcept -> vk::Buffer
{
    return m_Buffer ? m_Buffer->GetBufferVk() : nullptr;
}

auto BulletRT::Utils::VulkanStaging::GetMemory() const noexcept -> const BulletRT::Core::VulkanDeviceMemory*
{
	return m_Memory.get();
}

auto BulletRT::Utils::VulkanStaging::GetMemoryVk() const noexcept -> vk::DeviceMemory
{
    return m_Memory ? m_Memory->GetDeviceMemoryVk() : nullptr;
}

auto BulletRT::Utils::VulkanStaging::GetSize() const noexcept -> vk::DeviceSize
{
	return m_Buffer->GetSize();
}

auto BulletRT::Utils::VulkanStaging::GetDeviceAddress() const noexcept -> std::optional<vk::DeviceAddress>
{
	return m_MemoryBuffer->GetDeviceAddress();
}

BulletRT::Utils::VulkanStaging::VulkanStaging() noexcept
{

}
