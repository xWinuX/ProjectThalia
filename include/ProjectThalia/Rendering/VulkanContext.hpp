#pragma once
#include "Device.hpp"
#include "Instance.hpp"
#include "PhysicalDevice.hpp"
#include "ProjectThalia/Window.hpp"
#include "SDL2/SDL.h"
#include "vulkan/vulkan.hpp"
#include <optional>
#include <vector>

namespace ProjectThalia::Rendering
{
	class VulkanContext
	{
		public:
			void Initialize(Window& sdlWindow);
			void Destroy();
			void DrawFrame();

			static const vk::Device& GetDevice();

		private:
			const std::vector<const char*> _validationLayers = {"VK_LAYER_KHRONOS_validation"};
			const std::vector<const char*> _deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

			static std::unique_ptr<Device> _device;

			Instance       _instance;
			PhysicalDevice _physicalDevice;

			vk::CommandPool   _commandPool;
			vk::CommandBuffer _commandBuffer;

			vk::Semaphore _imageAvailableSemaphore;
			vk::Semaphore _renderFinishedSemaphore;
			vk::Fence     _inFlightFence;

			std::vector<VkFramebuffer> _swapChainFrameBuffers;

			void CreateInstance(SDL_Window* sdlWindow);
			void CreateFrameBuffers();
			void CreateCommandBuffers();
			void RecordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex);
			void CreateSyncObjects();
	};

	inline std::unique_ptr<Device> VulkanContext::_device = nullptr;
}
