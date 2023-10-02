#pragma once
#include "Buffer.hpp"
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
			Context() {}

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

			Buffer _quadModelBuffer;

			std::vector<vk::CommandBuffer> _commandBuffer           = std::vector<vk::CommandBuffer>(MAX_FRAMES_IN_FLIGHT);
			std::vector<vk::Semaphore>     _imageAvailableSemaphore = std::vector<vk::Semaphore>(MAX_FRAMES_IN_FLIGHT);
			std::vector<vk::Semaphore>     _renderFinishedSemaphore = std::vector<vk::Semaphore>(MAX_FRAMES_IN_FLIGHT);
			std::vector<vk::Fence>         _inFlightFence           = std::vector<vk::Fence>(MAX_FRAMES_IN_FLIGHT);

			uint32_t _currentFrame       = 0;
			bool     _frameBufferResized = false;

			void CreateInstance(SDL_Window* sdlWindow);
			void CreateCommandBuffers();
			void RecordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex);
			void CreateSyncObjects();
	};
}
