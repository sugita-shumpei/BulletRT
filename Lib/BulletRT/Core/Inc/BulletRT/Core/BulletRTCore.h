#ifndef BULLET_RT_CORE_BULLET_RT_CORE_H
#define BULLET_RT_CORE_BULLET_RT_CORE_H
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
#include <unordered_set>
#include <unordered_map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <tuple>
#include <memory>
namespace BulletRT
{
    namespace Core
    {
        class VulkanContext
        {
        public:
            ~VulkanContext() noexcept {}

            VulkanContext(const VulkanContext &) noexcept = delete;
            VulkanContext(VulkanContext &&) noexcept = delete;
            VulkanContext &operator=(const VulkanContext &) noexcept = delete;
            VulkanContext &operator=(VulkanContext &&) noexcept = delete;

            static auto GetHandle() noexcept -> VulkanContext &;
            static void Initialize() noexcept;
            static void Terminate() noexcept;
            static bool IsInitialized() noexcept;

        private:
            VulkanContext() noexcept {}

        private:
            std::unique_ptr<vk::DynamicLoader> m_DLL = nullptr;
        };

        class VulkanInstance;

        class VulkanInstanceBuilder
        {
        public:
            VulkanInstanceBuilder() noexcept
            {
                VulkanContext::Initialize();
                m_MaxApiVersion = vk::enumerateInstanceVersion();
                m_ExtProps = vk::enumerateInstanceExtensionProperties();
                m_LyrProps = vk::enumerateInstanceLayerProperties();
                m_ApiVersion = m_MaxApiVersion;
                m_ApplicationName = "";
                m_EngineName = "";
                m_ApplicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
                m_EngineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
                m_ExtNameSet = {};
                m_LyrNameSet = {};
                m_DebugReportCallbackCreateInfoOp = std::nullopt;
                m_DebugUtilsMessengerCreateInfos = {};
            }
            VulkanInstanceBuilder(const VulkanInstanceBuilder &) noexcept = default;
            VulkanInstanceBuilder &operator=(const VulkanInstanceBuilder &) noexcept = default;

            auto Build() const -> std::unique_ptr<VulkanInstance>;

            auto SetApiVersion(uint32_t apiVersion) noexcept -> VulkanInstanceBuilder &
            {
                if (m_MaxApiVersion >= apiVersion)
                {
                    m_ApiVersion = apiVersion;
                }
                else
                {
                    m_ApiVersion = m_MaxApiVersion;
                }
                return *this;
            }
            auto GetApiVersion() const noexcept -> uint32_t
            {
                return m_ApiVersion;
            }
            auto SetApplicationVersion(uint32_t applicationVersion) noexcept -> VulkanInstanceBuilder &
            {
                m_ApplicationVersion = applicationVersion;
                return *this;
            }
            auto GetApplicationVersion() const noexcept -> uint32_t
            {
                return m_ApplicationVersion;
            }
            auto SetEngineVersion(uint32_t engineVersion) noexcept -> VulkanInstanceBuilder &
            {
                m_EngineVersion = engineVersion;
                return *this;
            }
            auto GetEngineVersion() const noexcept -> uint32_t
            {
                return m_EngineVersion;
            }

            auto SetApplicationName(const std::string &appName) noexcept -> VulkanInstanceBuilder &
            {
                m_ApplicationName = appName;
                return *this;
            }
            auto GetApplicationName() const noexcept -> std::string { return m_ApplicationName; }
            auto SetEngineName(const std::string &engName) noexcept -> VulkanInstanceBuilder &
            {
                m_EngineName = engName;
                return *this;
            }
            auto GetEngineName() const noexcept -> std::string { return m_EngineName; }

            auto SetExtension(const std::string &extName) noexcept -> VulkanInstanceBuilder &
            {
                auto isContained = std::find_if(std::begin(m_ExtProps), std::end(m_ExtProps), [extName](const auto &extProp)
                                                { return std::string(extProp.extensionName.data()) == extName; }) != std::end(m_ExtProps);
                if (isContained)
                {
                    m_ExtNameSet.insert(extName);
                }
                return *this;
            }
            auto GetExtensionSet() const noexcept -> const std::unordered_set<std::string> & { return m_ExtNameSet; }

            auto SetLayer(const std::string &lyrName) noexcept -> VulkanInstanceBuilder &
            {
                auto isContained = std::find_if(std::begin(m_LyrProps), std::end(m_LyrProps), [lyrName](const auto &lyrProp)
                                                { return std::string(lyrProp.layerName.data()) == lyrName; }) != std::end(m_LyrProps);
                if (isContained)
                {
                    m_LyrNameSet.insert(lyrName);
                }
                return *this;
            }
            auto GetLayerSet() const noexcept -> const std::unordered_set<std::string> & { return m_LyrNameSet; }

            auto SetDebugReportCallback(const vk::DebugReportCallbackCreateInfoEXT &debugReportCallback) noexcept -> VulkanInstanceBuilder &
            {
                m_DebugReportCallbackCreateInfoOp = debugReportCallback;
                m_DebugReportCallbackCreateInfoOp.value().pNext = nullptr;
                return *this;
            }
            auto GetDebugReportCallback() const noexcept -> const std::optional<vk::DebugReportCallbackCreateInfoEXT> &
            {
                return m_DebugReportCallbackCreateInfoOp;
            }

            auto SetDebugUtilsMessenger(size_t idx, const vk::DebugUtilsMessengerCreateInfoEXT &debugUtilsMessenger) noexcept -> VulkanInstanceBuilder &
            {
                if (m_DebugUtilsMessengerCreateInfos.size() > idx)
                {
                    m_DebugUtilsMessengerCreateInfos[idx] = debugUtilsMessenger;
                }
                return *this;
            }
            auto AddDebugUtilsMessenger(const vk::DebugUtilsMessengerCreateInfoEXT &debugUtilsMessenger) noexcept -> VulkanInstanceBuilder &
            {
                m_DebugUtilsMessengerCreateInfos.push_back(debugUtilsMessenger);
                return *this;
            }
            auto GetDebugUtilsMessengers() const noexcept -> const std::vector<vk::DebugUtilsMessengerCreateInfoEXT> &
            {
                return m_DebugUtilsMessengerCreateInfos;
            }

        private:
            uint32_t m_ApiVersion;
            uint32_t m_ApplicationVersion;
            uint32_t m_EngineVersion;
            std::string m_ApplicationName;
            std::string m_EngineName;
            std::unordered_set<std::string> m_ExtNameSet;
            std::unordered_set<std::string> m_LyrNameSet;
            std::optional<vk::DebugReportCallbackCreateInfoEXT> m_DebugReportCallbackCreateInfoOp;
            std::vector<vk::DebugUtilsMessengerCreateInfoEXT> m_DebugUtilsMessengerCreateInfos;

            uint32_t m_MaxApiVersion;
            std::vector<vk::ExtensionProperties> m_ExtProps;
            std::vector<vk::LayerProperties> m_LyrProps;
        };

        class VulkanInstance
        {
        public:
            using Builder = VulkanInstanceBuilder;

        public:
            static auto New(const VulkanInstanceBuilder &builder) noexcept -> std::unique_ptr<VulkanInstance>;
            ~VulkanInstance() noexcept;

            auto GetInstanceVk() const noexcept -> vk::Instance { return m_Instance.get(); }
            auto GetDebugUtilsMessengersVk() const noexcept -> std::vector<vk::DebugUtilsMessengerEXT>
            {
                std::vector<vk::DebugUtilsMessengerEXT> debugUtilsMessengers;
                debugUtilsMessengers.reserve(m_DebugUtilsMessengers.size());
                for (auto &debugUtilsMessenger : m_DebugUtilsMessengers)
                {
                    debugUtilsMessengers.push_back(debugUtilsMessenger.get());
                }
                return debugUtilsMessengers;
            }
            auto GetDebugUtilsMessengerVk(size_t idx) const noexcept -> vk::DebugUtilsMessengerEXT
            {
                if (m_DebugUtilsMessengers.size() > idx)
                {
                    return m_DebugUtilsMessengers[idx].get();
                }
                else
                {
                    return nullptr;
                }
            }
            auto GetDebugReportCallbackVk() const noexcept -> vk::DebugReportCallbackEXT
            {
                return m_DebugReportCallback.get();
            }
            auto GetApiVersion() const noexcept -> uint32_t
            {
                return m_ApiVersion;
            }
            bool SupportExtension(const char *extName) const noexcept
            {
                return m_EnabledExtNameSet.count(extName) > 0;
            }
            bool SupportLayer(const char *lyrName) const noexcept
            {
                return m_EnabledLyrNameSet.count(lyrName) > 0;
            }

        private:
            VulkanInstance() noexcept;

        private:
            vk::UniqueInstance m_Instance;
            std::vector<vk::UniqueDebugUtilsMessengerEXT> m_DebugUtilsMessengers;
            vk::UniqueDebugReportCallbackEXT m_DebugReportCallback;
            uint32_t m_ApiVersion;
            uint32_t m_ApplicationVersion;
            uint32_t m_EngineVersion;
            std::string m_ApplicationName;
            std::string m_EngineName;
            std::unordered_set<std::string> m_EnabledExtNameSet;
            std::unordered_set<std::string> m_EnabledLyrNameSet;
            std::optional<vk::DebugReportCallbackCreateInfoEXT> m_DebugReportCallbackCreateInfo;
            std::vector<vk::DebugUtilsMessengerCreateInfoEXT> m_DebugUtilsMessengerCreateInfos;
        };

        class VulkanDeviceFeaturesSet
        {
        public:
            VulkanDeviceFeaturesSet() noexcept {}
            ~VulkanDeviceFeaturesSet() noexcept {}

            VulkanDeviceFeaturesSet(const VulkanDeviceFeaturesSet &featureSet) noexcept;
            VulkanDeviceFeaturesSet(VulkanDeviceFeaturesSet &&featureSet) noexcept;

            VulkanDeviceFeaturesSet &operator=(const VulkanDeviceFeaturesSet &featureSet) noexcept;
            VulkanDeviceFeaturesSet &operator=(VulkanDeviceFeaturesSet &&featureSet) noexcept;

            void Clear() noexcept
            {
                m_IndexMap.clear();
                m_Holders.clear();
            }
            bool Empty() const noexcept { return m_IndexMap.empty(); }
            template <typename VulkanExtFeatureType>
            bool Contain() const noexcept
            {
                return Impl_Contain(VulkanExtFeatureType::structureType);
            }
            template <typename VulkanExtFeatureType>
            bool Insert(const VulkanExtFeatureType &feature) noexcept
            {
                if (!Contain<VulkanExtFeatureType>())
                {
                    Impl_Insert<VulkanExtFeatureType>(feature);
                    return true;
                }
                return false;
            }
            template <typename VulkanExtFeatureType>
            auto Get() noexcept -> const VulkanExtFeatureType *
            {
                if (Contain<VulkanExtFeatureType>())
                {
                    return static_cast<const VulkanExtFeatureType *>(Impl_Find(VulkanExtFeatureType::structureType));
                }
                else
                {
                    return Impl_Insert<VulkanExtFeatureType>({});
                }
            }
            template <typename VulkanExtFeatureType>
            auto Find() const noexcept -> const VulkanExtFeatureType *
            {
                return static_cast<const VulkanExtFeatureType *>(Impl_Read(VulkanExtFeatureType::structureType));
            }
            template <typename VulkanExtFeatureType>
            bool Read(VulkanExtFeatureType &feature) const noexcept
            {
                if (auto pData = Find<VulkanExtFeatureType>())
                {
                    std::memcpy((void *)&feature, pData, sizeof(VulkanExtFeatureType));
                    feature.pNext = nullptr;
                    return true;
                }
                return false;
            }
            template <typename VulkanExtFeatureType>
            auto Read() const noexcept -> std::optional<VulkanExtFeatureType>
            {
                VulkanExtFeatureType res;
                if (Read(res))
                {
                    return res;
                }
                else
                {
                    return std::nullopt;
                }
            }
            template <typename VulkanExtFeatureType>
            bool Write(const VulkanExtFeatureType &feature) noexcept
            {
                return Impl_Write(VulkanExtFeatureType::structureType, (const void *)&feature);
            }
            template <typename VulkanExtFeatureType>
            bool Map(VulkanExtFeatureType **ppFeature) noexcept
            {
                return Impl_Map(VulkanExtFeatureType::structureType, (void **)ppFeature);
            }
            template <typename VulkanExtFeatureType>
            bool Unmap() noexcept
            {
                return Impl_Unmap(VulkanExtFeatureType::structureType);
            }

