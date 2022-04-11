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
            ~VulkanContext()noexcept {}

            VulkanContext(const VulkanContext&)noexcept = delete;
            VulkanContext(VulkanContext&&)noexcept      = delete;
            VulkanContext& operator=(const VulkanContext&)noexcept = delete;
            VulkanContext& operator=(VulkanContext&&)noexcept = delete;

            static auto GetHandle()noexcept -> VulkanContext&;
            static void Initialize()noexcept;
            static void Terminate()noexcept;
            static bool IsInitialized() noexcept;
        private:
             VulkanContext()noexcept {}
        private:
            std::unique_ptr<vk::DynamicLoader> m_DLL  = nullptr;
        };
        
        class VulkanInstance;

        class VulkanInstanceBuilder
        {
        public:
            VulkanInstanceBuilder()noexcept {
                VulkanContext::Initialize();
                m_MaxApiVersion = vk::enumerateInstanceVersion();
                m_ExtProps      = vk::enumerateInstanceExtensionProperties();
                m_LyrProps      = vk::enumerateInstanceLayerProperties();
                m_ApiVersion    = m_MaxApiVersion;
                m_ApplicationName = "";
                m_EngineName      = "";
                m_ApplicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
                m_EngineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
                m_ExtNameSet    = {};
                m_LyrNameSet    = {};
                m_DebugReportCallbackCreateInfoOp = std::nullopt;
                m_DebugUtilsMessengerCreateInfos  = {};
            }
            VulkanInstanceBuilder(const VulkanInstanceBuilder&)noexcept = default;
            VulkanInstanceBuilder& operator=(const VulkanInstanceBuilder&)noexcept = default;

            auto Build()const -> std::unique_ptr<VulkanInstance>;

            auto SetApiVersion(uint32_t apiVersion)noexcept -> VulkanInstanceBuilder& {
                if (m_MaxApiVersion >= apiVersion) {
                    m_ApiVersion = apiVersion;
                }
                else {
                    m_ApiVersion = m_MaxApiVersion;
                }
                return *this;
            }
            auto GetApiVersion()const noexcept -> uint32_t {
                return m_ApiVersion;
            }
            auto SetApplicationVersion(uint32_t applicationVersion)noexcept -> VulkanInstanceBuilder& {
                m_ApplicationVersion = applicationVersion;
                return *this;
            }
            auto GetApplicationVersion()const noexcept -> uint32_t {
                return m_ApplicationVersion;
            }
            auto SetEngineVersion(uint32_t engineVersion)noexcept -> VulkanInstanceBuilder& {
                m_EngineVersion = engineVersion;
                return *this;
            }
            auto GetEngineVersion()const noexcept -> uint32_t {
                return m_EngineVersion;
            }

            auto SetApplicationName(const std::string& appName)noexcept -> VulkanInstanceBuilder&
            {
                m_ApplicationName = appName;
                return *this;
            }
            auto GetApplicationName()const noexcept -> std::string { return m_ApplicationName; }
            auto SetEngineName(const std::string& engName)noexcept -> VulkanInstanceBuilder&
            {
                m_EngineName = engName;
                return *this;
            }
            auto GetEngineName()const noexcept -> std::string { return m_EngineName; }

            auto SetExtension(const std::string& extName) noexcept -> VulkanInstanceBuilder&
            {
                auto isContained = std::find_if(std::begin(m_ExtProps), std::end(m_ExtProps), [extName](const auto& extProp) {
                    return std::string(extProp.extensionName.data()) == extName;
                })!= std::end(m_ExtProps);
                if (isContained) {
                    m_ExtNameSet.insert(extName);
                }
                return *this;
            }
            auto GetExtensionSet()const noexcept -> const std::unordered_set<std::string>& { return m_ExtNameSet; }

            auto SetLayer(const std::string& lyrName) noexcept -> VulkanInstanceBuilder&
            {
                auto isContained = std::find_if(std::begin(m_LyrProps), std::end(m_LyrProps), [lyrName](const auto& lyrProp) {
                    return std::string(lyrProp.layerName.data()) == lyrName;
                    }) != std::end(m_LyrProps);
                    if (isContained) {
                        m_LyrNameSet.insert(lyrName);
                    }
                    return *this;
            }
            auto GetLayerSet()const noexcept     -> const std::unordered_set<std::string>& { return m_LyrNameSet; }

            auto SetDebugReportCallback(const vk::DebugReportCallbackCreateInfoEXT& debugReportCallback)noexcept -> VulkanInstanceBuilder&
            {
                m_DebugReportCallbackCreateInfoOp = debugReportCallback;
                m_DebugReportCallbackCreateInfoOp.value().pNext = nullptr;
                return *this;
            }
            auto GetDebugReportCallback()const noexcept -> const std::optional<vk::DebugReportCallbackCreateInfoEXT>& {
                return m_DebugReportCallbackCreateInfoOp;
            }

            auto SetDebugUtilsMessenger(size_t idx, const vk::DebugUtilsMessengerCreateInfoEXT& debugUtilsMessenger)noexcept ->  VulkanInstanceBuilder&
            {
                if (m_DebugUtilsMessengerCreateInfos.size() > idx)
                {
                    m_DebugUtilsMessengerCreateInfos[idx] = debugUtilsMessenger;
                }
                return *this;
            }
            auto AddDebugUtilsMessenger(const vk::DebugUtilsMessengerCreateInfoEXT& debugUtilsMessenger)noexcept -> VulkanInstanceBuilder&
            {
                m_DebugUtilsMessengerCreateInfos.push_back(debugUtilsMessenger);
                return *this;
            }
            auto GetDebugUtilsMessengers()const noexcept -> const std::vector<vk::DebugUtilsMessengerCreateInfoEXT>&
            {
                return m_DebugUtilsMessengerCreateInfos;
            }
        private:
            uint32_t                                            m_ApiVersion;
            uint32_t                                            m_ApplicationVersion;
            uint32_t                                            m_EngineVersion;
            std::string                                         m_ApplicationName;
            std::string                                         m_EngineName;
            std::unordered_set<std::string>                     m_ExtNameSet;
            std::unordered_set<std::string>                     m_LyrNameSet;
            std::optional<vk::DebugReportCallbackCreateInfoEXT> m_DebugReportCallbackCreateInfoOp;
            std::vector<vk::DebugUtilsMessengerCreateInfoEXT>   m_DebugUtilsMessengerCreateInfos;

            uint32_t                                            m_MaxApiVersion;
            std::vector<vk::ExtensionProperties>                m_ExtProps;
            std::vector<vk::LayerProperties>                    m_LyrProps;
        };

        class VulkanInstance
        {   
        public:
            using Builder = VulkanInstanceBuilder;
        public:
            static auto New(const VulkanInstanceBuilder& builder)noexcept -> std::unique_ptr<VulkanInstance>;
            ~VulkanInstance()noexcept;

            auto GetInstanceVk()const noexcept -> vk::Instance { return m_Instance.get(); }
            auto GetDebugUtilsMessengersVk()const noexcept -> std::vector<vk::DebugUtilsMessengerEXT> {
                std::vector<vk::DebugUtilsMessengerEXT> debugUtilsMessengers;
                debugUtilsMessengers.reserve(m_DebugUtilsMessengers.size());
                for (auto& debugUtilsMessenger : m_DebugUtilsMessengers)
                {
                    debugUtilsMessengers.push_back(debugUtilsMessenger.get());
                }
                return debugUtilsMessengers;
            }
            auto GetDebugUtilsMessengerVk(size_t idx)const noexcept -> vk::DebugUtilsMessengerEXT {
                if (m_DebugUtilsMessengers.size() > idx) {
                    return m_DebugUtilsMessengers[idx].get();
                }
                else {
                    return nullptr;
                }
            }
            auto GetDebugReportCallbackVk()const noexcept -> vk::DebugReportCallbackEXT
            {
                return m_DebugReportCallback.get();
            }
            auto GetApiVersion()const noexcept -> uint32_t {
                return m_ApiVersion;
            }
            bool SupportExtension(const char* extName)const noexcept
            {
                return m_EnabledExtNameSet.count(extName) > 0;
            }
            bool SupportLayer    (const char* lyrName)const noexcept
            {
                return m_EnabledLyrNameSet.count(lyrName) > 0;
            }
        private:
            VulkanInstance()noexcept;
        private:
            vk::UniqueInstance                                  m_Instance;
            std::vector<vk::UniqueDebugUtilsMessengerEXT>       m_DebugUtilsMessengers;
            vk::UniqueDebugReportCallbackEXT                    m_DebugReportCallback;
            uint32_t                                            m_ApiVersion;
            uint32_t                                            m_ApplicationVersion;
            uint32_t                                            m_EngineVersion;
            std::string                                         m_ApplicationName;
            std::string                                         m_EngineName;
            std::unordered_set<std::string>                     m_EnabledExtNameSet;
            std::unordered_set<std::string>                     m_EnabledLyrNameSet;
            std::optional<vk::DebugReportCallbackCreateInfoEXT> m_DebugReportCallbackCreateInfo;
            std::vector<vk::DebugUtilsMessengerCreateInfoEXT>   m_DebugUtilsMessengerCreateInfos;
        };

        class VulkanDeviceFeaturesSet
        {
        public:
             VulkanDeviceFeaturesSet()noexcept {}
            ~VulkanDeviceFeaturesSet()noexcept {}

            VulkanDeviceFeaturesSet(const VulkanDeviceFeaturesSet&  featureSet)noexcept;
            VulkanDeviceFeaturesSet(      VulkanDeviceFeaturesSet&& featureSet)noexcept;

            VulkanDeviceFeaturesSet& operator=(const VulkanDeviceFeaturesSet&  featureSet)noexcept;
            VulkanDeviceFeaturesSet& operator=(      VulkanDeviceFeaturesSet&& featureSet)noexcept;

            void Clear()noexcept { m_IndexMap.clear(); m_Holders.clear(); }
            bool Empty()const noexcept { return m_IndexMap.empty(); }
            template<typename VulkanExtFeatureType> 
            bool Contain()const noexcept {
                return Impl_Contain(VulkanExtFeatureType::structureType);
            }
            template<typename VulkanExtFeatureType>
            bool Insert(const VulkanExtFeatureType& feature)noexcept {
                if (!Contain<VulkanExtFeatureType>()) {
                    Impl_Insert<VulkanExtFeatureType>(feature);
                    return true;
                }
                return false;
            }
            template<typename VulkanExtFeatureType>
            auto Get ()noexcept -> const VulkanExtFeatureType* {
                if (Contain<VulkanExtFeatureType>()) {
                    return static_cast<const VulkanExtFeatureType*>(Impl_Find(VulkanExtFeatureType::structureType));
                }
                else {
                    return Impl_Insert<VulkanExtFeatureType>({});
                }
            }
            template<typename VulkanExtFeatureType> 
            auto Find()const noexcept -> const VulkanExtFeatureType *{
                return static_cast<const VulkanExtFeatureType*>(Impl_Read(VulkanExtFeatureType::structureType));
            }
            template<typename VulkanExtFeatureType> 
            bool Read(        VulkanExtFeatureType&    feature)const noexcept
            {
                if (auto pData = Find<VulkanExtFeatureType>()) {
                    std::memcpy((void*)&feature, pData, sizeof(VulkanExtFeatureType));
                    feature.pNext = nullptr;
                    return true;
                }
                return false;
            }
            template<typename VulkanExtFeatureType>
            auto Read()const noexcept -> std::optional<VulkanExtFeatureType>
            {
                VulkanExtFeatureType res;
                if (Read(res)) {
                    return res;
                }
                else {
                    return std::nullopt;
                }
            }
            template<typename VulkanExtFeatureType> 
            bool Write(const  VulkanExtFeatureType&    feature)noexcept
            {
                return Impl_Write(VulkanExtFeatureType::structureType, (const void*)&feature);
            }
            template<typename VulkanExtFeatureType> 
            bool Map (        VulkanExtFeatureType** ppFeature)noexcept
            {
                return Impl_Map(VulkanExtFeatureType::structureType, (void**)ppFeature);
            }
            template<typename VulkanExtFeatureType> 
            bool Unmap()noexcept
            {
                return Impl_Unmap(VulkanExtFeatureType::structureType);
            }

            auto   ReadHead()const noexcept -> const void* { 
                if (!m_Holders.empty()) {
                    return m_Holders.front()->Read();
                }
                return nullptr; 
            }
            void   LinkTail(void* pNext)noexcept {
                if (!m_Holders.empty()) {
                    m_Holders.back()->Link(pNext);
                    m_TailPNext = pNext;
                }
            }
            void UnlinkTail()noexcept
            {
                if (!m_Holders.empty()) {
                    m_Holders.back()->Unlink();
                    m_TailPNext = nullptr;
                }
            }
        private:
            template<typename VulkanExtFeatureType>
            auto Impl_Insert (const VulkanExtFeatureType& feature)noexcept -> VulkanExtFeatureType*;
            bool Impl_Contain(vk::StructureType sType) const noexcept;
            auto Impl_Read   (vk::StructureType sType) const noexcept -> const void*;
            bool Impl_Write  (vk::StructureType sType, const void*   )noexcept;
            bool Impl_Map    (vk::StructureType sType, void**        )noexcept;
            bool Impl_Unmap  (vk::StructureType sType)noexcept;
        private:
            class IVulkanDeviceExtFeatureHolder {
            public:
                virtual ~IVulkanDeviceExtFeatureHolder()noexcept {}
                virtual auto Clone()const noexcept -> IVulkanDeviceExtFeatureHolder* = 0;
                virtual auto Read()const noexcept -> const void* = 0;
                virtual void Write(const void* pData)noexcept    = 0;
                virtual void Link(void* pNext)noexcept = 0;
                virtual void Unlink()noexcept          = 0;
                virtual auto GetPointer()noexcept->void* = 0;
                virtual void Map()noexcept = 0;
                virtual void Unmap()noexcept = 0;
            };
            template<typename VulkanExtFeatureType>
            class VulkanDeviceExtFeatureHolder : public IVulkanDeviceExtFeatureHolder {
            public:
                VulkanDeviceExtFeatureHolder(const VulkanExtFeatureType& feature)noexcept{
                    m_Feature       = feature;
                    m_Feature.pNext = nullptr;
                    m_PNextForMap   = nullptr;
                }
                virtual ~VulkanDeviceExtFeatureHolder()noexcept {}
                virtual auto Clone()const noexcept -> IVulkanDeviceExtFeatureHolder* {
                    return new VulkanDeviceExtFeatureHolder(m_Feature);
                }
                virtual auto Read()const noexcept -> const void* {
                    return &m_Feature;
                }
                virtual void Write(const void* pData)noexcept {
                    auto pNext = m_Feature.pNext;
                    std::memcpy(&m_Feature, pData, sizeof(m_Feature));
                    m_Feature.pNext = pNext;
                }
                virtual void Link(void* pNext)noexcept {
                    m_Feature.pNext = pNext;
                }
                virtual void Unlink()noexcept {
                    m_Feature.pNext = nullptr;
                }
                virtual auto GetPointer()      noexcept->       void* {
                    return &m_Feature;
                }
                virtual void Map()noexcept {
                    m_PNextForMap = m_Feature.pNext;
                }
                virtual void Unmap()noexcept {
                    m_Feature.pNext = m_PNextForMap;
                    m_PNextForMap = nullptr;
                }
            private:
                VulkanExtFeatureType                  m_Feature;
                decltype(VulkanExtFeatureType::pNext) m_PNextForMap;
            };
        private:
            std::unordered_map<vk::StructureType,size_t>                m_IndexMap  = {};
            std::vector<std::unique_ptr<IVulkanDeviceExtFeatureHolder>> m_Holders   = {};
            void*                                                       m_TailPNext = nullptr;
        };

        template<typename VulkanExtFeatureType>
        auto  VulkanDeviceFeaturesSet::Impl_Insert(const VulkanExtFeatureType& feature)noexcept -> VulkanExtFeatureType*
        {
            if (!m_Holders.empty()) {
                auto  size        = m_Holders.size();
                auto* pBackHolder = m_Holders.back().get();
                m_Holders.push_back(std::unique_ptr<VulkanDeviceFeaturesSet::IVulkanDeviceExtFeatureHolder>(
                    new VulkanDeviceFeaturesSet::VulkanDeviceExtFeatureHolder<VulkanExtFeatureType>(feature)
                ));
                m_IndexMap.insert({ VulkanExtFeatureType::structureType,size });
                pBackHolder->Link(m_Holders.back()->GetPointer());
                m_Holders.back()->Link(m_TailPNext);
            }
            else {
                m_Holders.push_back(std::unique_ptr<VulkanDeviceFeaturesSet::IVulkanDeviceExtFeatureHolder>(
                    new VulkanDeviceFeaturesSet::VulkanDeviceExtFeatureHolder<VulkanExtFeatureType>(feature)
                ));
                m_IndexMap.insert({ VulkanExtFeatureType::structureType,0 });
            }
            return static_cast<VulkanExtFeatureType *> (m_Holders.back()->GetPointer());
        }

        class VulkanDevice;

        class VulkanQueue {
        public:
            static auto Acquire(  const VulkanDevice* device, uint32_t queueFamilyIndex, uint32_t queueIndex)->std::optional<VulkanQueue>;
            static auto Enumerate(const VulkanDevice* device, uint32_t queueFamilyIndex)->std::vector<VulkanQueue>;

            VulkanQueue(const VulkanQueue&)noexcept = default;
            VulkanQueue& operator=(const VulkanQueue&)noexcept = default;

            auto GetDevice()const noexcept -> const VulkanDevice* { return m_Device; }
            auto GetQueueVk()const noexcept  -> vk::Queue { return m_Queue; }
            auto GetQueueFamilyIndex()const noexcept -> uint32_t { return m_QueueFamilyIndex; }
            auto GetQueueIndex()const noexcept -> uint32_t { return m_QueueIndex; }
            auto GetQueuePriority()const noexcept -> float { return m_Priority; }
        private:
            VulkanQueue()noexcept;
        private:
            const VulkanDevice* m_Device;
            vk::Queue           m_Queue;
            uint32_t            m_QueueFamilyIndex;
            uint32_t            m_QueueIndex;
            float               m_Priority;
        };

        class VulkanQueueFamily;

        class VulkanQueueFamilyBuilder
        {
        public:
            VulkanQueueFamilyBuilder()noexcept {
                m_QueueFamilyIndex = 0;
                m_QueueProperties = {};
            }
            VulkanQueueFamilyBuilder(uint32_t queueFamilyIndex)noexcept
            {
                m_QueueFamilyIndex = queueFamilyIndex;
                m_QueueProperties  = {};
            }
            VulkanQueueFamilyBuilder(uint32_t queueFamilyIndex, uint32_t queueCount)noexcept
            {
                m_QueueFamilyIndex = queueFamilyIndex;
                m_QueueProperties  = std::vector<float>(queueCount, 1.0f);
            }

            VulkanQueueFamilyBuilder(const VulkanQueueFamilyBuilder& builder)noexcept = default;
            VulkanQueueFamilyBuilder& operator=(const VulkanQueueFamilyBuilder& builder)noexcept = default;

            auto SetQueueFamilyIndex(uint32_t queueFamilyIndex)noexcept -> VulkanQueueFamilyBuilder&
            {
                m_QueueFamilyIndex = queueFamilyIndex;
                return *this;
            }
            auto GetQueueFamilyIndex()const noexcept -> uint32_t { return m_QueueFamilyIndex; }
            auto SetQueueCount(uint32_t queueCount)noexcept -> VulkanQueueFamilyBuilder&
            {
                m_QueueProperties = std::vector<float>(queueCount, 1.0f);
                return *this;
            }
            auto GetQueueCount()const noexcept -> uint32_t { return m_QueueProperties.size(); }
            auto SetQueueProperty(size_t idx, float val)noexcept -> VulkanQueueFamilyBuilder&
            {
                if (m_QueueProperties.size() > idx) {
                    m_QueueProperties[idx] = val;
                }
                return *this;
            }
            auto SetQueueProperties(const std::vector<float>& properties)noexcept -> VulkanQueueFamilyBuilder&
            {
                m_QueueProperties = properties;
                return *this;
            }
            auto GetQueueProperties()const noexcept -> const std::vector<float>&
            {
                return m_QueueProperties;
            }

            auto Build(const VulkanDevice* device)const -> std::optional<VulkanQueueFamily>;
        private:
            uint32_t           m_QueueFamilyIndex;
            std::vector<float> m_QueueProperties;
        };

        class VulkanCommandPool;

        class VulkanQueueFamily
        {
        public:
            using Builder = VulkanQueueFamilyBuilder;
            static auto Acquire(const VulkanDevice* device, uint32_t queueFamilyIndex)noexcept->std::optional<VulkanQueueFamily>;

            VulkanQueueFamily(const VulkanQueueFamily& queueFamily)noexcept             = default;
            VulkanQueueFamily& operator=(const VulkanQueueFamily & queueFamily)noexcept = default;

            auto GetDevice()const noexcept -> const VulkanDevice* { return m_Device; }
            auto GetQueueFamilyIndex()const noexcept -> uint32_t  { return m_QueueFamilyIndex; }
            auto GetQueues()const noexcept -> const std::vector<VulkanQueue>& { return m_Queues; }

            auto NewCommandPool()const->std::unique_ptr<VulkanCommandPool>;
        private:
            VulkanQueueFamily()noexcept :m_Device{ nullptr }, m_QueueFamilyIndex{ 0 }, m_Queues{}{}
        private:
            const VulkanDevice*      m_Device;
            uint32_t                 m_QueueFamilyIndex;
            std::vector<VulkanQueue> m_Queues;
        };

        class VulkanCommandBuffer;

        class VulkanCommandPool
        {
        public:
            static auto New(const VulkanDevice* device, uint32_t queueFamilyIndex)noexcept->std::unique_ptr<VulkanCommandPool>;
            ~VulkanCommandPool()noexcept;

            auto GetDevice()const noexcept -> const VulkanDevice*;
            auto GetCommandPoolVk()const noexcept -> vk::CommandPool;
            auto GetQueueFamilyIndex()const noexcept -> uint32_t;

            auto NewCommandBuffer(vk::CommandBufferLevel level)const noexcept -> std::unique_ptr<VulkanCommandBuffer>;
        private:
            VulkanCommandPool()noexcept;
        private:
            const VulkanDevice*   m_Device;
            vk::UniqueCommandPool m_CommandPool;
            uint32_t              m_QueueFamilyIndex;
        };

        class VulkanCommandBuffer
        {
        public:
            static auto New(const VulkanCommandPool* commandPool, vk::CommandBufferLevel commandBufferLevel)noexcept->std::unique_ptr<VulkanCommandBuffer>;
            ~VulkanCommandBuffer()noexcept;

            auto GetCommandPool()const noexcept -> const VulkanCommandPool*;
            auto GetCommandBufferVk()const noexcept -> vk::CommandBuffer;
        private:
            VulkanCommandBuffer()noexcept;
        private:
            const VulkanCommandPool* m_CommandPool;
            vk::UniqueCommandBuffer  m_CommandBuffer;
        };

        class VulkanDeviceBuilder
        {
        public:
            VulkanDeviceBuilder(const VulkanInstance* instance, vk::PhysicalDevice physicalDevice)noexcept {
                m_Instance              = instance;
                m_PhysicalDevice        = physicalDevice;
                m_ExtNameSet            = {};
                m_FeaturesSet.Insert<vk::PhysicalDeviceFeatures2>(physicalDevice.getFeatures2());
                m_QueueFamilyMap        = {};
                m_ExtProps              = physicalDevice.enumerateDeviceExtensionProperties();
                m_QueueFamilyProperties = physicalDevice.getQueueFamilyProperties();
            }
            VulkanDeviceBuilder(const VulkanDeviceBuilder&)noexcept            = default;
            VulkanDeviceBuilder& operator=(const VulkanDeviceBuilder&)noexcept = default;

            static auto Enumerate(const VulkanInstance* instance)noexcept -> std::vector<VulkanDeviceBuilder> {
                auto physicalDevices = instance->GetInstanceVk().enumeratePhysicalDevices();
                auto deviceBuilders = std::vector<VulkanDeviceBuilder>();
                deviceBuilders.reserve(physicalDevices.size());
                for (auto& physicalDevice : physicalDevices) {
                    deviceBuilders.push_back(VulkanDeviceBuilder(instance, physicalDevice));
                }
                return deviceBuilders;
            }
            //
            auto GetInstance()const noexcept -> const VulkanInstance* { return m_Instance; }
            auto GetPhysicalDevice()const noexcept -> vk::PhysicalDevice { return m_PhysicalDevice; }
            //Extensions
            auto SetExtension(const std::string& extName) noexcept -> VulkanDeviceBuilder&
            {
                auto isContained = std::find_if(std::begin(m_ExtProps), std::end(m_ExtProps), [extName](const auto& extProp) {
                    return std::string(extProp.extensionName.data()) == extName;
                }) != std::end(m_ExtProps);
                if (isContained) {
                    m_ExtNameSet.insert(extName);
                }
                return *this;
            }
            auto GetExtensionSet()const noexcept -> const std::unordered_set<std::string>& { return m_ExtNameSet; }
            //Features
            template<typename VulkanFeaturesType>
            auto SetFeatures(const VulkanFeaturesType& features)noexcept -> VulkanDeviceBuilder&
            {
                if (
                   !m_FeaturesSet.Insert(features)
                ){
                    m_FeaturesSet.Write(features);
                }
                return *this;
            }
            auto GetFeaturesSet()const noexcept -> const VulkanDeviceFeaturesSet& { return m_FeaturesSet; }
            template<typename VulkanFeaturesType>
            auto ResetFeatures()noexcept -> VulkanDeviceBuilder&
            {
                if constexpr (std::is_same_v<vk::PhysicalDeviceFeatures2, VulkanFeaturesType>)
                {
                    auto features      = m_PhysicalDevice.getFeatures2();
                    return SetFeatures(features);
                }
                else {
                    auto featuresChain = m_PhysicalDevice.getFeatures2<vk::PhysicalDeviceFeatures2, VulkanFeaturesType>();
                    auto features = featuresChain.template get<VulkanFeaturesType>();
                    return SetFeatures(features);
                }
            }
            //QueueFamilyProperties
            auto SetQueueFamily(const VulkanQueueFamilyBuilder& queueFamily)noexcept -> VulkanDeviceBuilder& {
                if (m_QueueFamilyProperties.size() > queueFamily.GetQueueFamilyIndex()) {
                    if (m_QueueFamilyProperties[queueFamily.GetQueueFamilyIndex()].queueCount >= queueFamily.GetQueueCount() && queueFamily.GetQueueCount() > 0) {
                        m_QueueFamilyMap[queueFamily.GetQueueFamilyIndex()] = queueFamily;
                    }
                }
                return *this;
            }
            auto SetQueueFamilies(const std::vector<VulkanQueueFamilyBuilder>& queueFamilies)noexcept -> VulkanDeviceBuilder& {
                for (auto& queueFamily:queueFamilies){
                    SetQueueFamily(queueFamily);
                }
                return *this;
            }
            auto GetQueueFamilySet()const noexcept -> const std::unordered_map<uint32_t, VulkanQueueFamilyBuilder>& {
                return m_QueueFamilyMap;
            }
            auto Build()const->std::unique_ptr<VulkanDevice>;
        private:
            const VulkanInstance*                                        m_Instance;
            vk::PhysicalDevice                                           m_PhysicalDevice;
            std::unordered_set<std::string>                              m_ExtNameSet;
            VulkanDeviceFeaturesSet                                      m_FeaturesSet;
            std::unordered_map<uint32_t, VulkanQueueFamilyBuilder>       m_QueueFamilyMap;
            std::vector<vk::ExtensionProperties>                         m_ExtProps;
            std::vector<vk::QueueFamilyProperties>                       m_QueueFamilyProperties;
        };

        class VulkanFence;

        class VulkanDevice{
        public:
            using Builder = VulkanDeviceBuilder;
            static auto New(const VulkanDeviceBuilder& builder)noexcept -> std::unique_ptr<VulkanDevice>;
            virtual ~VulkanDevice()noexcept;

            auto NewFence(bool isSignaled = false)->std::unique_ptr<VulkanFence>;
            auto WaitForFences(const std::vector<const VulkanFence*>& fences, uint64_t timeOut = UINT64_MAX, VkBool32 waitForAll = VK_FALSE)->vk::Result;

            auto GetInstance()const noexcept -> const VulkanInstance* { return m_Instance; }
            auto GetPhysicalDeviceVk()const noexcept -> vk::PhysicalDevice { return m_PhysicalDevice; }
            auto GetDeviceVk() const noexcept -> vk::Device         { return m_LogigalDevice.get(); }

            auto EnumerateQueues(uint32_t queueFamilyIndex)const->std::vector<VulkanQueue>;
            auto AcquireQueueFamily(uint32_t queueFamilyIndex)const -> std::optional<VulkanQueueFamily>;
            bool SupportQueueFamily(uint32_t queueFamilyIndex)const noexcept { return m_QueueFamilyMap.count(queueFamilyIndex) > 0; }
            bool SupportExtension(const char* extName)const noexcept
            {
                return m_EnabledExtNameSet.count(extName) > 0;
            }
            auto QueryQueuePriorities(uint32_t queueFamilyIndex)const noexcept -> std::vector<float> {
                return m_QueueFamilyMap.count(queueFamilyIndex) > 0 ? m_QueueFamilyMap.at(queueFamilyIndex).GetQueueProperties() : std::vector<float>{};
            }
            auto QueryQueueCount(uint32_t queueFamilyIndex)const noexcept -> uint32_t { return  m_QueueFamilyMap.count(queueFamilyIndex) > 0 ? m_QueueFamilyMap.at(queueFamilyIndex).GetQueueCount() : 0; }
            template<typename VulkanFeatureType>
            auto QueryFeatures(VulkanFeatureType& features)const noexcept -> bool {
                return m_EnabledFeaturesSet.Read(features);;
            }
            template<typename VulkanFeatureType>
            auto QueryFeatures()const noexcept -> std::optional<VulkanFeatureType>
            {
                return m_EnabledFeaturesSet.Read<VulkanFeatureType>();
            }
        private:
            VulkanDevice()noexcept;
        private:
            const VulkanInstance*                                        m_Instance;
            vk::PhysicalDevice                                           m_PhysicalDevice;
            vk::UniqueDevice                                             m_LogigalDevice;
            std::unordered_set<std::string>                              m_EnabledExtNameSet;
            VulkanDeviceFeaturesSet                                      m_EnabledFeaturesSet;
            std::unordered_map<uint32_t, VulkanQueueFamilyBuilder>       m_QueueFamilyMap;
        };

        class VulkanFence {
        public:
            static auto New(const VulkanDevice* device, bool isSignaled = false)->std::unique_ptr<VulkanFence>;
            virtual ~VulkanFence()noexcept;

            auto Wait(uint64_t timeout)const noexcept -> vk::Result;
            auto QueryStatus()const noexcept -> vk::Result;
            auto GetFenceVk() const noexcept -> vk::Fence { return m_Fence.get(); }
            auto GetDevice()  const noexcept -> const VulkanDevice* { return m_Device; }
            auto GetDeviceVk()const noexcept -> vk::Device { return m_Device->GetDeviceVk(); }
        private:
            VulkanFence()noexcept;
        private:
            const VulkanDevice* m_Device;
            vk::UniqueFence     m_Fence;

        };

        class VulkanBuffer;

        class VulkanBufferBuilder {
        public:
            VulkanBufferBuilder()noexcept {
                m_Flags = {};
                m_Size  = 0;
                m_Usage = {};
                m_QueueFamilyIndices = {};
            }

            VulkanBufferBuilder(const VulkanBufferBuilder&) noexcept = default;

            VulkanBufferBuilder& operator=(const VulkanBufferBuilder&) noexcept = default;

            auto GetFlags()const noexcept -> vk::BufferCreateFlags { return m_Flags; }
            auto SetFlags(vk::BufferCreateFlags flags)noexcept -> VulkanBufferBuilder& { 
                m_Flags = flags;
                return *this; 
            }
            auto GetSize()const noexcept  -> vk::DeviceSize { return m_Size; }
            auto SetSize(vk::DeviceSize size)noexcept -> VulkanBufferBuilder& {
                m_Size = size;
                return *this;
            }
            auto GetUsage()const noexcept -> vk::BufferUsageFlags { return m_Usage; }
            auto SetUsage(vk::BufferUsageFlags usage)noexcept -> VulkanBufferBuilder& {
                m_Usage = usage;
                return *this;
            }

            auto GetQueueFamilyIndices()const noexcept  -> std::vector<uint32_t> { return m_QueueFamilyIndices; }
            auto SetQueueFamilyIndices(const std::vector<uint32_t>& queueFamilyIndices)noexcept -> VulkanBufferBuilder& {
                m_QueueFamilyIndices = queueFamilyIndices;
                return *this;
            }
            auto GetSharingMode()const noexcept -> vk::SharingMode { return m_QueueFamilyIndices.empty() ? vk::SharingMode::eExclusive : vk::SharingMode::eConcurrent; }

            auto Build(const VulkanDevice* device)const noexcept -> std::unique_ptr<VulkanBuffer>;
        private:
            vk::BufferCreateFlags m_Flags;
            vk::DeviceSize        m_Size;
            vk::BufferUsageFlags  m_Usage;
            std::vector<uint32_t> m_QueueFamilyIndices;
        };

        class VulkanBuffer {
        public:
            using Builder = VulkanBufferBuilder;
            static auto New(const VulkanDevice* device, const VulkanBufferBuilder& buffer)->std::unique_ptr<VulkanBuffer>;
            virtual ~VulkanBuffer()noexcept;

            auto QueryMemoryRequirements()const -> vk::MemoryRequirements;

            auto GetDevice()const noexcept -> const VulkanDevice* { return m_Device; }
            auto GetBufferVk()const noexcept    -> vk::Buffer { return m_Buffer.get(); }
            auto GetFlags()const noexcept       -> vk::BufferCreateFlags  { return m_Flags; }
            auto GetSize ()const noexcept       -> vk::DeviceSize { return m_Size; }
            auto GetUsage()const noexcept       -> vk::BufferUsageFlags { return m_Usage; }
            auto GetSharingMode()const noexcept -> vk::SharingMode { return m_QueueFamilyIndices.empty() ? vk::SharingMode::eExclusive : vk::SharingMode::eConcurrent; }
        private:
            VulkanBuffer()noexcept;
        private:
            const VulkanDevice*   m_Device;
            vk::UniqueBuffer      m_Buffer;
            vk::BufferCreateFlags m_Flags;
            vk::DeviceSize        m_Size;
            vk::BufferUsageFlags  m_Usage;
            std::vector<uint32_t> m_QueueFamilyIndices;
        };

        class VulkanImage;

        class VulkanImageBuilder {
        public:
            VulkanImageBuilder()noexcept {
                m_Flags       = {};
                m_ImageType   = vk::ImageType::e2D;
                m_Format      = vk::Format::eUndefined;
                m_Extent      = vk::Extent3D();
                m_MipLevels   = 1;
                m_ArrayLayers = 1;
                m_Samples     = vk::SampleCountFlagBits::e1;
                m_Usage       = {};
                m_QueueFamilyIndices = {};
                m_InitialLayout = vk::ImageLayout::eUndefined;
            }

            VulkanImageBuilder(const VulkanImageBuilder&) noexcept = default;

            VulkanImageBuilder& operator=(const VulkanImageBuilder&) noexcept = default;

            auto GetFlags()const noexcept -> vk::ImageCreateFlags { return m_Flags; }
            auto SetFlags(vk::ImageCreateFlags flags)noexcept -> VulkanImageBuilder& {
                m_Flags = flags;
                return *this;
            }

            auto GetImageType()const noexcept  -> vk::ImageType { return m_ImageType; }
            auto SetImageType(vk::ImageType imageType)noexcept -> VulkanImageBuilder& {
                m_ImageType = imageType;
                return *this;
            }

            auto GetExtent()const noexcept -> const vk::Extent3D& { return m_Extent; }
            auto SetExtent(const vk::Extent3D& extent) noexcept ->  VulkanImageBuilder& {
                m_Extent = extent;
                return *this; 
            }

            auto GetFormat()const noexcept -> const vk::Format { return m_Format; }
            auto SetFormat(const vk::Format& format) noexcept ->  VulkanImageBuilder& {
                m_Format = format;
                return *this;
            }

            auto GetMipLevels()  const noexcept   -> uint32_t { return m_MipLevels; }
            auto SetMipLevels(uint32_t mipLevels) noexcept -> VulkanImageBuilder& { 
                m_MipLevels = mipLevels;
                return *this; 
            }

            auto GetArrayLayers()const noexcept -> uint32_t { return m_ArrayLayers; }
            auto SetArrayLayers(uint32_t arrayLayers) noexcept -> VulkanImageBuilder& {
                m_ArrayLayers = arrayLayers;
                return *this;
            }

            auto GetSamples()    const noexcept -> vk::SampleCountFlagBits { return m_Samples; }
            auto SetSamples(vk::SampleCountFlagBits samples)noexcept -> VulkanImageBuilder& {
                m_Samples = samples;
                return *this;
            }

            auto GetTiling()const noexcept -> vk::ImageTiling { return m_Tiling; }
            auto SetTiling(vk::ImageTiling tiling)noexcept ->  VulkanImageBuilder& {
                m_Tiling = tiling;
                return *this;
            }

            auto GetUsage()const noexcept -> vk::ImageUsageFlags { return m_Usage; }
            auto SetUsage(vk::ImageUsageFlags usage)noexcept -> VulkanImageBuilder& {
                m_Usage = usage;
                return *this;
            }

            auto GetQueueFamilyIndices()const noexcept  -> std::vector<uint32_t> { return m_QueueFamilyIndices; }
            auto SetQueueFamilyIndices(const std::vector<uint32_t>& queueFamilyIndices)noexcept -> VulkanImageBuilder& {
                m_QueueFamilyIndices = queueFamilyIndices;
                return *this;
            }
            auto GetSharingMode()const noexcept -> vk::SharingMode { return m_QueueFamilyIndices.empty() ? vk::SharingMode::eExclusive : vk::SharingMode::eConcurrent; }

            auto GetInitialLayout()const noexcept -> vk::ImageLayout { return m_InitialLayout; }
            auto SetInitialLayout(vk::ImageLayout initialLayout)noexcept -> VulkanImageBuilder& {
                m_InitialLayout = initialLayout;
                return *this;
            }

            auto Build(const VulkanDevice* device)const noexcept -> std::unique_ptr<VulkanImage>;
        private:
            vk::ImageCreateFlags    m_Flags;
            vk::ImageType           m_ImageType;
            vk::Format              m_Format;
            vk::Extent3D            m_Extent;
            uint32_t                m_MipLevels;
            uint32_t                m_ArrayLayers;
            vk::SampleCountFlagBits m_Samples;
            vk::ImageTiling         m_Tiling;
            vk::ImageUsageFlags     m_Usage;
            std::vector<uint32_t>   m_QueueFamilyIndices;
            vk::ImageLayout         m_InitialLayout;
        };

        class VulkanImage {
        public:
            using Builder = VulkanImageBuilder;
            static auto New(const VulkanDevice* device, const VulkanImageBuilder& builder)->std::unique_ptr<VulkanImage>;
            virtual ~VulkanImage()noexcept;

            auto GetDevice()const noexcept -> const VulkanDevice* { return m_Device; }
            auto GetImageVk()const noexcept    -> vk::Image { return m_Image.get(); }
            auto GetFlags()const noexcept -> vk::ImageCreateFlags { return m_Flags; }
            auto GetImageType()const noexcept  -> vk::ImageType { return m_ImageType; }
            auto GetExtent()const noexcept -> const vk::Extent3D& { return m_Extent; }
            auto GetFormat()const noexcept -> const vk::Format { return m_Format; }
            auto GetMipLevels()  const noexcept   -> uint32_t { return m_MipLevels; }
            auto GetArrayLayers()const noexcept -> uint32_t { return m_ArrayLayers; }
            auto GetSamples()    const noexcept -> vk::SampleCountFlagBits { return m_Samples; }
            auto GetTiling()const noexcept -> vk::ImageTiling { return m_Tiling; }
            auto GetUsage()const noexcept -> vk::ImageUsageFlags { return m_Usage; }
            auto GetQueueFamilyIndices()const noexcept  -> std::vector<uint32_t> { return m_QueueFamilyIndices; }
            auto GetInitialLayout()const noexcept -> vk::ImageLayout { return m_InitialLayout; }
        private:
            VulkanImage()noexcept;
        private:
            const VulkanDevice*     m_Device;
            vk::UniqueImage         m_Image;
            vk::ImageCreateFlags    m_Flags;
            vk::ImageType           m_ImageType;
            vk::Format              m_Format;
            vk::Extent3D            m_Extent;
            uint32_t                m_MipLevels;
            uint32_t                m_ArrayLayers;
            vk::SampleCountFlagBits m_Samples;
            vk::ImageTiling         m_Tiling;
            vk::ImageUsageFlags     m_Usage;
            std::vector<uint32_t>   m_QueueFamilyIndices;
            vk::ImageLayout         m_InitialLayout;
        };

        class VulkanDeviceMemory;

        class VulkanDeviceMemoryBuilder {
        public:
            VulkanDeviceMemoryBuilder()noexcept {
                m_AllocationSize  = 0;
                m_MemoryTypeIndex = 0;
                m_MemoryAllocateFlagsInfo     = std::nullopt;
                m_MemoryDedicatedAllocateInfo = std::nullopt;
            }

            VulkanDeviceMemoryBuilder(const VulkanDeviceMemoryBuilder&) = default;

            VulkanDeviceMemoryBuilder& operator=(const VulkanDeviceMemoryBuilder&)noexcept = default;

            auto GetAllocationSize() const noexcept -> vk::DeviceSize { return m_AllocationSize;  }
            auto SetAllocationSize(vk::DeviceSize allocationSize)noexcept -> VulkanDeviceMemoryBuilder& {
                m_AllocationSize = allocationSize;
                return *this;
            }

            auto GetMemoryTypeIndex()const noexcept -> uint32_t       { return m_MemoryTypeIndex; }
            auto SetMemoryTypeIndex(uint32_t memoryTypeIndex)noexcept -> VulkanDeviceMemoryBuilder& {
                m_MemoryTypeIndex = memoryTypeIndex;
                return *this;
            }

            auto GetMemoryAllocateFlagsInfo()const noexcept -> const std::optional<vk::MemoryAllocateFlagsInfo>& {
                return m_MemoryAllocateFlagsInfo;
            }
            auto SetMemoryAllocateFlagsInfo(const vk::MemoryAllocateFlagsInfo& flagsInfo)noexcept -> VulkanDeviceMemoryBuilder& {
                m_MemoryAllocateFlagsInfo = flagsInfo;
                return *this;
            }

            auto GetMemoryDedicatedAllocateInfo()const noexcept -> const std::optional<vk::MemoryDedicatedAllocateInfo>& {
                return m_MemoryDedicatedAllocateInfo;
            }
            auto SetMemoryDedicatedAllocateInfo(const vk::MemoryDedicatedAllocateInfo& dedicatedAllocation)noexcept -> VulkanDeviceMemoryBuilder& {
                m_MemoryDedicatedAllocateInfo = dedicatedAllocation;
                return *this;
            }

            auto Build(const VulkanDevice* device)const->std::unique_ptr<VulkanDeviceMemory>;
        private:
            vk::DeviceSize                                 m_AllocationSize;
            uint32_t                                       m_MemoryTypeIndex;
            std::optional<vk::MemoryAllocateFlagsInfo>     m_MemoryAllocateFlagsInfo;
            std::optional<vk::MemoryDedicatedAllocateInfo> m_MemoryDedicatedAllocateInfo;
        };

        class VulkanDeviceMemory {
        public:
            using Builder = VulkanDeviceMemoryBuilder;
            static auto New(const VulkanDevice* device, const VulkanDeviceMemoryBuilder& builder)->std::unique_ptr<VulkanDeviceMemory>;
            virtual ~VulkanDeviceMemory()noexcept;

            auto   Map(void** pPData, vk::MemoryMapFlags flags = {})const->vk::Result;
            auto   Map(void** pPData, vk::DeviceSize size, vk::DeviceSize offset = 0, vk::MemoryMapFlags flags = {})const->vk::Result;
            void Unmap()const;

            auto GetDevice()const -> const VulkanDevice* { return m_Device; }
            auto GetDeviceMemoryVk()const -> vk::DeviceMemory { return m_DeviceMemory.get(); }
            auto GetAllocationSize() const noexcept -> vk::DeviceSize { return m_AllocationSize; }
            auto GetMemoryTypeIndex()const noexcept -> uint32_t  { return m_MemoryTypeIndex; }
            auto GetMemoryAllocateFlagsInfo()const noexcept -> const std::optional<vk::MemoryAllocateFlagsInfo>& {
                return m_MemoryAllocateFlagsInfo;
            }
            auto GetMemoryDedicatedAllocateInfo()const noexcept -> const std::optional<vk::MemoryDedicatedAllocateInfo>& {
                return m_MemoryDedicatedAllocateInfo;
            }
        private:
            VulkanDeviceMemory()noexcept;
        private:
            const VulkanDevice*                            m_Device;
            vk::UniqueDeviceMemory                         m_DeviceMemory;
            vk::DeviceSize                                 m_AllocationSize;
            uint32_t                                       m_MemoryTypeIndex;
            std::optional<vk::MemoryAllocateFlagsInfo>     m_MemoryAllocateFlagsInfo;
            std::optional<vk::MemoryDedicatedAllocateInfo> m_MemoryDedicatedAllocateInfo;
        };
        class VulkanMemoryBuffer {
        public:
            static auto Bind(const VulkanBuffer* buffer, const VulkanDeviceMemory* memory, vk::DeviceSize memoryOffset = 0) -> std::unique_ptr<VulkanMemoryBuffer>;
            virtual ~VulkanMemoryBuffer()noexcept;

            auto   Map(void** pPData, vk::MemoryMapFlags flags = {})const->vk::Result;
            auto   Map(void** pPData, vk::DeviceSize size, vk::DeviceSize offset = 0, vk::MemoryMapFlags flags = {})const->vk::Result;
            void Unmap()const;

            auto GetBuffer()const noexcept -> const VulkanBuffer      * { return m_Buffer; }
            auto GetBufferVk()const noexcept -> vk::Buffer { return m_Buffer->GetBufferVk(); }
            auto GetBufferSize   ()const noexcept -> vk::DeviceSize     { return m_Buffer->GetSize();}
            auto GetMemory()const noexcept -> const VulkanDeviceMemory* { return m_Memory; }
            auto GetMemoryVk()const noexcept -> vk::DeviceMemory { return m_Memory->GetDeviceMemoryVk(); }
            auto GetMemoryOffset ()const noexcept -> vk::DeviceSize     { return m_MemoryOffset; }
            auto GetMemoryAllocationSize()const noexcept -> vk::DeviceSize { return m_Memory->GetAllocationSize();}
            auto GetDeviceAddress()const noexcept -> std::optional<vk::DeviceAddress> { return m_DeviceAddress;}
        private:
            VulkanMemoryBuffer()noexcept;
        private:
            const VulkanBuffer      *     m_Buffer;
            const VulkanDeviceMemory*     m_Memory;
            vk::DeviceSize                m_MemoryOffset;
            std::optional<vk::DeviceSize> m_DeviceAddress;
        };

        class VulkanMemoryImage {
        public:
            static auto Bind(const VulkanImage* image, const VulkanDeviceMemory* memory, vk::DeviceSize memoryOffset = 0) -> std::unique_ptr<VulkanMemoryImage>;
            virtual ~VulkanMemoryImage()noexcept;

            auto   Map(void** pPData, vk::DeviceSize size, vk::DeviceSize offset = 0, vk::MemoryMapFlags flags = {})const->vk::Result;
            void Unmap()const;

            auto GetImage ()const noexcept -> const VulkanImage       *    { return m_Image; }
            auto GetImageVk()const noexcept -> vk::Image { return m_Image->GetImageVk(); }
            auto GetMemory()const noexcept -> const VulkanDeviceMemory*    { return m_Memory; }
            auto GetMemoryVk()const noexcept -> vk::DeviceMemory { return m_Memory->GetDeviceMemoryVk(); }
            auto GetMemoryOffset()const noexcept -> vk::DeviceSize         { return m_MemoryOffset; }
            auto GetMemoryAllocationSize()const noexcept -> vk::DeviceSize { return m_Memory->GetAllocationSize(); }
        private:
            VulkanMemoryImage()noexcept;
        private:
            const VulkanImage       *     m_Image;
            const VulkanDeviceMemory*     m_Memory;
            vk::DeviceSize                m_MemoryOffset;
        };
    }
}
#endif

