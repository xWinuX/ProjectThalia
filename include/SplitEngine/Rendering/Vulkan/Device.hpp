#pragma once

#include "Image.hpp"
#include "Pipeline.hpp"
#include "RenderPass.hpp"
#include "Swapchain.hpp"

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

#include "QueueFamily.hpp"

namespace SplitEngine::Rendering::Vulkan
{
	class PhysicalDevice;

	class Device
	{
		public:
			static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

			explicit Device(PhysicalDevice& physicalDevice);

			void Destroy();

			void AdvanceFrame();

			void WaitForIdle();

			void CreateSwapchain(vk::SurfaceKHR surfaceKhr, glm::ivec2 size);
			void DestroySwapchain();

			template<typename T>
			InFlightResource<T> CreateInFlightResource(bool singleInstance = false, T defaultValue = {})
			{
				return InFlightResource<T>(GetCurrentFramePtr(singleInstance), std::vector<T>(singleInstance ? 1 : MAX_FRAMES_IN_FLIGHT, defaultValue));
			}

			template<typename T>
			InFlightResource<T> CreateInFlightResource(std::vector<T>&& data, bool singleInstance = false) { return InFlightResource<T>(GetCurrentFramePtr(singleInstance), std::move(data)); }

			[[nodiscard]] const PhysicalDevice&                     GetPhysicalDevice() const;
			[[nodiscard]] const Swapchain&                          GetSwapchain() const;
			[[nodiscard]] const RenderPass&                         GetRenderPass() const;
			[[nodiscard]] const vk::Device&                         GetVkDevice() const;
			[[nodiscard]] const QueueFamily&                        GetQueueFamily(const QueueType queueFamilyType) const;
			[[nodiscard]] QueueFamily&                              GetQueueFamily(const QueueType queueFamilyType);
			[[nodiscard]] const vk::PhysicalDeviceMemoryProperties& GetMemoryProperties() const;

			[[nodiscard]] const vk::Semaphore& GetImageAvailableSemaphore() const;
			[[nodiscard]] const vk::Semaphore& GetRenderFinishedSemaphore() const;
			[[nodiscard]] const vk::Fence&     GetInFlightFence() const;

			uint32_t* GetCurrentFramePtr(bool staticPtr = false);

			[[nodiscard]] int FindMemoryTypeIndex(const vk::MemoryRequirements& memoryRequirements, const vk::Flags<vk::MemoryPropertyFlagBits>& memoryPropertyFlags) const;

		private:
			uint32_t _currentStaticFrame = 0;
			uint32_t _currentFrame       = 0;

			vk::Device      _vkDevice;
			PhysicalDevice& _physicalDevice;

			std::unique_ptr<Swapchain> _swapchain;

			RenderPass _renderPass;

			std::vector<QueueFamily>                                                _queueFamilies;
			std::array<uint32_t, static_cast<size_t>(QueueType::MAX_VALUE)> _queueFamilyLookup{};

			InFlightResource<vk::Semaphore> _imageAvailableSemaphore{};
			InFlightResource<vk::Semaphore> _renderFinishedSemaphore{};
			InFlightResource<vk::Fence>     _inFlightFence{};

			InFlightResource<vk::CommandBuffer> _graphicsCommandBuffer{};
			InFlightResource<vk::CommandBuffer> _computeCommandBuffer{};

			vk::PhysicalDeviceMemoryProperties _memoryProperties;

			void CreateLogicalDevice(PhysicalDevice& physicalDevice);
			void CreateRenderPass();
			void CreateSyncObjects();
	};
}