            auto ReadHead() const noexcept -> const void *
            {
                if (!m_Holders.empty())
                {
                    return m_Holders.front()->Read();
                }
                return nullptr;
            }
            void LinkTail(void *pNext) noexcept
            {
                if (!m_Holders.empty())
                {
                    m_Holders.back()->Link(pNext);
                    m_TailPNext = pNext;
                }
            }
            void UnlinkTail() noexcept
            {
                if (!m_Holders.empty())
                {
                    m_Holders.back()->Unlink();
                    m_TailPNext = nullptr;
                }
            }

        private:
            template <typename VulkanExtFeatureType>
            auto Impl_Insert(const VulkanExtFeatureType &feature) noexcept -> VulkanExtFeatureType *;
            bool Impl_Contain(vk::StructureType sType) const noexcept;
            auto Impl_Read(vk::StructureType sType) const noexcept -> const void *;
            bool Impl_Write(vk::StructureType sType, const void *) noexcept;
            bool Impl_Map(vk::StructureType sType, void **) noexcept;
            bool Impl_Unmap(vk::StructureType sType) noexcept;

        private:
            class IVulkanDeviceExtFeatureHolder
            {
            public:
                virtual ~IVulkanDeviceExtFeatureHolder() noexcept {}
                virtual auto Clone() const noexcept -> IVulkanDeviceExtFeatureHolder * = 0;
                virtual auto Read() const noexcept -> const void * = 0;
                virtual void Write(const void *pData) noexcept = 0;
                virtual void Link(void *pNext) noexcept = 0;
                virtual void Unlink() noexcept = 0;
                virtual auto GetPointer() noexcept -> void * = 0;
                virtual void Map() noexcept = 0;
                virtual void Unmap() noexcept = 0;
            };
            template <typename VulkanExtFeatureType>
            class VulkanDeviceExtFeatureHolder : public IVulkanDeviceExtFeatureHolder
            {
            public:
                VulkanDeviceExtFeatureHolder(const VulkanExtFeatureType &feature) noexcept
                {
                    m_Feature = feature;
                    m_Feature.pNext = nullptr;
                    m_PNextForMap = nullptr;
                }
                virtual ~VulkanDeviceExtFeatureHolder() noexcept {}
                virtual auto Clone() const noexcept -> IVulkanDeviceExtFeatureHolder *
                {
                    return new VulkanDeviceExtFeatureHolder(m_Feature);
                }
                virtual auto Read() const noexcept -> const void *
                {
                    return &m_Feature;
                }
                virtual void Write(const void *pData) noexcept
                {
                    auto pNext = m_Feature.pNext;
                    std::memcpy(&m_Feature, pData, sizeof(m_Feature));
                    m_Feature.pNext = pNext;
                }
                virtual void Link(void *pNext) noexcept
                {
                    m_Feature.pNext = pNext;
                }
                virtual void Unlink() noexcept
                {
                    m_Feature.pNext = nullptr;
                }
                virtual auto GetPointer() noexcept -> void *
                {
                    return &m_Feature;
                }
                virtual void Map() noexcept
                {
                    m_PNextForMap = m_Feature.pNext;
                }
                virtual void Unmap() noexcept
                {
                    m_Feature.pNext = m_PNextForMap;
                    m_PNextForMap = nullptr;
                }

            private:
                VulkanExtFeatureType m_Feature;
                decltype(VulkanExtFeatureType::pNext) m_PNextForMap;
            };

        private:
            std::unordered_map<vk::StructureType, size_t> m_IndexMap = {};
            std::vector<std::unique_ptr<IVulkanDeviceExtFeatureHolder>> m_Holders = {};
            void *m_TailPNext = nullptr;
        };

        template <typename VulkanExtFeatureType>
        auto VulkanDeviceFeaturesSet::Impl_Insert(const VulkanExtFeatureType &feature) noexcept -> VulkanExtFeatureType *
        {
            if (!m_Holders.empty())
            {
                auto size = m_Holders.size();
                auto *pBackHolder = m_Holders.back().get();
                m_Holders.push_back(std::unique_ptr<VulkanDeviceFeaturesSet::IVulkanDeviceExtFeatureHolder>(
                    new VulkanDeviceFeaturesSet::VulkanDeviceExtFeatureHolder<VulkanExtFeatureType>(feature)));
                m_IndexMap.insert({VulkanExtFeatureType::structureType, size});
                pBackHolder->Link(m_Holders.back()->GetPointer());
                m_Holders.back()->Link(m_TailPNext);
            }
            else
            {
                m_Holders.push_back(std::unique_ptr<VulkanDeviceFeaturesSet::IVulkanDeviceExtFeatureHolder>(
                    new VulkanDeviceFeaturesSet::VulkanDeviceExtFeatureHolder<VulkanExtFeatureType>(feature)));
                m_IndexMap.insert({VulkanExtFeatureType::structureType, 0});
            }
            return static_cast<VulkanExtFeatureType *>(m_Holders.back()->GetPointer());
        }

        class VulkanDevice;

        class VulkanQueue
        {
        public:
            static auto Acquire(const VulkanDevice *device, uint32_t queueFamilyIndex, uint32_t queueIndex) -> std::optional<VulkanQueue>;
            static auto Enumerate(const VulkanDevice *device, uint32_t queueFamilyIndex) -> std::vector<VulkanQueue>;

            VulkanQueue(const VulkanQueue &) noexcept = default;
            VulkanQueue &operator=(const VulkanQueue &) noexcept = default;

            auto GetDevice() const noexcept -> const VulkanDevice * { return m_Device; }
            auto GetQueueVk() const noexcept -> vk::Queue { return m_Queue; }
            auto GetQueueFamilyIndex() const noexcept -> uint32_t { return m_QueueFamilyIndex; }
            auto GetQueueIndex() const noexcept -> uint32_t { return m_QueueIndex; }
            auto GetQueuePriority() const noexcept -> float { return m_Priority; }

        private:
            VulkanQueue() noexcept;

        private:
            const VulkanDevice *m_Device;
            vk::Queue m_Queue;
            uint32_t m_QueueFamilyIndex;
            uint32_t m_QueueIndex;
            float m_Priority;
        };

        class VulkanQueueFamily;

        class VulkanQueueFamilyBuilder
        {
        public:
            VulkanQueueFamilyBuilder() noexcept
            {
                m_QueueFamilyIndex = 0;
                m_QueueProperties = {};
            }
            VulkanQueueFamilyBuilder(uint32_t queueFamilyIndex) noexcept
            {
                m_QueueFamilyIndex = queueFamilyIndex;
                m_QueueProperties = {};
            }
            VulkanQueueFamilyBuilder(uint32_t queueFamilyIndex, uint32_t queueCount) noexcept
            {
                m_QueueFamilyIndex = queueFamilyIndex;
                m_QueueProperties = std::vector<float>(queueCount, 1.0f);
            }

            VulkanQueueFamilyBuilder(const VulkanQueueFamilyBuilder &builder) noexcept = default;
            VulkanQueueFamilyBuilder &operator=(const VulkanQueueFamilyBuilder &builder) noexcept = default;

            auto SetQueueFamilyIndex(uint32_t queueFamilyIndex) noexcept -> VulkanQueueFamilyBuilder &
            {
                m_QueueFamilyIndex = queueFamilyIndex;
                return *this;
            }
            auto GetQueueFamilyIndex() const noexcept -> uint32_t { return m_QueueFamilyIndex; }
            auto SetQueueCount(uint32_t queueCount) noexcept -> VulkanQueueFamilyBuilder &
            {
                m_QueueProperties = std::vector<float>(queueCount, 1.0f);
                return *this;
            }
            auto GetQueueCount() const noexcept -> uint32_t { return m_QueueProperties.size(); }
            auto SetQueueProperty(size_t idx, float val) noexcept -> VulkanQueueFamilyBuilder &
            {
                if (m_QueueProperties.size() > idx)
                {
                    m_QueueProperties[idx] = val;
                }
                return *this;
            }
            auto SetQueueProperties(const std::vector<float> &properties) noexcept -> VulkanQueueFamilyBuilder &
            {
                m_QueueProperties = properties;
                return *this;
            }
            auto GetQueueProperties() const noexcept -> const std::vector<float> &
            {
                return m_QueueProperties;
            }

            auto Build(const VulkanDevice *device) const -> std::optional<VulkanQueueFamily>;

        private:
            uint32_t m_QueueFamilyIndex;
            std::vector<float> m_QueueProperties;
        };

        class VulkanCommandPool;

        class VulkanQueueFamily
        {
        public:
            using Builder = VulkanQueueFamilyBuilder;
            static auto Acquire(const VulkanDevice *device, uint32_t queueFamilyIndex) noexcept -> std::optional<VulkanQueueFamily>;

            VulkanQueueFamily(const VulkanQueueFamily &queueFamily) noexcept = default;
            VulkanQueueFamily &operator=(const VulkanQueueFamily &queueFamily) noexcept = default;

            auto GetDevice() const noexcept -> const VulkanDevice * { return m_Device; }
            auto GetQueueFamilyIndex() const noexcept -> uint32_t { return m_QueueFamilyIndex; }
            auto GetQueues() const noexcept -> const std::vector<VulkanQueue> & { return m_Queues; }

            auto NewCommandPool() const -> std::unique_ptr<VulkanCommandPool>;

        private:
            VulkanQueueFamily() noexcept : m_Device{nullptr}, m_QueueFamilyIndex{0}, m_Queues{} {}

        private:
            const VulkanDevice *m_Device;
            uint32_t m_QueueFamilyIndex;
            std::vector<VulkanQueue> m_Queues;
        };

        class VulkanCommandBuffer;

        class VulkanCommandPool
        {
        public:
            static auto New(const VulkanDevice *device, uint32_t queueFamilyIndex) noexcept -> std::unique_ptr<VulkanCommandPool>;
            ~VulkanCommandPool() noexcept;

            auto GetDevice() const noexcept -> const VulkanDevice *;
            auto GetCommandPoolVk() const noexcept -> vk::CommandPool;
            auto GetQueueFamilyIndex() const noexcept -> uint32_t;

            auto NewCommandBuffer(vk::CommandBufferLevel level) const noexcept -> std::unique_ptr<VulkanCommandBuffer>;

        private:
            VulkanCommandPool() noexcept;

