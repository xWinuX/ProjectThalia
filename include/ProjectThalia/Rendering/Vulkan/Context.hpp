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
			void Initialize(Window* window);
			void Destroy();
			void DrawFrame();

		private:
			const int MAX_FRAMES_IN_FLIGHT = 2;

			const std::vector<const char*> _validationLayers = {"VK_LAYER_KHRONOS_validation"};
			const std::vector<const char*> _deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

			Window* _window; // TODO: Move to renderer

			std::unique_ptr<Device> _device;

			Instance       _instance;
			PhysicalDevice _physicalDevice;

			vk::CommandPool   _commandPool;
			vk::CommandBuffer _commandBuffer;

			vk::Semaphore _imageAvailableSemaphore;
			vk::Semaphore _renderFinishedSemaphore;
			vk::Fence     _inFlightFence;

			bool _frameBufferResized = false;

			void CreateInstance(SDL_Window* sdlWindow);
			void CreateCommandBuffers();
			void RecordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex);
			void CreateSyncObjects();
	};
}
