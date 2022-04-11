#include <BulletRT/Core/BulletRTCore.h>
#include <iostream>
#include <string>
#include <string_view>
#include <ranges>
#include <algorithm>
static VKAPI_ATTR VkBool32 VKAPI_CALL DebugUtilsUserCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT                  messageTypes,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	std::cout << "[" << vk::to_string(static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(messageSeverity)) << "]";
	std::cout << "[" << vk::to_string(static_cast<vk::DebugUtilsMessageTypeFlagsEXT>(messageTypes)) << "]: ";
	std::cout << pCallbackData->pMessage << std::endl;
	return VK_FALSE;
}
auto FindMemoryTypeIndices(const vk::PhysicalDeviceMemoryProperties& memoryProps,
	uint32_t               memoryTypeBits,
	vk::MemoryPropertyFlags requiredFlags,
	vk::MemoryPropertyFlags    avoidFlags = vk::MemoryPropertyFlags{})
{
	auto indices = std::vector<uint32_t>();
	for (uint32_t i = 0; i < memoryProps.memoryTypeCount; ++i) {
		if ((static_cast<uint32_t>(1) << i) & memoryTypeBits) {
			if (((memoryProps.memoryTypes[i].propertyFlags & requiredFlags) == requiredFlags) &&
				((memoryProps.memoryTypes[i].propertyFlags & ~avoidFlags) == memoryProps.memoryTypes[i].propertyFlags)) {
				indices.push_back(i);
			}
		}
	}
	return indices;
};
auto CalcAlignedSize(vk::DeviceSize size, vk::DeviceSize alignment)->vk::DeviceSize {
	return ((size + alignment - 1) / alignment) * alignment;
}
int main(int argc,const char** argv)
{
	BulletRT::Core::VulkanContext::Initialize();
	{
		auto instance = std::unique_ptr<BulletRT::Core::VulkanInstance>(BulletRT::Core::VulkanInstance::Builder()
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
#ifdef _DEBUG
			.AddDebugUtilsMessenger(vk::DebugUtilsMessengerCreateInfoEXT()
				.setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
				.setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral)
				.setPfnUserCallback(DebugUtilsUserCallback)
			)
#endif
			.Build());

		{
			if (!instance->SupportExtension(VK_KHR_SURFACE_EXTENSION_NAME)) {
				throw std::runtime_error("Failed To Support Surface!");
			}
#ifdef  WIN32
			if (!instance->SupportExtension("VK_KHR_win32_surface")) {
				throw std::runtime_error("Failed To Support Win32 Surface!");
			}
#endif
#ifdef _DEBUG
			if (!instance->SupportExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
				throw std::runtime_error("Failed To Support Debug Utils!");
			}
#endif
		}

		auto deviceBuilders = BulletRT::Core::VulkanDevice::Builder::Enumerate(instance.get());
		
		auto gQueFamIndices = std::vector<uint32_t>();
		auto cQueFamIndices = std::vector<uint32_t>();
		auto tQueFamIndices = std::vector<uint32_t>();
		{
			auto queueFamilyProperties = deviceBuilders.front().GetPhysicalDevice().getQueueFamilyProperties();
			auto queueFamilyIndex = uint32_t(0);
			for (auto& queueFamilyProp : queueFamilyProperties) {
				bool isGraphics = (queueFamilyProp.queueFlags & vk::QueueFlagBits::eGraphics) == vk::QueueFlagBits::eGraphics;
				bool isCompute  = (queueFamilyProp.queueFlags & vk::QueueFlagBits::eCompute ) == vk::QueueFlagBits::eCompute;
				bool isTransfer = (queueFamilyProp.queueFlags & vk::QueueFlagBits::eTransfer) == vk::QueueFlagBits::eTransfer;
				if (isGraphics&&isCompute&&isTransfer)
				{
					gQueFamIndices.push_back(queueFamilyIndex);
				}
				else if (isCompute && isTransfer) {
					cQueFamIndices.push_back(queueFamilyIndex);
				}
				else if (isTransfer) {
					tQueFamIndices.push_back(queueFamilyIndex);
				}
				++queueFamilyIndex;
			}
		}

		auto device = std::unique_ptr<BulletRT::Core::VulkanDevice>(deviceBuilders.front()
			.SetExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME)
			.SetExtension(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME)
			.SetExtension(VK_KHR_RAY_QUERY_EXTENSION_NAME)
			.SetExtension(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME)
			.SetExtension(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME)
			.SetExtension(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME)
			.ResetFeatures<vk::PhysicalDeviceVulkan11Features>()
			.ResetFeatures<vk::PhysicalDeviceVulkan12Features>()
			.ResetFeatures<vk::PhysicalDeviceVulkan13Features>()
			.ResetFeatures<vk::PhysicalDeviceRayTracingPipelineFeaturesKHR>()
			.ResetFeatures<vk::PhysicalDeviceRayQueryFeaturesKHR>()
			.ResetFeatures<vk::PhysicalDeviceAccelerationStructureFeaturesKHR>()
			.SetQueueFamily(BulletRT::Core::VulkanQueueFamilyBuilder().SetQueueFamilyIndex(gQueFamIndices.front()).SetQueueCount(1))
			.SetQueueFamily(BulletRT::Core::VulkanQueueFamilyBuilder().SetQueueFamilyIndex(tQueFamIndices.front()).SetQueueCount(1))
			.SetQueueFamily(BulletRT::Core::VulkanQueueFamilyBuilder().SetQueueFamilyIndex(cQueFamIndices.front()).SetQueueCount(1))
			.Build()
		);

		{
			if (!device->SupportExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME)) {
				throw std::runtime_error("Failed To Support Swapchain!");
			}
		}

		auto gQueueFamily = device->AcquireQueueFamily(gQueFamIndices.front());
		auto cQueueFamily = device->AcquireQueueFamily(cQueFamIndices.front());
		auto tQueueFamily = device->AcquireQueueFamily(tQueFamIndices.front());

		auto gCmdPool = gQueueFamily->NewCommandPool();
		auto cCmdPool = cQueueFamily->NewCommandPool();
		auto tCmdPool = tQueueFamily->NewCommandPool();

		auto gQueues = gQueueFamily->GetQueues();
		auto cQueues = cQueueFamily->GetQueues();
		auto tQueues = tQueueFamily->GetQueues();


		auto stagingBuffer = std::unique_ptr<BulletRT::Core::VulkanBuffer>(
			BulletRT::Core::VulkanBuffer::Builder()
			.SetUsage(vk::BufferUsageFlagBits::eTransferSrc|vk::BufferUsageFlagBits::eShaderDeviceAddress)
			.SetSize(128*1024*1024)
			.SetQueueFamilyIndices({})
			.Build(device.get())
		);
		auto stagingMemory        = std::unique_ptr<BulletRT::Core::VulkanDeviceMemory>();
		auto stagingMemoryBuffer  = std::unique_ptr<BulletRT::Core::VulkanMemoryBuffer>();
		{
			auto sMemRequirements = stagingBuffer->QueryMemoryRequirements();
			auto sMemTypeIndex    = uint32_t(0);
			{
				auto memoryProperties = device->GetPhysicalDeviceVk().getMemoryProperties();
				auto sMemTypeIndices  = FindMemoryTypeIndices(memoryProperties, sMemRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostCached);
				if (sMemTypeIndices.empty()) {
					throw std::runtime_error("Failed To Find Memory Type!");
				}
				sMemTypeIndex = sMemTypeIndices.front();
			}
			stagingMemory = BulletRT::Core::VulkanDeviceMemory::Builder()
				.SetAllocationSize(sMemRequirements.size).SetMemoryTypeIndex(sMemTypeIndex)
				.SetMemoryAllocateFlagsInfo(
					vk::MemoryAllocateFlagsInfo().setFlags(vk::MemoryAllocateFlagBits::eDeviceAddress)
				)
				.Build(device.get()
			);
			stagingMemoryBuffer = BulletRT::Core::VulkanMemoryBuffer::Bind(stagingBuffer.get(), stagingMemory.get());
		}
		
		auto vMeshData = std::vector<float   >{
			-1.0f,0.0f,0.0f,
			 1.0f,0.0f,0.0f,
			 0.0f,1.0f,0.0f
		};
		auto iMeshData = std::vector<uint32_t>{
			0,1,2
		};

		auto vMeshBuffer = std::unique_ptr<BulletRT::Core::VulkanBuffer>(
			BulletRT::Core::VulkanBuffer::Builder()
			.SetUsage(vk::BufferUsageFlagBits::eVertexBuffer|vk::BufferUsageFlagBits::eStorageBuffer|vk::BufferUsageFlagBits::eShaderDeviceAddress|vk::BufferUsageFlagBits::eTransferDst)
			.SetSize(sizeof(vMeshData[0]) * std::size(vMeshData))
			.SetQueueFamilyIndices({})
			.Build(device.get())
		);
		auto iMeshBuffer = std::unique_ptr<BulletRT::Core::VulkanBuffer>(
			BulletRT::Core::VulkanBuffer::Builder()
			.SetUsage(vk::BufferUsageFlagBits::eIndexBuffer|vk::BufferUsageFlagBits::eStorageBuffer|vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eTransferDst)
			.SetSize(sizeof(iMeshData[0]) * std::size(iMeshData))
			.SetQueueFamilyIndices({})
			.Build(device.get())
		);

		auto meshMemories      = std::vector<std::unique_ptr<BulletRT::Core::VulkanDeviceMemory>>();
		auto vMeshMemoryBuffer = std::unique_ptr<BulletRT::Core::VulkanMemoryBuffer>();
		auto iMeshMemoryBuffer = std::unique_ptr<BulletRT::Core::VulkanMemoryBuffer>();

		{
			auto vMemRequirements = vMeshBuffer->QueryMemoryRequirements();
			auto iMemRequirements = iMeshBuffer->QueryMemoryRequirements();

			auto vMemTypeIndex = uint32_t(0);
			auto iMemTypeIndex = uint32_t(0);

			{
				auto memoryProperties = device->GetPhysicalDeviceVk().getMemoryProperties();
				auto vMemTypeIndices = FindMemoryTypeIndices(memoryProperties, vMemRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal, vk::MemoryPropertyFlagBits::eHostVisible);
				auto iMemTypeIndices = FindMemoryTypeIndices(memoryProperties, iMemRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal, vk::MemoryPropertyFlagBits::eHostVisible);
				if (vMemTypeIndices.empty()) {
					throw std::runtime_error("Failed To Find Valid Memory Type!\n");
				}
				if (iMemTypeIndices.empty()) {
					throw std::runtime_error("Failed To Find Valid Memory Type!\n");
				}
				vMemTypeIndex = vMemTypeIndices.front();
				iMemTypeIndex = iMemTypeIndices.front();
			}

			auto memoryVbIndex = static_cast<vk::DeviceSize>(0); 
			auto memoryIbIndex = static_cast<vk::DeviceSize>(0);
			auto memoryVbOffset = vk::DeviceSize(0);
			auto memoryIbOffset = vk::DeviceSize(0);
			if (vMemTypeIndex == iMemTypeIndex) {
				auto memoryVbRange   = CalcAlignedSize(vMemRequirements.size, vMemRequirements.alignment);
				auto memoryIbRange   = CalcAlignedSize(iMemRequirements.size, iMemRequirements.alignment);
				memoryVbOffset       = 0;
				memoryIbOffset       = CalcAlignedSize(memoryVbRange, iMemRequirements.alignment);
				auto memoryAllocSize = memoryIbOffset + memoryIbRange;
				meshMemories.push_back(BulletRT::Core::VulkanDeviceMemory::Builder()
					.SetAllocationSize(memoryAllocSize).SetMemoryTypeIndex(vMemTypeIndex)
					.SetMemoryAllocateFlagsInfo(
						vk::MemoryAllocateFlagsInfo().setFlags(vk::MemoryAllocateFlagBits::eDeviceAddress)
					).Build(device.get())
				);
			}
			else {
				meshMemories.push_back(BulletRT::Core::VulkanDeviceMemory::Builder()
					.SetAllocationSize(vMemRequirements.size).SetMemoryTypeIndex(vMemTypeIndex)
					.SetMemoryAllocateFlagsInfo(
						vk::MemoryAllocateFlagsInfo().setFlags(vk::MemoryAllocateFlagBits::eDeviceAddress)
					).Build(device.get())
				);
				meshMemories.push_back(BulletRT::Core::VulkanDeviceMemory::Builder()
					.SetAllocationSize(iMemRequirements.size).SetMemoryTypeIndex(iMemTypeIndex)
					.SetMemoryAllocateFlagsInfo(
						vk::MemoryAllocateFlagsInfo().setFlags(vk::MemoryAllocateFlagBits::eDeviceAddress)
					).Build(device.get())
				);
				memoryVbIndex = 0;
				memoryIbIndex = 1;
			}
			vMeshMemoryBuffer = BulletRT::Core::VulkanMemoryBuffer::Bind(vMeshBuffer.get(), meshMemories[memoryVbIndex].get(), memoryVbOffset);
			iMeshMemoryBuffer = BulletRT::Core::VulkanMemoryBuffer::Bind(iMeshBuffer.get(), meshMemories[memoryIbIndex].get(), memoryIbOffset);
		}

		std::cout << "vb address: " << vMeshMemoryBuffer->GetDeviceAddress().value() << std::endl;
		std::cout << "ib address: " << iMeshMemoryBuffer->GetDeviceAddress().value() << std::endl;
		{
			auto stagingMemVbSize   = vMeshMemoryBuffer->GetBufferSize();
			auto stagingMemIbSize   = iMeshMemoryBuffer->GetBufferSize();
			auto stagingMemVbOffset = 0;
			auto stagingMemIbOffset = stagingMemVbSize;
			auto stagingMemSize     = stagingMemVbSize + stagingMemIbSize;
			{
				char* pMapData = nullptr;
				if (stagingMemoryBuffer->Map((void**)&pMapData, stagingMemSize) == vk::Result::eSuccess) {
					std::memcpy(pMapData+stagingMemVbOffset, vMeshData.data(), stagingMemVbSize);
					std::memcpy(pMapData+stagingMemIbOffset, iMeshData.data(), stagingMemIbSize);
					stagingMemoryBuffer->Unmap();
				}
			}
			if (tCmdPool) {
				auto tCopyCmd    = tCmdPool->NewCommandBuffer(vk::CommandBufferLevel::ePrimary);
				auto tCmdBegInfo = vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
				if (tCopyCmd->GetCommandBufferVk().begin(&tCmdBegInfo) == vk::Result::eSuccess)
				{
					tCopyCmd->GetCommandBufferVk().copyBuffer(stagingBuffer->GetBufferVk(), vMeshMemoryBuffer->GetBufferVk(), vk::BufferCopy().setSrcOffset(stagingMemVbOffset).setDstOffset(vMeshMemoryBuffer->GetMemoryOffset()).setSize(vMeshMemoryBuffer->GetBufferSize()));
					tCopyCmd->GetCommandBufferVk().copyBuffer(stagingBuffer->GetBufferVk(), iMeshMemoryBuffer->GetBufferVk(), vk::BufferCopy().setSrcOffset(stagingMemIbOffset).setDstOffset(iMeshMemoryBuffer->GetMemoryOffset()).setSize(iMeshMemoryBuffer->GetBufferSize()));
					tCopyCmd->GetCommandBufferVk().end();
				}

			}
			else {
				auto gCopyCmd = tCmdPool->NewCommandBuffer(vk::CommandBufferLevel::ePrimary);
				auto gCmdBegInfo = vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
				if (gCopyCmd->GetCommandBufferVk().begin(&gCmdBegInfo) == vk::Result::eSuccess)
				{
					gCopyCmd->GetCommandBufferVk().copyBuffer(stagingBuffer->GetBufferVk(), vMeshMemoryBuffer->GetBufferVk(), vk::BufferCopy().setSrcOffset(stagingMemVbOffset).setDstOffset(vMeshMemoryBuffer->GetMemoryOffset()).setSize(vMeshMemoryBuffer->GetBufferSize()));
					gCopyCmd->GetCommandBufferVk().copyBuffer(stagingBuffer->GetBufferVk(), iMeshMemoryBuffer->GetBufferVk(), vk::BufferCopy().setSrcOffset(stagingMemIbOffset).setDstOffset(iMeshMemoryBuffer->GetMemoryOffset()).setSize(iMeshMemoryBuffer->GetBufferSize()));
					gCopyCmd->GetCommandBufferVk().end();
				}
				auto fence = device->GetDeviceVk().createFenceUnique(vk::FenceCreateInfo());
				auto commandBuffersVk = std::vector<vk::CommandBuffer>{ gCopyCmd->GetCommandBufferVk() };
				gQueues[0].GetQueueVk().submit(vk::SubmitInfo().setCommandBuffers(commandBuffersVk), fence.get());
			}
		}
		

		stagingMemory.reset();
		stagingBuffer.reset();

		meshMemories.clear();
		vMeshBuffer.reset();
		iMeshBuffer.reset();

		tCmdPool.reset();
		cCmdPool.reset();
		gCmdPool.reset();

		device.reset();
		instance.reset();
	}
	BulletRT::Core::VulkanContext::Terminate();
	return 0;
}