        private:
            const VulkanDevice *m_Device;
            vk::UniqueCommandPool m_CommandPool;
            uint32_t m_QueueFamilyIndex;
        };

        class VulkanCommandBuffer
        {
        public:
            static auto New(const VulkanCommandPool *commandPool, vk::CommandBufferLevel commandBufferLevel) noexcept -> std::unique_ptr<VulkanCommandBuffer>;
            ~VulkanCommandBuffer() noexcept;

            auto GetCommandPool() const noexcept -> const VulkanCommandPool *;
            auto GetCommandBufferVk() const noexcept -> vk::CommandBuffer;

        private:
            VulkanCommandBuffer() noexcept;

        private:
            const VulkanCommandPool *m_CommandPool;
            vk::UniqueCommandBuffer m_CommandBuffer;
        };

        class VulkanDeviceBuilder
        {
        public:
            VulkanDeviceBuilder(const VulkanInstance *instance, vk::PhysicalDevice physicalDevice) noexcept
            {
                m_Instance = instance;
                m_PhysicalDevice = physicalDevice;
                m_ExtNameSet = {};
                m_FeaturesSet.Insert<vk::PhysicalDeviceFeatures2>(physicalDevice.getFeatures2());
                m_QueueFamilyMap = {};
                m_ExtProps = physicalDevice.enumerateDeviceExtensionProperties();
                m_QueueFamilyProperties = physicalDevice.getQueueFamilyProperties();
            }
            VulkanDeviceBuilder(const VulkanDeviceBuilder &) noexcept = default;
            VulkanDeviceBuilder &operator=(const VulkanDeviceBuilder &) noexcept = default;

            static auto Enumerate(const VulkanInstance *instance) noexcept -> std::vector<VulkanDeviceBuilder>
            {
                auto physicalDevices = instance->GetInstanceVk().enumeratePhysicalDevices();
                auto deviceBuilders = std::vector<VulkanDeviceBuilder>();
                deviceBuilders.reserve(physicalDevices.size());
                for (auto &physicalDevice : physicalDevices)
                {
                    deviceBuilders.push_back(VulkanDeviceBuilder(instance, physicalDevice));
                }
                return deviceBuilders;
            }
            //
            auto GetInstance() const noexcept -> const VulkanInstance * { return m_Instance; }
            auto GetPhysicalDevice() const noexcept -> vk::PhysicalDevice { return m_PhysicalDevice; }
            // Extensions
            auto SetExtension(const std::string &extName) noexcept -> VulkanDeviceBuilder &
            {
                auto isContained = std::find_if(std::begin(m_ExtProps), std::end(m_ExtProps), [extName](const auto &extProp)
                                                { return std::string(extProp.extensionName.data()) == extName; }) != std::end(m_ExtProps);
                if (isContained)
                {
                    m_ExtNameSet.insert(extName);
                }
                return *this;
            }
            auto GetExtensionSet() const noexcept -> const std::unordered_set<std::string> & { return m_ExtNameSet; }
            // Features
            template <typename VulkanFeaturesType>
            auto SetFeatures(const VulkanFeaturesType &features) noexcept -> VulkanDeviceBuilder &
            {
                if (
                    !m_FeaturesSet.Insert(features))
                {
                    m_FeaturesSet.Write(features);
                }
                return *this;
            }
            auto GetFeaturesSet() const noexcept -> const VulkanDeviceFeaturesSet & { return m_FeaturesSet; }
            template <typename VulkanFeaturesType>
            auto ResetFeatures() noexcept -> VulkanDeviceBuilder &
            {
                if constexpr (std::is_same_v<vk::PhysicalDeviceFeatures2, VulkanFeaturesType>)
                {
                    auto features = m_PhysicalDevice.getFeatures2();
                    return SetFeatures(features);
                }
                else
                {
                    auto featuresChain = m_PhysicalDevice.getFeatures2<vk::PhysicalDeviceFeatures2, VulkanFeaturesType>();
                    auto features = featuresChain.template get<VulkanFeaturesType>();
                    return SetFeatures(features);
                }
            }
            // QueueFamilyProperties
            auto SetQueueFamily(const VulkanQueueFamilyBuilder &queueFamily) noexcept -> VulkanDeviceBuilder &
            {
                if (m_QueueFamilyProperties.size() > queueFamily.GetQueueFamilyIndex())
                {
                    if (m_QueueFamilyProperties[queueFamily.GetQueueFamilyIndex()].queueCount >= queueFamily.GetQueueCount() && queueFamily.GetQueueCount() > 0)
                    {
                        m_QueueFamilyMap[queueFamily.GetQueueFamilyIndex()] = queueFamily;
                    }
                }
                return *this;
            }
            auto SetQueueFamilies(const std::vector<VulkanQueueFamilyBuilder> &queueFamilies) noexcept -> VulkanDeviceBuilder &
            {
                for (auto &queueFamily : queueFamilies)
                {
                    SetQueueFamily(queueFamily);
                }
                return *this;
            }
            auto GetQueueFamilySet() const noexcept -> const std::unordered_map<uint32_t, VulkanQueueFamilyBuilder> &
            {
                return m_QueueFamilyMap;
            }
            auto Build() const -> std::unique_ptr<VulkanDevice>;

        private:
            const VulkanInstance *m_Instance;
            vk::PhysicalDevice m_PhysicalDevice;
            std::unordered_set<std::string> m_ExtNameSet;
            VulkanDeviceFeaturesSet m_FeaturesSet;
            std::unordered_map<uint32_t, VulkanQueueFamilyBuilder> m_QueueFamilyMap;
            std::vector<vk::ExtensionProperties> m_ExtProps;
            std::vector<vk::QueueFamilyProperties> m_QueueFamilyProperties;
        };

        class VulkanFence;

        class VulkanDevice
        {
        public:
            using Builder = VulkanDeviceBuilder;
            static auto New(const VulkanDeviceBuilder &builder) noexcept -> std::unique_ptr<VulkanDevice>;
            virtual ~VulkanDevice() noexcept;

            auto NewFence(bool isSignaled = false) -> std::unique_ptr<VulkanFence>;
            auto WaitForFences(const std::vector<const VulkanFence *> &fences, uint64_t timeOut = UINT64_MAX, VkBool32 waitForAll = VK_FALSE) -> vk::Result;

            auto GetInstance() const noexcept -> const VulkanInstance * { return m_Instance; }
            auto GetPhysicalDeviceVk() const noexcept -> vk::PhysicalDevice { return m_PhysicalDevice; }
            auto GetDeviceVk() const noexcept -> vk::Device { return m_LogigalDevice.get(); }

            auto EnumerateQueues(uint32_t queueFamilyIndex) const -> std::vector<VulkanQueue>;
            auto AcquireQueueFamily(uint32_t queueFamilyIndex) const -> std::optional<VulkanQueueFamily>;
            bool SupportQueueFamily(uint32_t queueFamilyIndex) const noexcept { return m_QueueFamilyMap.count(queueFamilyIndex) > 0; }
            bool SupportExtension(const char *extName) const noexcept
            {
                return m_EnabledExtNameSet.count(extName) > 0;
            }
            auto QueryQueuePriorities(uint32_t queueFamilyIndex) const noexcept -> std::vector<float>
            {
                return m_QueueFamilyMap.count(queueFamilyIndex) > 0 ? m_QueueFamilyMap.at(queueFamilyIndex).GetQueueProperties() : std::vector<float>{};
            }
            auto QueryQueueCount(uint32_t queueFamilyIndex) const noexcept -> uint32_t { return m_QueueFamilyMap.count(queueFamilyIndex) > 0 ? m_QueueFamilyMap.at(queueFamilyIndex).GetQueueCount() : 0; }
            template <typename VulkanFeatureType>
            auto QueryFeatures(VulkanFeatureType &features) const noexcept -> bool
            {
                return m_EnabledFeaturesSet.Read(features);
                ;
            }
            template <typename VulkanFeatureType>
            auto QueryFeatures() const noexcept -> std::optional<VulkanFeatureType>
            {
                return m_EnabledFeaturesSet.Read<VulkanFeatureType>();
            }

        private:
            VulkanDevice() noexcept;

        private:
            const VulkanInstance *m_Instance;
            vk::PhysicalDevice m_PhysicalDevice;
            vk::UniqueDevice m_LogigalDevice;
            std::unordered_set<std::string> m_EnabledExtNameSet;
            VulkanDeviceFeaturesSet m_EnabledFeaturesSet;
            std::unordered_map<uint32_t, VulkanQueueFamilyBuilder> m_QueueFamilyMap;
        };

        class VulkanFence
        {
        public:
            static auto New(const VulkanDevice *device, bool isSignaled = false) -> std::unique_ptr<VulkanFence>;
            virtual ~VulkanFence() noexcept;

            auto Wait(uint64_t timeout) const noexcept -> vk::Result;
            auto QueryStatus() const noexcept -> vk::Result;
            auto GetFenceVk() const noexcept -> vk::Fence { return m_Fence.get(); }
            auto GetDevice() const noexcept -> const VulkanDevice * { return m_Device; }
            auto GetDeviceVk() const noexcept -> vk::Device { return m_Device->GetDeviceVk(); }

        private:
            VulkanFence() noexcept;

        private:
            const VulkanDevice *m_Device;
            vk::UniqueFence m_Fence;
        };

        class VulkanBuffer;

        class VulkanBufferBuilder
        {
        public:
            VulkanBufferBuilder() noexcept
            {
                m_Flags = {};
                m_Size = 0;
                m_Usage = {};
                m_QueueFamilyIndices = {};
            }

            VulkanBufferBuilder(const VulkanBufferBuilder &) noexcept = default;

            VulkanBufferBuilder &operator=(const VulkanBufferBuilder &) noexcept = default;

            auto GetFlags() const noexcept -> vk::BufferCreateFlags { return m_Flags; }
            auto SetFlags(vk::BufferCreateFlags flags) noexcept -> VulkanBufferBuilder &
            {
                m_Flags = flags;
                return *this;
            }
            auto GetSize() const noexcept -> vk::DeviceSize { return m_Size; }
            auto SetSize(vk::DeviceSize size) noexcept -> VulkanBufferBuilder &
            {
                m_Size = size;
                return *this;
            }
            auto GetUsage() const noexcept -> vk::BufferUsageFlags { return m_Usage; }
            auto SetUsage(vk::BufferUsageFlags usage) noexcept -> VulkanBufferBuilder &
            {
                m_Usage = usage;
                return *this;
            }

            auto GetQueueFamilyIndices() const noexcept -> std::vector<uint32_t> { return m_QueueFamilyIndices; }
            auto SetQueueFamilyIndices(const std::vector<uint32_t> &queueFamilyIndices) noexcept -> VulkanBufferBuilder &
            {
                m_QueueFamilyIndices = queueFamilyIndices;
                return *this;
            }
            auto GetSharingMode() const noexcept -> vk::SharingMode { return m_QueueFamilyIndices.empty() ? vk::SharingMode::eExclusive : vk::SharingMode::eConcurrent; }

            auto Build(const VulkanDevice *device) const noexcept -> std::unique_ptr<VulkanBuffer>;

        private:
            vk::BufferCreateFlags m_Flags;
            vk::DeviceSize m_Size;
            vk::BufferUsageFlags m_Usage;
            std::vector<uint32_t> m_QueueFamilyIndices;
        };

