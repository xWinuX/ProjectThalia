#pragma once
#include "Device.hpp"
#include "Instance.hpp"
#include "PhysicalDevice.hpp"
#include "ProjectThalia/Window.hpp"
#include "SDL2/SDL.h"
#include "vulkan/vulkan.hpp"
#include <optional>
#include <vector>

namespace ProjectThalia::Rendering::Vulkan
{
	class Context
	{
		public:
			void Initialize(Window& sdlWindow);
			void Destroy();
			void DrawFrame();

		private:
			const std::vector<const char*> _validationLayers = {"VK_LAYER_KHRONOS_validation"};
			const std::vector<const char*> _deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

			std::unique_ptr<Device> _device;

			Instance       _instance;
			PhysicalDevice _physicalDevice;

			vk::CommandPool   _commandPool;
			vk::CommandBuffer _commandBuffer;

			vk::Semaphore _imageAvailableSemaphore;
			vk::Semaphore _renderFinishedSemaphore;
			vk::Fence     _inFlightFence;

			void CreateInstance(SDL_Window* sdlWindow);
			void CreateCommandBuffers();
			void RecordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex);
			void CreateSyncObjects();
	};
}
