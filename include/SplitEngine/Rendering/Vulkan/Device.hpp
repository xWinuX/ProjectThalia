#pragma once

#include "Image.hpp"
#include "Pipeline.hpp"
#include "RenderPass.hpp"
#include "Swapchain.hpp"

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

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

			[[nodiscard]] const PhysicalDevice&                     GetPhysicalDevice() const;
			[[nodiscard]] const Swapchain&                          GetSwapchain() const;
			[[nodiscard]] const RenderPass&                         GetRenderPass() const;
			[[nodiscard]] const vk::Device&                         GetVkDevice() const;
			[[nodiscard]] const vk::Queue&                          GetGraphicsQueue() const;
			[[nodiscard]] const vk::Queue&                          GetPresentQueue() const;
			[[nodiscard]] const vk::PhysicalDeviceMemoryProperties& GetMemoryProperties() const;
			[[nodiscard]] const vk::CommandPool&                    GetGraphicsCommandPool() const;

			[[nodiscard]] const vk::Semaphore&                      GetImageAvailableSemaphore() const;
			[[nodiscard]] const vk::Semaphore&                      GetRenderFinishedSemaphore() const;
			[[nodiscard]] const vk::Fence&                          GetInFlightFence() const;
			[[nodiscard]] const vk::CommandBuffer&                  GetCommandBuffer() const;

			uint32_t* GetCurrentFramePtr();

			[[nodiscard]] int FindMemoryTypeIndex(const vk::MemoryRequirements& memoryRequirements, const vk::Flags<vk::MemoryPropertyFlagBits>& memoryPropertyFlags) const;

			[[nodiscard]] vk::CommandBuffer BeginOneshotCommands() const;
			void                            EndOneshotCommands(vk::CommandBuffer commandBuffer) const;

		private:
			uint32_t _currentFrame = 0;

			vk::Device      _vkDevice;
			PhysicalDevice& _physicalDevice;

			std::unique_ptr<Swapchain> _swapchain;

			RenderPass _renderPass;

			vk::CommandPool _graphicsCommandPool;

			vk::Queue _graphicsQueue;
			vk::Queue _presentQueue;

			InFlightResource<vk::Semaphore>     _imageAvailableSemaphore{};
			InFlightResource<vk::Semaphore>     _renderFinishedSemaphore{};
			InFlightResource<vk::Fence>         _inFlightFence{};
			InFlightResource<vk::CommandBuffer> _commandBuffer{};

			vk::PhysicalDeviceMemoryProperties _memoryProperties;

			void CreateLogicalDevice(PhysicalDevice& physicalDevice);
			void CreateRenderPass();
			void CreateGraphicsCommandPool(PhysicalDevice& physicalDevice);
			void CreateSyncObjects();
			void CreateCommandBuffers();
	};
}