        class VulkanBuffer
        {
        public:
            using Builder = VulkanBufferBuilder;
            static auto New(const VulkanDevice *device, const VulkanBufferBuilder &buffer) -> std::unique_ptr<VulkanBuffer>;
            virtual ~VulkanBuffer() noexcept;

            auto QueryMemoryRequirements() const -> vk::MemoryRequirements;

            auto GetDevice() const noexcept -> const VulkanDevice * { return m_Device; }
            auto GetBufferVk() const noexcept -> vk::Buffer { return m_Buffer.get(); }
            auto GetFlags() const noexcept -> vk::BufferCreateFlags { return m_Flags; }
            auto GetSize() const noexcept -> vk::DeviceSize { return m_Size; }
            auto GetUsage() const noexcept -> vk::BufferUsageFlags { return m_Usage; }
            auto GetSharingMode() const noexcept -> vk::SharingMode { return m_QueueFamilyIndices.empty() ? vk::SharingMode::eExclusive : vk::SharingMode::eConcurrent; }

        private:
            VulkanBuffer() noexcept;

        private:
            const VulkanDevice *m_Device;
            vk::UniqueBuffer m_Buffer;
            vk::BufferCreateFlags m_Flags;
            vk::DeviceSize m_Size;
            vk::BufferUsageFlags m_Usage;
            std::vector<uint32_t> m_QueueFamilyIndices;
        };

        class VulkanImage;

        class VulkanImageBuilder
        {
        public:
            VulkanImageBuilder() noexcept
            {
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

            VulkanImageBuilder(const VulkanImageBuilder &) noexcept = default;

            VulkanImageBuilder &operator=(const VulkanImageBuilder &) noexcept = default;

            auto GetFlags() const noexcept -> vk::ImageCreateFlags { return m_Flags; }
            auto SetFlags(vk::ImageCreateFlags flags) noexcept -> VulkanImageBuilder &
            {
                m_Flags = flags;
                return *this;
            }

            auto GetImageType() const noexcept -> vk::ImageType { return m_ImageType; }
            auto SetImageType(vk::ImageType imageType) noexcept -> VulkanImageBuilder &
            {
                m_ImageType = imageType;
                return *this;
            }

            auto GetExtent() const noexcept -> const vk::Extent3D & { return m_Extent; }
            auto SetExtent(const vk::Extent3D &extent) noexcept -> VulkanImageBuilder &
            {
                m_Extent = extent;
                return *this;
            }

            auto GetFormat() const noexcept -> const vk::Format { return m_Format; }
            auto SetFormat(const vk::Format &format) noexcept -> VulkanImageBuilder &
            {
                m_Format = format;
                return *this;
            }

            auto GetMipLevels() const noexcept -> uint32_t { return m_MipLevels; }
            auto SetMipLevels(uint32_t mipLevels) noexcept -> VulkanImageBuilder &
            {
                m_MipLevels = mipLevels;
                return *this;
            }

            auto GetArrayLayers() const noexcept -> uint32_t { return m_ArrayLayers; }
            auto SetArrayLayers(uint32_t arrayLayers) noexcept -> VulkanImageBuilder &
            {
                m_ArrayLayers = arrayLayers;
                return *this;
            }

            auto GetSamples() const noexcept -> vk::SampleCountFlagBits { return m_Samples; }
            auto SetSamples(vk::SampleCountFlagBits samples) noexcept -> VulkanImageBuilder &
            {
                m_Samples = samples;
                return *this;
            }

            auto GetTiling() const noexcept -> vk::ImageTiling { return m_Tiling; }
            auto SetTiling(vk::ImageTiling tiling) noexcept -> VulkanImageBuilder &
            {
                m_Tiling = tiling;
                return *this;
            }

            auto GetUsage() const noexcept -> vk::ImageUsageFlags { return m_Usage; }
            auto SetUsage(vk::ImageUsageFlags usage) noexcept -> VulkanImageBuilder &
            {
                m_Usage = usage;
                return *this;
            }

            auto GetQueueFamilyIndices() const noexcept -> std::vector<uint32_t> { return m_QueueFamilyIndices; }
            auto SetQueueFamilyIndices(const std::vector<uint32_t> &queueFamilyIndices) noexcept -> VulkanImageBuilder &
            {
                m_QueueFamilyIndices = queueFamilyIndices;
                return *this;
            }
            auto GetSharingMode() const noexcept -> vk::SharingMode { return m_QueueFamilyIndices.empty() ? vk::SharingMode::eExclusive : vk::SharingMode::eConcurrent; }

            auto GetInitialLayout() const noexcept -> vk::ImageLayout { return m_InitialLayout; }
            auto SetInitialLayout(vk::ImageLayout initialLayout) noexcept -> VulkanImageBuilder &
            {
                m_InitialLayout = initialLayout;
                return *this;
            }

            auto Build(const VulkanDevice *device) const noexcept -> std::unique_ptr<VulkanImage>;

        private:
            vk::ImageCreateFlags m_Flags;
            vk::ImageType m_ImageType;
            vk::Format m_Format;
            vk::Extent3D m_Extent;
            uint32_t m_MipLevels;
            uint32_t m_ArrayLayers;
            vk::SampleCountFlagBits m_Samples;
            vk::ImageTiling m_Tiling;
            vk::ImageUsageFlags m_Usage;
            std::vector<uint32_t> m_QueueFamilyIndices;
            vk::ImageLayout m_InitialLayout;
        };

        class VulkanImage
        {
        public:
            using Builder = VulkanImageBuilder;
            static auto New(const VulkanDevice *device, const VulkanImageBuilder &builder) -> std::unique_ptr<VulkanImage>;
            virtual ~VulkanImage() noexcept;

            auto GetDevice() const noexcept -> const VulkanDevice * { return m_Device; }
            auto GetImageVk() const noexcept -> vk::Image { return m_Image.get(); }
            auto GetFlags() const noexcept -> vk::ImageCreateFlags { return m_Flags; }
            auto GetImageType() const noexcept -> vk::ImageType { return m_ImageType; }
            auto GetExtent() const noexcept -> const vk::Extent3D & { return m_Extent; }
            auto GetFormat() const noexcept -> const vk::Format { return m_Format; }
            auto GetMipLevels() const noexcept -> uint32_t { return m_MipLevels; }
            auto GetArrayLayers() const noexcept -> uint32_t { return m_ArrayLayers; }
            auto GetSamples() const noexcept -> vk::SampleCountFlagBits { return m_Samples; }
            auto GetTiling() const noexcept -> vk::ImageTiling { return m_Tiling; }
            auto GetUsage() const noexcept -> vk::ImageUsageFlags { return m_Usage; }
            auto GetQueueFamilyIndices() const noexcept -> std::vector<uint32_t> { return m_QueueFamilyIndices; }
            auto GetInitialLayout() const noexcept -> vk::ImageLayout { return m_InitialLayout; }

        private:
            VulkanImage() noexcept;

        private:
            const VulkanDevice *m_Device;
            vk::UniqueImage m_Image;
            vk::ImageCreateFlags m_Flags;
            vk::ImageType m_ImageType;
            vk::Format m_Format;
            vk::Extent3D m_Extent;
            uint32_t m_MipLevels;
            uint32_t m_ArrayLayers;
            vk::SampleCountFlagBits m_Samples;
            vk::ImageTiling m_Tiling;
            vk::ImageUsageFlags m_Usage;
            std::vector<uint32_t> m_QueueFamilyIndices;
            vk::ImageLayout m_InitialLayout;
        };

        class VulkanDeviceMemory;

        class VulkanDeviceMemoryBuilder
        {
        public:
            VulkanDeviceMemoryBuilder() noexcept
            {
                m_AllocationSize = 0;
                m_MemoryTypeIndex = 0;
                m_MemoryAllocateFlagsInfo = std::nullopt;
                m_MemoryDedicatedAllocateInfo = std::nullopt;
            }

            VulkanDeviceMemoryBuilder(const VulkanDeviceMemoryBuilder &) = default;

            VulkanDeviceMemoryBuilder &operator=(const VulkanDeviceMemoryBuilder &) noexcept = default;

            auto GetAllocationSize() const noexcept -> vk::DeviceSize { return m_AllocationSize; }
            auto SetAllocationSize(vk::DeviceSize allocationSize) noexcept -> VulkanDeviceMemoryBuilder &
            {
                m_AllocationSize = allocationSize;
                return *this;
            }

            auto GetMemoryTypeIndex() const noexcept -> uint32_t { return m_MemoryTypeIndex; }
            auto SetMemoryTypeIndex(uint32_t memoryTypeIndex) noexcept -> VulkanDeviceMemoryBuilder &
            {
                m_MemoryTypeIndex = memoryTypeIndex;
                return *this;
            }

            auto GetMemoryAllocateFlagsInfo() const noexcept -> const std::optional<vk::MemoryAllocateFlagsInfo> &
            {
                return m_MemoryAllocateFlagsInfo;
            }
            auto SetMemoryAllocateFlagsInfo(const vk::MemoryAllocateFlagsInfo &flagsInfo) noexcept -> VulkanDeviceMemoryBuilder &
            {
                m_MemoryAllocateFlagsInfo = flagsInfo;
                return *this;
            }

            auto GetMemoryDedicatedAllocateInfo() const noexcept -> const std::optional<vk::MemoryDedicatedAllocateInfo> &
            {
                return m_MemoryDedicatedAllocateInfo;
            }
            auto SetMemoryDedicatedAllocateInfo(const vk::MemoryDedicatedAllocateInfo &dedicatedAllocation) noexcept -> VulkanDeviceMemoryBuilder &
            {
                m_MemoryDedicatedAllocateInfo = dedicatedAllocation;
                return *this;
            }

            auto Build(const VulkanDevice *device) const -> std::unique_ptr<VulkanDeviceMemory>;

        private:
            vk::DeviceSize m_AllocationSize;
            uint32_t m_MemoryTypeIndex;
            std::optional<vk::MemoryAllocateFlagsInfo> m_MemoryAllocateFlagsInfo;
            std::optional<vk::MemoryDedicatedAllocateInfo> m_MemoryDedicatedAllocateInfo;
        };

        class VulkanDeviceMemory
        {
        public:
            using Builder = VulkanDeviceMemoryBuilder;
            static auto New(const VulkanDevice *device, const VulkanDeviceMemoryBuilder &builder) -> std::unique_ptr<VulkanDeviceMemory>;
            virtual ~VulkanDeviceMemory() noexcept;

            auto Map(void **pPData, vk::MemoryMapFlags flags = {}) const -> vk::Result;
            auto Map(void **pPData, vk::DeviceSize size, vk::DeviceSize offset = 0, vk::MemoryMapFlags flags = {}) const -> vk::Result;
            void Unmap() const;

            auto GetDevice() const -> const VulkanDevice * { return m_Device; }
            auto GetDeviceMemoryVk() const -> vk::DeviceMemory { return m_DeviceMemory.get(); }
            auto GetAllocationSize() const noexcept -> vk::DeviceSize { return m_AllocationSize; }
            auto GetMemoryTypeIndex() const noexcept -> uint32_t { return m_MemoryTypeIndex; }
            auto GetMemoryAllocateFlagsInfo() const noexcept -> const std::optional<vk::MemoryAllocateFlagsInfo> &
            {
                return m_MemoryAllocateFlagsInfo;
            }
            auto GetMemoryDedicatedAllocateInfo() const noexcept -> const std::optional<vk::MemoryDedicatedAllocateInfo> &
            {
                return m_MemoryDedicatedAllocateInfo;
            }

