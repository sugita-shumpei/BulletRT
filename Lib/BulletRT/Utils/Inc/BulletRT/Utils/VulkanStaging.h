#ifndef BULLET_RT_UTILS_VULKAN_STAGING_H
#define BULLET_RT_UTILS_VULKAN_STAGING_H
#include <BulletRT/Core/BulletRTCore.h>
namespace BulletRT
{
    namespace Utils
    {
        struct VulkanStagingUploadDesc
        {
            const void*    pData;
            vk::DeviceSize sizeInBytes;
            vk::DeviceSize offset;
        };
        class VulkanStaging
        {
        public:
            static auto New(const BulletRT::Core::VulkanDevice* device, vk::DeviceSize size)->std::unique_ptr<VulkanStaging>;
            ~VulkanStaging()noexcept;

            auto Map(void** pPData, vk::MemoryMapFlags flags = {})const->vk::Result;
            auto Map(void** pPData, vk::DeviceSize size, vk::DeviceSize offset = 0, vk::MemoryMapFlags flags = {})const->vk::Result;
            void Unmap()const;

            auto Upload(const std::vector< VulkanStagingUploadDesc>& descs)const->vk::Result;

            auto GetBuffer()const noexcept -> const BulletRT::Core::VulkanBuffer*;
            auto GetBufferVk()const noexcept -> vk::Buffer;
            auto GetMemory()const noexcept -> const BulletRT::Core::VulkanDeviceMemory*;
            auto GetMemoryVk()const noexcept -> vk::DeviceMemory;
            auto GetSize()  const noexcept -> vk::DeviceSize;
            auto GetDeviceAddress()const noexcept -> std::optional<vk::DeviceAddress>;
        private:
            VulkanStaging()noexcept;
        private:
            std::unique_ptr<BulletRT::Core::VulkanBuffer>       m_Buffer;
            std::unique_ptr<BulletRT::Core::VulkanDeviceMemory> m_Memory;
            std::unique_ptr<BulletRT::Core::VulkanMemoryBuffer> m_MemoryBuffer;
        };
    }
}
#endif
