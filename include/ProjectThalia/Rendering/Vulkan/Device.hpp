#pragma once

#include "Allocator.hpp"
#include "Image.hpp"
#include "Instance.hpp"
#include "PhysicalDevice.hpp"
#include "Pipeline.hpp"
#include "RenderPass.hpp"
#include "Swapchain.hpp"

#include <glm/glm.hpp>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

namespace ProjectThalia::Rendering::Vulkan
{
	class Device
	{
		public:
			static const int MAX_FRAMES_IN_FLIGHT = 1;

			explicit Device(PhysicalDevice& physicalDevice);

			void CreateAllocator(const Instance& instance);
			void CreateSwapchain(vk::SurfaceKHR surfaceKhr, glm::ivec2 size);
			void CreateRenderPass();
			void CreateGraphicsCommandPool();
			void CreateDefaultResources();

			void Destroy();

			[[nodiscard]] const Swapchain&                          GetSwapchain() const;
			[[nodiscard]] const RenderPass&                         GetRenderPass() const;
			[[nodiscard]] const vk::Device&                         GetVkDevice() const;
			[[nodiscard]] const PhysicalDevice&                     GetPhysicalDevice() const;
			[[nodiscard]] const vk::Queue&                          GetGraphicsQueue() const;
			[[nodiscard]] const vk::Queue&                          GetPresentQueue() const;
			[[nodiscard]] const vk::PhysicalDeviceMemoryProperties& GetMemoryProperties() const;
			[[nodiscard]] const vk::CommandPool&                    GetGraphicsCommandPool() const;
			[[nodiscard]] Allocator&                                GetAllocator();
			[[nodiscard]] const Image&                              GetDefaultImage() const;
			[[nodiscard]] const vk::Sampler&                        GetDefaultSampler() const;

			[[nodiscard]] int FindMemoryTypeIndex(const vk::MemoryRequirements&                memoryRequirements,
												  const vk::Flags<vk::MemoryPropertyFlagBits>& memoryPropertyFlags) const;

			[[nodiscard]] vk::CommandBuffer BeginOneshotCommands() const;
			void                            EndOneshotCommands(vk::CommandBuffer commandBuffer) const;

		private:
			vk::Device      _vkDevice;
			PhysicalDevice& _physicalDevice;
			Allocator       _allocator;

			Swapchain  _swapchain;
			RenderPass _renderPass;

			Image       _defaultImages;
			vk::Sampler _defaultSampler;

			vk::CommandPool _graphicsCommandPool;

			vk::Queue _graphicsQueue;
			vk::Queue _presentQueue;

			vk::PhysicalDeviceMemoryProperties _memoryProperties;
	};
}