        private:
            VulkanDeviceMemory() noexcept;

        private:
            const VulkanDevice *m_Device;
            vk::UniqueDeviceMemory m_DeviceMemory;
            vk::DeviceSize m_AllocationSize;
            uint32_t m_MemoryTypeIndex;
            std::optional<vk::MemoryAllocateFlagsInfo> m_MemoryAllocateFlagsInfo;
            std::optional<vk::MemoryDedicatedAllocateInfo> m_MemoryDedicatedAllocateInfo;
        };

        class VulkanMemoryBuffer
        {
        public:
            static auto Bind(const VulkanBuffer *buffer, const VulkanDeviceMemory *memory, vk::DeviceSize memoryOffset = 0) -> std::unique_ptr<VulkanMemoryBuffer>;
            virtual ~VulkanMemoryBuffer() noexcept;

            auto Map(void **pPData, vk::MemoryMapFlags flags = {}) const -> vk::Result;
            auto Map(void **pPData, vk::DeviceSize size, vk::DeviceSize offset = 0, vk::MemoryMapFlags flags = {}) const -> vk::Result;
            void Unmap() const;

            auto GetBuffer() const noexcept -> const VulkanBuffer * { return m_Buffer; }
            auto GetBufferVk() const noexcept -> vk::Buffer { return m_Buffer->GetBufferVk(); }
            auto GetBufferSize() const noexcept -> vk::DeviceSize { return m_Buffer->GetSize(); }
            auto GetMemory() const noexcept -> const VulkanDeviceMemory * { return m_Memory; }
            auto GetMemoryVk() const noexcept -> vk::DeviceMemory { return m_Memory->GetDeviceMemoryVk(); }
            auto GetMemoryOffset() const noexcept -> vk::DeviceSize { return m_MemoryOffset; }
            auto GetMemoryAllocationSize() const noexcept -> vk::DeviceSize { return m_Memory->GetAllocationSize(); }
            auto GetDeviceAddress() const noexcept -> std::optional<vk::DeviceAddress> { return m_DeviceAddress; }

        private:
            static bool SupportDeviceAddress(const VulkanBuffer *buffer, const VulkanDeviceMemory *memory) noexcept;
            VulkanMemoryBuffer() noexcept;

        private:
            const VulkanBuffer *m_Buffer;
            const VulkanDeviceMemory *m_Memory;
            vk::DeviceSize m_MemoryOffset;
            std::optional<vk::DeviceSize> m_DeviceAddress;
        };

        class VulkanMemoryImage
        {
        public:
            static auto Bind(const VulkanImage *image, const VulkanDeviceMemory *memory, vk::DeviceSize memoryOffset = 0) -> std::unique_ptr<VulkanMemoryImage>;
            virtual ~VulkanMemoryImage() noexcept;

            auto Map(void **pPData, vk::DeviceSize size, vk::DeviceSize offset = 0, vk::MemoryMapFlags flags = {}) const -> vk::Result;
            void Unmap() const;

            auto GetImage() const noexcept -> const VulkanImage * { return m_Image; }
            auto GetImageVk() const noexcept -> vk::Image { return m_Image->GetImageVk(); }
            auto GetMemory() const noexcept -> const VulkanDeviceMemory * { return m_Memory; }
            auto GetMemoryVk() const noexcept -> vk::DeviceMemory { return m_Memory->GetDeviceMemoryVk(); }
            auto GetMemoryOffset() const noexcept -> vk::DeviceSize { return m_MemoryOffset; }
            auto GetMemoryAllocationSize() const noexcept -> vk::DeviceSize { return m_Memory->GetAllocationSize(); }

        private:
            VulkanMemoryImage() noexcept;

        private:
            const VulkanImage *m_Image;
            const VulkanDeviceMemory *m_Memory;
            vk::DeviceSize m_MemoryOffset;
        };
        class VulkanShaderModule;
        class VulkanShaderModuleBuilder
        {
        public:
            VulkanShaderModuleBuilder() noexcept;
            VulkanShaderModuleBuilder(const VulkanShaderModuleBuilder &) noexcept;
            VulkanShaderModuleBuilder(VulkanShaderModuleBuilder &&) noexcept;
            VulkanShaderModuleBuilder &operator=(const VulkanShaderModuleBuilder &) noexcept;
            VulkanShaderModuleBuilder &operator=(VulkanShaderModuleBuilder &&) noexcept;

            auto GetShaderModuleCreateInfoVk() const noexcept -> vk::ShaderModuleCreateInfo;

            auto SetFlags(vk::ShaderModuleCreateFlags flags) noexcept -> VulkanShaderModuleBuilder &;
            auto GetFlags() const noexcept -> vk::ShaderModuleCreateFlags { return m_Flags; }

            auto SetCodes(const std::vector<uint32_t> &codes) noexcept -> VulkanShaderModuleBuilder &;
            auto GetCodes() const noexcept -> const std::vector<uint32_t> &;
            auto GetPCode() const noexcept -> const uint32_t *;
            auto GetCodeSize() const noexcept -> uint32_t;

            auto Build(const VulkanDevice *device) const -> std::unique_ptr<VulkanShaderModule>;

        private:
            vk::ShaderModuleCreateFlags m_Flags = {};
            std::vector<uint32_t> m_Codes = {};
        };
        class VulkanShaderModule
        {
        public:
            using Builder = VulkanShaderModuleBuilder;
            static auto New(const VulkanDevice *device, const Builder &builder) -> std::unique_ptr<VulkanShaderModule>;
            virtual ~VulkanShaderModule() noexcept;

            auto GetDevice() const noexcept -> const VulkanDevice *;
            auto GetDeviceVk() const noexcept -> vk::Device;

            auto GetShaderModuleVk() const noexcept -> vk::ShaderModule;

            auto GetFlags() const noexcept -> vk::ShaderModuleCreateFlags;
            auto GetCodes() const noexcept -> const std::vector<uint32_t> &;
            auto GetPCode() const noexcept -> const uint32_t *;
            auto GetCodeSize() const noexcept -> uint32_t;

        private:
            VulkanShaderModule() noexcept;

        private:
            const VulkanDevice *m_Device = nullptr;
            vk::UniqueShaderModule m_ShaderModule = {};
            vk::ShaderModuleCreateFlags m_Flags = {};
            std::vector<uint32_t> m_Codes = {};
        };
        class VulkanSpecializationDesc
        {
        public:
            VulkanSpecializationDesc() noexcept;
            VulkanSpecializationDesc(const VulkanSpecializationDesc &) noexcept;
            VulkanSpecializationDesc(VulkanSpecializationDesc &&) noexcept;
            VulkanSpecializationDesc &operator=(const VulkanSpecializationDesc &) noexcept;
            VulkanSpecializationDesc &operator=(VulkanSpecializationDesc &&) noexcept;

            auto GetSpecializationInfoVk() const noexcept -> vk::SpecializationInfo;

            auto SetEntryCount(size_t entryCount) noexcept -> VulkanSpecializationDesc &;
            auto SetEntries(const std::vector<vk::SpecializationMapEntry> &entries) noexcept -> VulkanSpecializationDesc &;
            auto AddEntry(const vk::SpecializationMapEntry &entry) noexcept -> VulkanSpecializationDesc &;
            auto SetEntry(size_t idx, const vk::SpecializationMapEntry &entry) noexcept -> VulkanSpecializationDesc &;
            auto GetEntries() const noexcept -> const std::vector<vk::SpecializationMapEntry> &;
            auto GetEntry(size_t idx) const -> const vk::SpecializationMapEntry &;

            template <typename T>
            auto SetData(const T &t) noexcept -> VulkanSpecializationDesc &
            {
                m_Data.resize(sizeof(T));
                std::memcpy(m_Data.data(), &t, sizeof(T));
                return *this;
            }
            template <typename T>
            auto GetData() const noexcept -> std::optional<T>
            {
                if (sizeof(T) == m_Data.size())
                {
                    T t;
                    std::memcpy(&t, m_Data.data(), sizeof(T));
                    return t;
                }
                else
                {
                    return std::nullopt;
                }
            }
            auto GetDataSize() const noexcept -> size_t { return m_Data.size(); }

        private:
            std::vector<vk::SpecializationMapEntry> m_Entries;
            std::vector<char> m_Data;
        };

        class VulkanPipelineShaderStageDesc
        {
        public:
            VulkanPipelineShaderStageDesc() noexcept;
            VulkanPipelineShaderStageDesc(const VulkanPipelineShaderStageDesc &) noexcept;
            VulkanPipelineShaderStageDesc(VulkanPipelineShaderStageDesc &&) noexcept;
            VulkanPipelineShaderStageDesc &operator=(const VulkanPipelineShaderStageDesc &) noexcept;
            VulkanPipelineShaderStageDesc &operator=(VulkanPipelineShaderStageDesc &&) noexcept;

            auto GetPipelineShaderStageCreateInfoVk() const noexcept -> vk::PipelineShaderStageCreateInfo;
            auto GetSpecializationInfoVk() const noexcept -> std::optional<vk::SpecializationInfo>;
            auto GetShaderModuleCreateInfoVk() const noexcept -> std::optional<vk::ShaderModuleCreateInfo>;

            auto SetFlags(vk::PipelineShaderStageCreateFlags flags) noexcept -> VulkanPipelineShaderStageDesc &;
            auto GetFlags() const noexcept -> vk::PipelineShaderStageCreateFlags;

            auto SetStage(vk::ShaderStageFlagBits stage) noexcept -> VulkanPipelineShaderStageDesc &;
            auto GetStage() const noexcept -> vk::ShaderStageFlagBits;

            auto SetModule(const VulkanShaderModule *module) noexcept -> VulkanPipelineShaderStageDesc &;
            auto GetModule() const noexcept -> const VulkanShaderModule *;
            auto GetModuleVk() const noexcept -> vk::ShaderModule;

            auto SetName(const std::string &name) noexcept -> VulkanPipelineShaderStageDesc &;
            auto GetName() const noexcept -> std::string;

            auto SetSpecializationDesc(const VulkanSpecializationDesc &specialization) noexcept -> VulkanPipelineShaderStageDesc &;
            auto GetSpecializationDesc() const noexcept -> const std::optional<VulkanSpecializationDesc> &;

            auto SetShaderModuleBuilder(const VulkanShaderModuleBuilder &builder) noexcept -> VulkanPipelineShaderStageDesc &;
            auto GetShaderModuleBuilder() const noexcept -> const std::optional<VulkanShaderModuleBuilder> &;

        private:
            vk::PipelineShaderStageCreateFlags m_Flags = {};
            vk::ShaderStageFlagBits m_Stage = {};
            const VulkanShaderModule *m_Module = nullptr;
            std::string m_Name = "";
            std::optional<VulkanSpecializationDesc> m_SpecializationDesc = std::nullopt;
            std::optional<VulkanShaderModuleBuilder> m_ModuleBuilder = std::nullopt;
        };
        class VulkanPipelineVertexInputStateDesc
        {
        public:
            VulkanPipelineVertexInputStateDesc() noexcept;

            VulkanPipelineVertexInputStateDesc(const VulkanPipelineVertexInputStateDesc &) noexcept;
            VulkanPipelineVertexInputStateDesc(VulkanPipelineVertexInputStateDesc &&) noexcept;
            VulkanPipelineVertexInputStateDesc &operator=(const VulkanPipelineVertexInputStateDesc &) noexcept;
            VulkanPipelineVertexInputStateDesc &operator=(VulkanPipelineVertexInputStateDesc &&) noexcept;

            auto SetVertexBindingDescriptions(const std::vector<vk::VertexInputBindingDescription> &bindingDescs) noexcept -> VulkanPipelineVertexInputStateDesc &;
            auto AddVertexBindingDescription(const vk::VertexInputBindingDescription &bindingDesc) noexcept -> VulkanPipelineVertexInputStateDesc &;
            auto SetVertexBindingDescription(size_t idx, const vk::VertexInputBindingDescription &bindingDesc) noexcept -> VulkanPipelineVertexInputStateDesc &;
            auto GetVertexBindingDescriptions() const noexcept -> const std::vector<vk::VertexInputBindingDescription> &;
            auto GetVertexBindingDescription(size_t idx) const -> const vk::VertexInputBindingDescription &;

            auto SetVertexAttributeDescriptions(const std::vector<vk::VertexInputAttributeDescription> &attributeDescs) noexcept -> VulkanPipelineVertexInputStateDesc &;
            auto AddVertexAttributeDescription(const vk::VertexInputAttributeDescription &attributeDesc) noexcept -> VulkanPipelineVertexInputStateDesc &;
            auto SetVertexAttributeDescription(size_t idx, const vk::VertexInputAttributeDescription &attributeDesc) noexcept -> VulkanPipelineVertexInputStateDesc &;
            auto GetVertexAttributeDescriptions() const noexcept -> const std::vector<vk::VertexInputAttributeDescription> &;
            auto GetVertexAttributeDescription(size_t idx) const -> const vk::VertexInputAttributeDescription &;

            auto SetVertexBindingDivisors(const std::vector<vk::VertexInputBindingDivisorDescriptionEXT> &divisors) noexcept -> VulkanPipelineVertexInputStateDesc &;
            auto AddVertexBindingDivisors(const vk::VertexInputBindingDivisorDescriptionEXT &divisor) const noexcept -> VulkanPipelineVertexInputStateDesc &;
            auto SetVertexBindingDivisor(size_t idx, const vk::VertexInputBindingDivisorDescriptionEXT &divisor) noexcept -> VulkanPipelineVertexInputStateDesc &;
            auto GetVertexBindingDivisors() const noexcept -> const std::vector<vk::VertexInputBindingDivisorDescriptionEXT> &;
            auto GetVertexBindingDivisor(size_t idx) const -> const vk::VertexInputBindingDivisorDescriptionEXT &;

            auto GetVulkanPipelineVertexInputStateCreateInfoVk() const noexcept -> vk::PipelineVertexInputStateCreateInfo;
            auto GetPipelineVertexInputDivisorStateCreateInfoEXT() const noexcept -> std::optional<vk::PipelineVertexInputDivisorStateCreateInfoEXT>;

        private:
            vk::PipelineVertexInputStateCreateFlags m_Flags = {};
            std::vector<vk::VertexInputBindingDescription> m_VertexBindingDescriptions = {};
            std::vector<vk::VertexInputAttributeDescription> m_VertexAttributeDescriptions = {};
            std::vector<vk::VertexInputBindingDivisorDescriptionEXT> m_VertexBindingDivisors = {};
        };
        class VulkanPipelineTessellationStateDesc
        {
        public:
            VulkanPipelineTessellationStateDesc() noexcept;
            VulkanPipelineTessellationStateDesc(const VulkanPipelineTessellationStateDesc &) noexcept;
            VulkanPipelineTessellationStateDesc(VulkanPipelineTessellationStateDesc &&) noexcept;
            VulkanPipelineTessellationStateDesc &operator=(const VulkanPipelineTessellationStateDesc &) noexcept;
            VulkanPipelineTessellationStateDesc &operator=(VulkanPipelineTessellationStateDesc &&) noexcept;

            auto SetFlags(vk::PipelineTessellationStateCreateFlags flags) noexcept -> VulkanPipelineTessellationStateDesc &;
            auto GetFlags() const noexcept -> vk::PipelineTessellationStateCreateFlags;

            auto SetPatchControlPoints(uint32_t patchControlPoints) noexcept -> VulkanPipelineTessellationStateDesc &;
            auto GetPatchControlPoints() const noexcept -> uint32_t;

            auto SetDomainOrigin(const vk::TessellationDomainOrigin &origin) noexcept -> VulkanPipelineTessellationStateDesc &;
            auto GetDomainOrigin() const noexcept -> const std::optional<vk::TessellationDomainOrigin> &;

            auto GetPipelineTessellationStateInfoVk() const noexcept -> vk::PipelineTessellationStateCreateInfo;

        private:
            vk::PipelineTessellationStateCreateFlags m_Flags = {};
            uint32_t m_PatchControlPoints = 0;
            std::optional<vk::TessellationDomainOrigin> m_DomainOrigin = std::nullopt;
        };
        class VulkanPipelineViewportStateDesc
        {
        public:
            VulkanPipelineViewportStateDesc() noexcept;
            VulkanPipelineViewportStateDesc(const VulkanPipelineViewportStateDesc &) noexcept;
            VulkanPipelineViewportStateDesc(VulkanPipelineViewportStateDesc &&) noexcept;
            VulkanPipelineViewportStateDesc &operator=(const VulkanPipelineViewportStateDesc &) noexcept;
            VulkanPipelineViewportStateDesc &operator=(VulkanPipelineViewportStateDesc &&) noexcept;

            auto SetFlags(vk::PipelineViewportStateCreateFlags flags) noexcept -> VulkanPipelineViewportStateDesc &;
            auto GetFlags() const noexcept -> vk::PipelineViewportStateCreateFlags;

            auto SetViewportCount(uint32_t viewportCount) noexcept -> VulkanPipelineViewportStateDesc &;
            auto SetViewports(const std::vector<vk::Viewport> &viewports) noexcept -> VulkanPipelineViewportStateDesc &;
            auto AddViewport(const vk::Viewport &viewport) noexcept -> VulkanPipelineViewportStateDesc &;
            auto SetViewport(size_t idx, const vk::Viewport &viewport) noexcept -> VulkanPipelineViewportStateDesc &;
            auto GetViewports() const noexcept -> const std::vector<vk::Viewport> &;
            auto GetViewport(size_t idx) const -> const vk::Viewport &;

            auto SetScissorCount(uint32_t scissorCount) noexcept -> VulkanPipelineViewportStateDesc &;
            auto SetScissors(const std::vector<vk::Rect2D> &scissors) noexcept -> VulkanPipelineViewportStateDesc &;
            auto AddScissor(const vk::Rect2D &scissor) noexcept -> VulkanPipelineViewportStateDesc &;
            auto SetScissor(size_t idx, const vk::Rect2D &scissor) noexcept -> VulkanPipelineViewportStateDesc &;
            auto GetScissors() const noexcept -> const std::vector<vk::Rect2D> &;
            auto GetScissor(size_t idx) const -> const vk::Rect2D &;

            auto SetNegativeOneToOne(vk::Bool32 negativeOneToOne) noexcept -> VulkanPipelineViewportStateDesc &;
            auto GetNegativeOneToOne() const noexcept -> std::optional<vk::Bool32>;

            auto GetPipelineViewportStateCreateInfoVk() const noexcept -> vk::PipelineViewportStateCreateInfo;

        private:
            vk::PipelineViewportStateCreateFlags m_Flags = {};
            std::vector<vk::Viewport> m_Viewports = {};
            std::vector<vk::Rect2D> m_Scissors = {};
            std::optional<vk::Bool32> m_NegativeOneToOne = std::nullopt;
        };
        class VulkanPipelineRasterizationStateDesc
        {
        public:
            VulkanPipelineRasterizationStateDesc() noexcept;
            VulkanPipelineRasterizationStateDesc(const VulkanPipelineRasterizationStateDesc &) noexcept;
            VulkanPipelineRasterizationStateDesc(VulkanPipelineRasterizationStateDesc &&) noexcept;
            VulkanPipelineRasterizationStateDesc &operator=(const VulkanPipelineRasterizationStateDesc &) noexcept;
            VulkanPipelineRasterizationStateDesc &operator=(VulkanPipelineRasterizationStateDesc &&) noexcept;

            auto GetFlags() const noexcept -> vk::PipelineRasterizationStateCreateFlags;
            auto SetFlags(vk::PipelineRasterizationStateCreateFlags flags) noexcept -> VulkanPipelineRasterizationStateDesc &;

            auto SetDepthClampEnable(vk::Bool32 depthClampEnable) noexcept -> VulkanPipelineRasterizationStateDesc &;
            auto GetDepthClampEnable() const noexcept -> vk::Bool32;

            auto SetRasterizerDiscardEnable(vk::Bool32 rasterizerDiscardEnable) noexcept -> VulkanPipelineRasterizationStateDesc &;
            auto GetRasterizerDiscardEnable() const noexcept -> vk::Bool32;

            auto SetPolygonMode(vk::PolygonMode polygonMode) noexcept -> VulkanPipelineRasterizationStateDesc &;
            auto GetPolygonMode() const noexcept -> vk::PolygonMode;

            auto GetCullMode() const noexcept -> vk::CullModeFlags;
            auto SetCullMode(vk::CullModeFlags cullMode) noexcept -> VulkanPipelineRasterizationStateDesc &;

            auto GetFrontFace() const noexcept -> vk::FrontFace;
            auto SetFrontFace(vk::FrontFace frontFace) noexcept -> VulkanPipelineRasterizationStateDesc &;

            auto SetDepthBiasEnable(vk::Bool32 depthBiasEnable) noexcept -> VulkanPipelineRasterizationStateDesc &;
            auto GetDepthBiasEnable() const noexcept -> vk::Bool32;

            auto SetDepthBiasConstantFactor(float depthBiasConstantFactor) noexcept -> VulkanPipelineRasterizationStateDesc &;
            auto GetDepthBiasConstantFactor() const noexcept -> float;

            auto SetDepthBiasClamp(float depthBiasClamp) noexcept -> VulkanPipelineRasterizationStateDesc &;
            auto GetDepthBiasClamp() const noexcept -> float;

            auto SetDepthBiasSlopeFactor(float depthBiasSlopeFactor) noexcept -> VulkanPipelineRasterizationStateDesc &;
            auto GetDepthBiasSlopeFactor() const noexcept -> float;

            auto SetLineWidth(float lineWidth) noexcept -> VulkanPipelineRasterizationStateDesc &;
            auto GetLineWidth() const noexcept -> float;

        private:
            vk::PipelineRasterizationStateCreateFlags m_Flags = {};
            vk::Bool32 m_DepthClampEnable;
            vk::Bool32 m_RasterizerDiscardEnable;
            vk::PolygonMode m_PolygonMode;
            vk::CullModeFlags m_CullMode;
            vk::FrontFace m_FrontFace;
            vk::Bool32 m_DepthBiasEnable;
            float m_DepthBiasConstantFactor;
            float m_DepthBiasClamp;
            float m_DepthBiasSlopeFactor;
            float m_LineWidth;
        };
        class VulkanPipelineMultiSampleStateDesc
        {
        public:
            VulkanPipelineMultiSampleStateDesc() noexcept;
            VulkanPipelineMultiSampleStateDesc(const VulkanPipelineMultiSampleStateDesc &) noexcept;
            VulkanPipelineMultiSampleStateDesc(VulkanPipelineMultiSampleStateDesc &&) noexcept;
            VulkanPipelineMultiSampleStateDesc &operator=(const VulkanPipelineMultiSampleStateDesc &) noexcept;
            VulkanPipelineMultiSampleStateDesc &operator=(VulkanPipelineMultiSampleStateDesc &&) noexcept;

            auto GetFlags() const noexcept -> vk::PipelineMultisampleStateCreateFlags;
            auto SetFlags(vk::PipelineMultisampleStateCreateFlags flags) noexcept -> VulkanPipelineMultiSampleStateDesc &;

            auto GetRasterizationSamples() const noexcept -> vk::SampleCountFlagBits;
            auto SetRasterizationSamples(vk::SampleCountFlagBits rasterizationSamples) noexcept -> VulkanPipelineMultiSampleStateDesc &;

            auto GetSampleShadingEnable() const noexcept -> vk::Bool32;
            auto SetSampleShadingEnable(vk::Bool32 sampleShadingEnable) noexcept -> VulkanPipelineMultiSampleStateDesc &;

            auto GetMinSampleShading() const noexcept -> float;
            auto SetMinSampleShading(float minSampleShading) noexcept -> VulkanPipelineMultiSampleStateDesc &;

            auto GetAlphaToCoverageEnable() const noexcept -> vk::Bool32;
            auto SetAlphaToCoverageEnable(vk::Bool32 alphaToCoverageEnable) noexcept -> VulkanPipelineMultiSampleStateDesc &;

            auto GetAlphaToOneEnable() const noexcept -> vk::Bool32;
            auto SetAlphaToOneEnable(vk::Bool32 alphaToOneEnable) noexcept -> VulkanPipelineMultiSampleStateDesc &;

        private:
            vk::PipelineMultisampleStateCreateFlags m_Flags = {};
            vk::SampleCountFlagBits m_RasterizationSamples = vk::SampleCountFlagBits::e1;
            vk::Bool32 m_SampleShadingEnable = VK_FALSE;
            float m_MinSampleShading = 0.0f;
            std::vector<vk::SampleMask> m_SampleMasks = {};
            vk::Bool32 m_AlphaToCoverageEnable = VK_FALSE;
            vk::Bool32 m_AlphaToOneEnable = VK_FALSE;
        };
        class VulkanPipelineColorBlendStateDesc
        {
        public:
            VulkanPipelineColorBlendStateDesc() noexcept;
            VulkanPipelineColorBlendStateDesc(const VulkanPipelineColorBlendStateDesc &) noexcept;
            VulkanPipelineColorBlendStateDesc(VulkanPipelineColorBlendStateDesc &&) noexcept;
            VulkanPipelineColorBlendStateDesc &operator=(const VulkanPipelineColorBlendStateDesc &) noexcept;
            VulkanPipelineColorBlendStateDesc &operator=(VulkanPipelineColorBlendStateDesc &&) noexcept;

            auto GetFlags() const noexcept -> vk::PipelineColorBlendStateCreateFlags;
            auto SetFlags(vk::PipelineColorBlendStateCreateFlags flags) noexcept -> VulkanPipelineColorBlendStateDesc &;

            auto SetLogicOpEnable(vk::Bool32 logicOpEnable) noexcept -> VulkanPipelineColorBlendStateDesc &;
            auto GetLogicOpEnable() const noexcept -> vk::Bool32;

            auto SetAttachmentCount(uint32_t attachmentCount) noexcept -> VulkanPipelineColorBlendStateDesc &;
            auto GetAttachmentCount() const noexcept -> uint32_t;

            auto AddAttachment(const vk::PipelineColorBlendAttachmentState &attachment) noexcept -> VulkanPipelineColorBlendStateDesc &;
            auto SetAttachment(size_t idx, const vk::PipelineColorBlendAttachmentState &attachment) noexcept -> VulkanPipelineColorBlendStateDesc &;
            auto GetAttachment(size_t idx) const -> const vk::PipelineColorBlendAttachmentState &;

            auto SetAttachments(const std::vector<vk::PipelineColorBlendAttachmentState> &attachments) noexcept -> VulkanPipelineColorBlendStateDesc &;
            auto GetAttachments() const noexcept -> const std::vector<vk::PipelineColorBlendAttachmentState> &;

            auto SetBlendConstants(const std::array<float, 4> &blendConstants) noexcept -> VulkanPipelineColorBlendStateDesc &;
            auto GetBlendConstants() const noexcept -> std::array<float, 4>;
            auto SetBlendConstant(size_t idx, float blendConstant) noexcept -> VulkanPipelineColorBlendStateDesc &;
            auto GetBlendConstant(size_t idx) const -> float;

        private:
            vk::PipelineColorBlendStateCreateFlags m_Flags = {};
            vk::Bool32 m_LogicOpEnable = VK_FALSE;
            vk::LogicOp m_LogicOp = {};
            std::vector<vk::PipelineColorBlendAttachmentState> m_Attachments = {};
            float m_BlendConstants[4] = {};
        };
        class VulkanPipelineLayout;
        class VulkanRenderPass;
        class VulkanGraphicsPipeline;
        class VulkanGraphicsPipelineBuilder
        {
        public:
            VulkanGraphicsPipelineBuilder() noexcept;
            VulkanGraphicsPipelineBuilder(const VulkanGraphicsPipelineBuilder &) noexcept;
            VulkanGraphicsPipelineBuilder(VulkanGraphicsPipelineBuilder &&) noexcept;
            VulkanGraphicsPipelineBuilder &operator=(const VulkanGraphicsPipelineBuilder &) noexcept;
            VulkanGraphicsPipelineBuilder &operator=(VulkanGraphicsPipelineBuilder &&) noexcept;

            auto SetFlags(vk::PipelineCreateFlags flags) noexcept -> VulkanGraphicsPipelineBuilder &;
            auto GetFlags() const noexcept -> vk::PipelineCreateFlags;

            auto SetStages(const std::vector<VulkanPipelineShaderStageDesc> &stages) noexcept -> VulkanGraphicsPipelineBuilder &;
            auto AddStage(const VulkanPipelineShaderStageDesc &stage) noexcept -> VulkanGraphicsPipelineBuilder &;
            auto SetStage(size_t idx, const VulkanPipelineShaderStageDesc &stage) noexcept -> VulkanGraphicsPipelineBuilder &;
            auto GetStages() const noexcept -> std::vector<VulkanPipelineShaderStageDesc>;
            auto GetStage(size_t idx) const -> const VulkanPipelineShaderStageDesc &;

            auto SetVertexInputState(const VulkanPipelineVertexInputStateDesc &vertexInput) noexcept -> VulkanGraphicsPipelineBuilder &;
            auto GetVertexInputState() const noexcept -> const std::optional<VulkanPipelineVertexInputStateDesc> &;

            auto SetInputAssemblyState(const vk::PipelineInputAssemblyStateCreateInfo &inputAssembly) noexcept -> VulkanGraphicsPipelineBuilder &;
            auto GetInputAssemblyState() const noexcept -> const std::optional<vk::PipelineInputAssemblyStateCreateInfo> &;

            auto SetTessellationState(const VulkanPipelineTessellationStateDesc &tessellation) noexcept -> VulkanGraphicsPipelineBuilder &;
            auto GetTessellationState() const noexcept -> const std::optional<VulkanPipelineTessellationStateDesc> &;

            auto SetViewportState(const VulkanPipelineViewportStateDesc &viewport) noexcept -> VulkanGraphicsPipelineBuilder &;
            auto GetViewportState() const noexcept -> const std::optional<VulkanPipelineViewportStateDesc> &;

            auto SetRasterizationState(const VulkanPipelineRasterizationStateDesc &rasterization) noexcept -> VulkanGraphicsPipelineBuilder &;
            auto GetRasterizationState() const noexcept -> const std::optional<VulkanPipelineRasterizationStateDesc> &;

            auto SetMultiSampleState(const VulkanPipelineMultiSampleStateDesc &multiSample) noexcept -> VulkanGraphicsPipelineBuilder &;
            auto GetMultiSampleState() const noexcept -> const std::optional<VulkanPipelineMultiSampleStateDesc> &;

            auto GetDepthStencilState(const vk::PipelineDepthStencilStateCreateInfo &depthStencil) noexcept -> VulkanGraphicsPipelineBuilder &;
            auto SetDepthStencilState() const noexcept -> const std::optional<vk::PipelineDepthStencilStateCreateInfo> &;

            auto SetColorBlendState(const VulkanPipelineColorBlendStateDesc &colorBlendState) noexcept -> VulkanGraphicsPipelineBuilder &;
            auto GetColorBlendState() const noexcept -> const std::optional<VulkanPipelineColorBlendStateDesc> &;

            auto SetLayout(const VulkanPipelineLayout *layout) noexcept -> VulkanGraphicsPipelineBuilder &;
            auto GetLayout() const noexcept -> const VulkanPipelineLayout *;

            auto SetRenderPass(const VulkanRenderPass *renderPass) noexcept -> VulkanGraphicsPipelineBuilder &;
            auto GetRenderPass() const noexcept -> const VulkanRenderPass *;

            auto SetSubpass(uint32_t subpass) noexcept -> VulkanGraphicsPipelineBuilder &;
            auto GetSubpass() const noexcept -> std::optional<uint32_t>;

            auto SetBasePipelineHandle(const VulkanGraphicsPipeline *basePipeline) noexcept -> VulkanGraphicsPipelineBuilder &;
            auto GetBasePipelineHandle() const noexcept -> const VulkanGraphicsPipeline *;

            auto SetBasePipelineIndex(uint32_t basePipelineIndex) noexcept -> VulkanGraphicsPipelineBuilder &;
            auto GetBasePipelineIndex() const noexcept -> std::optional<uint32_t>;

        private:
            vk::PipelineCreateFlags m_Flags = {};
            std::vector<VulkanPipelineShaderStageDesc> m_Stages = {};
            std::optional<VulkanPipelineVertexInputStateDesc> m_VertexInputState = std::nullopt;
            std::optional<vk::PipelineInputAssemblyStateCreateInfo> m_InputAssemblyState = std::nullopt;
            std::optional<VulkanPipelineTessellationStateDesc> m_TessellationState = std::nullopt;
            std::optional<VulkanPipelineViewportStateDesc> m_ViewportState = std::nullopt;
            std::optional<VulkanPipelineRasterizationStateDesc> m_RasterizationState = std::nullopt;
            std::optional<VulkanPipelineMultiSampleStateDesc> m_MultiSampleState = std::nullopt;
            std::optional<vk::PipelineDepthStencilStateCreateInfo> m_DepthStencilState = std::nullopt;
            std::optional<VulkanPipelineColorBlendStateDesc> m_ColorBlendState = std::nullopt;
            std::unordered_set<vk::DynamicState> m_DynamicStates = {};
            const VulkanPipelineLayout *m_Layout = nullptr;
            const VulkanRenderPass *m_RenderPass = nullptr;
            std::optional<uint32_t> m_Subpass = std::nullopt;
            const VulkanGraphicsPipeline *m_BasePipelineHandle = nullptr;
            std::optional<uint32_t> m_BasePipelineIndex = std::nullopt;
        };
        class VulkanGraphicsPipeline
        {
        public:
        private:
            vk::UniquePipeline m_Pipeline;
            VulkanGraphicsPipelineBuilder m_Builder;
        };
        class VulkanSubpassDesc
        {
        public:
            VulkanSubpassDesc() noexcept;
            VulkanSubpassDesc(const VulkanSubpassDesc &) noexcept;
            VulkanSubpassDesc(VulkanSubpassDesc &&) noexcept;
            VulkanSubpassDesc &operator=(const VulkanSubpassDesc &) noexcept;
            VulkanSubpassDesc &operator=(VulkanSubpassDesc &&) noexcept;

            auto GetFlags() const noexcept -> vk::SubpassDescriptionFlags;
            auto SetFlags(vk::SubpassDescriptionFlags flags) noexcept -> VulkanSubpassDesc &;

            auto SetPipelineBindPoint(vk::PipelineBindPoint bindPipelineBindPoint) noexcept -> VulkanSubpassDesc &;
            auto GetPipelineBindPoint() const noexcept -> vk::PipelineBindPoint;

            auto SetInputAttachmentCount(uint32_t inputAttachmentCount) noexcept -> VulkanSubpassDesc &;
            auto GetInputAttachmentCount() const noexcept -> uint32_t;

            auto GetInputAttachments() const noexcept -> const std::vector<vk::AttachmentReference> &;
            auto SetInputAttachments(const std::vector<vk::AttachmentReference> &attachments) noexcept -> VulkanSubpassDesc &;

            auto AddInputAttachment(const vk::AttachmentReference &attachmentReference) noexcept -> VulkanSubpassDesc &;
            auto SetInputAttachment(size_t idx, const vk::AttachmentReference &attachmentReference) noexcept -> VulkanSubpassDesc &;
            auto GetInputAttachment(size_t idx) const noexcept -> std::optional<vk::AttachmentReference>;

            auto SetColorAttachmentCount(uint32_t colorAttachmentCount) noexcept -> VulkanSubpassDesc &;
            auto GetColorAttachmentCount() const noexcept -> uint32_t;
            
            auto AddColorAttachment (const vk::AttachmentReference& reference)noexcept ->VulkanSubpassDesc &;
            auto SetColorAttachment (size_t idx, const vk::AttachmentReference& reference)noexcept ->VulkanSubpassDesc &;
            auto GetColorAttachment (size_t idx) const noexcept -> std::optional<vk::AttachmentReference>;
            
            auto GetColorAttachments() const noexcept -> const std::vector<vk::AttachmentReference> &;
            auto SetColorAttachments(const std::vector<vk::AttachmentReference> &attachments) noexcept -> VulkanSubpassDesc &;

            auto SetResolveAttachments(const std::vector<vk::AttachmentReference> &attachments) noexcept -> VulkanSubpassDesc &;
            auto GetResolveAttachments() const noexcept -> const std::vector<vk::AttachmentReference> &;

            auto SetResolveEnable(bool resolveEnable) noexcept -> VulkanSubpassDesc &;
            auto GetResolveEnable() const noexcept -> bool;

            auto SetDepthStencilAttachment(const vk::AttachmentReference &attachment) noexcept -> VulkanSubpassDesc &;
            auto GetDepthStencilAttachment() const noexcept -> std::optional<vk::AttachmentReference>;

            auto SetPreserveAttachmentCount(uint32_t attachmentCount) noexcept -> VulkanSubpassDesc &;
            auto GetPreserveAttachmentCount() const noexcept -> uint32_t;

            auto SetPreserveAttachments(const std::vector<uint32_t> &preserveAttachments) noexcept -> VulkanSubpassDesc &;
            auto GetPreserveAttachments() const noexcept -> const std::vector<uint32_t> &;

            auto AddPreserveAttachment(uint32_t preserveAttachment) noexcept -> VulkanSubpassDesc &;
            auto SetPreserveAttachment(uint32_t idx, uint32_t preserveAttachment) noexcept -> VulkanSubpassDesc &;
            auto GetPreserveAttachment(uint32_t idx) const noexcept -> std::optional<uint32_t>;
            
            auto GetSubpassDescriptionVk()const noexcept->vk::SubpassDescription;
        private:
            vk::SubpassDescriptionFlags m_Flags;
            vk::PipelineBindPoint m_PipelineBindPoint;
            std::vector<vk::AttachmentReference> m_InputAttachments;
            std::vector<vk::AttachmentReference> m_ColorAttachments;
            std::vector<vk::AttachmentReference> m_ResolveAttachments;
            std::optional<vk::AttachmentReference> m_DepthStencilAttachment;
            std::vector<uint32_t> m_PreserveAttachments;
        };
        class VulkanRenderPassBuilder
        {
        public:
            VulkanRenderPassBuilder()noexcept;
            VulkanRenderPassBuilder(const VulkanRenderPassBuilder& )noexcept;
            VulkanRenderPassBuilder(      VulkanRenderPassBuilder&&)noexcept;
            VulkanRenderPassBuilder& operator=(const VulkanRenderPassBuilder& )noexcept;
            VulkanRenderPassBuilder& operator=(      VulkanRenderPassBuilder&&)noexcept;
            
            auto Build(const VulkanDevice* device)const -> std::unique_ptr<BulletRT::Core::VulkanRenderPass>;
            auto GetSubpassDescriptionVks()const noexcept -> std::vector<vk::SubpassDescription>;
            
            auto SetFlags(vk::RenderPassCreateFlags flags) noexcept -> VulkanRenderPassBuilder &;
            auto GetFlags() const noexcept -> vk::RenderPassCreateFlags;

            auto SetAttachmentCount(uint32_t attachmentCount) noexcept -> VulkanRenderPassBuilder &;
            auto GetAttachmentCount() const noexcept -> uint32_t;

            auto SetAttachments(const std::vector<vk::AttachmentDescription> &attachments) noexcept -> VulkanRenderPassBuilder &;
            auto GetAttachments() const noexcept -> const std::vector<vk::AttachmentDescription> &;

            auto AddAttachment(const vk::AttachmentDescription &attachment) noexcept -> VulkanRenderPassBuilder &;
            auto SetAttachment(size_t idx, const vk::AttachmentDescription &attachment) noexcept -> VulkanRenderPassBuilder &;
            auto GetAttachment(size_t idx) const -> std::optional<vk::AttachmentDescription>;
            
            auto SetSubpassCount(uint32_t subpassCount) noexcept ->  VulkanRenderPassBuilder &;
            auto GetSubpassCount()const noexcept -> uint32_t;
            
            auto SetSubpasses(const std::vector<VulkanSubpassDesc>& subpasses)noexcept -> VulkanRenderPassBuilder &;
            auto GetSubpasses()const noexcept -> const std::vector<VulkanSubpassDesc>& ;
            
            auto AddSubpass(const VulkanSubpassDesc& subpass)noexcept ->  VulkanRenderPassBuilder &;
            auto SetSubpass(size_t idx, const VulkanSubpassDesc& subpass)noexcept -> VulkanRenderPassBuilder &;
            auto GetSubpass(size_t idx)const noexcept -> std::optional<VulkanSubpassDesc>;
            
            auto SetDependencyCount(uint32_t dependencyCount)noexcept -> VulkanRenderPassBuilder &;
            auto GetDependencyCount()const noexcept -> uint32_t;
            
            auto SetDependencies(const std::vector<vk::SubpassDependency>& dependencies)noexcept -> VulkanRenderPassBuilder &;
            auto GetDependencies()const noexcept -> const std::vector<vk::SubpassDependency>& ;
            
            auto AddDependency(const vk::SubpassDependency& dependency)noexcept -> VulkanRenderPassBuilder &;
            auto SetDependency(size_t idx, const vk::SubpassDependency& dependency)noexcept -> VulkanRenderPassBuilder &;
            auto GetDependency(size_t idx)const noexcept -> std::optional<vk::SubpassDependency>;
            
            auto GetSrcSubpass(size_t subpassDependIdx) const -> std::optional<VulkanSubpassDesc>;
            auto GetDstSubpass(size_t subpassDependIdx) const -> std::optional<VulkanSubpassDesc>;

            auto GetInputAttachments(size_t subpassIdx) const noexcept -> std::vector<vk::AttachmentDescription>;
            auto GetColorAttachments(size_t subpassIdx) const noexcept -> std::vector<vk::AttachmentDescription>;
            auto GetResolveAttachments(size_t subpassIdx) const noexcept -> std::vector<vk::AttachmentDescription>;
            auto GetDepthStencilAttachment(size_t subpassIdx) const noexcept -> std::optional<vk::AttachmentDescription>;
            auto GetPreserveAttachments(size_t subpassIdx) const noexcept -> std::vector<vk::AttachmentDescription>;

        private:
            vk::RenderPassCreateFlags m_Flags = {};
            std::vector<vk::AttachmentDescription> m_Attachments = {};
            std::vector<VulkanSubpassDesc> m_Subpasses = {};
            std::vector<vk::SubpassDependency> m_Dependencies = {};
        };
        class VulkanRenderPass
        {
        public:
            using Builder = VulkanRenderPassBuilder;
            static auto New(const VulkanDevice* device,const VulkanRenderPassBuilder& builder)->std::unique_ptr<VulkanRenderPass>;
            virtual ~VulkanRenderPass()noexcept;
            
            auto GetDevice()const noexcept -> const VulkanDevice*;
            auto GetDeviceVk()const noexcept -> vk::Device;
            
            auto GetRenderPassVk()const noexcept -> vk::RenderPass;
            
            auto GetFlags() const noexcept -> vk::RenderPassCreateFlags;

            auto GetAttachmentCount() const noexcept -> uint32_t;
            
            auto GetAttachments() const noexcept -> const std::vector<vk::AttachmentDescription> &;

            auto GetAttachment(size_t idx) const -> std::optional<vk::AttachmentDescription>;
            
            auto GetSubpassCount()const noexcept -> uint32_t;
            
            auto GetSubpasses()const noexcept -> const std::vector<VulkanSubpassDesc>& ;
            
            auto GetSubpass(size_t idx)const noexcept -> std::optional<VulkanSubpassDesc>;
            
            auto GetDependencyCount()const noexcept -> uint32_t;
            
            auto GetDependencies()const noexcept -> const std::vector<vk::SubpassDependency>& ;
            
            auto GetDependency(size_t idx)const noexcept -> std::optional<vk::SubpassDependency>;
            
            auto GetSrcSubpass(size_t subpassDependIdx) const -> std::optional<VulkanSubpassDesc>;
            auto GetDstSubpass(size_t subpassDependIdx) const -> std::optional<VulkanSubpassDesc>;

            auto GetInputAttachments(size_t subpassIdx) const noexcept -> std::vector<vk::AttachmentDescription>;
            auto GetColorAttachments(size_t subpassIdx) const noexcept -> std::vector<vk::AttachmentDescription>;
            auto GetResolveAttachments(size_t subpassIdx) const noexcept -> std::vector<vk::AttachmentDescription>;
            auto GetDepthStencilAttachment(size_t subpassIdx) const noexcept -> std::optional<vk::AttachmentDescription>;
            auto GetPreserveAttachments(size_t subpassIdx) const noexcept -> std::vector<vk::AttachmentDescription>;
            
        private:
            VulkanRenderPass()noexcept;
        private:
            const VulkanDevice*  m_Device     = nullptr;
            vk::UniqueRenderPass m_RenderPass = {};
            vk::RenderPassCreateFlags m_Flags = {};
            std::vector<vk::AttachmentDescription> m_Attachments = {};
            std::vector<VulkanSubpassDesc> m_Subpasses = {};
            std::vector<vk::SubpassDependency> m_Dependencies = {};
        };
    }
}
#endif
