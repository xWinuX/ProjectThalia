#pragma once
#include "Buffer.hpp"
#include "DescriptorSetManager.hpp"
#include "Device.hpp"
#include "Instance.hpp"
#include "PhysicalDevice.hpp"
#include "ProjectThalia/Window.hpp"

#include <SDL2/SDL.h>
#include <optional>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace ProjectThalia::Rendering::Vulkan
{
	class Context
	{
		public:
			Context() = default;

			void Initialize(Window* window);
			static void WaitForIdle() ;
			void Destroy();
			void DrawFrame();

			static Device* GetDevice();

			[[nodiscard]] const vk::Semaphore&     GetImageAvailableSemaphore() const;
			[[nodiscard]] const vk::Semaphore&     GetRenderFinishedSemaphore() const;
			[[nodiscard]] const vk::Fence&         GetInFlightFence() const;
			[[nodiscard]] const Instance&          GetInstance() const;
			[[nodiscard]] const vk::CommandBuffer& GetCommandBuffer() const;

		private:
			const std::vector<const char*> _validationLayers = {"VK_LAYER_KHRONOS_validation"};
			const std::vector<const char*> _deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

			Window* _window = nullptr; // TODO: Move to renderer

			static std::unique_ptr<Device> _device;

			Instance       _instance;
			PhysicalDevice _physicalDevice;

			vk::CommandBuffer _commandBuffer;
			vk::Semaphore     _imageAvailableSemaphore;
			vk::Semaphore     _renderFinishedSemaphore;
			vk::Fence         _inFlightFence;

			vk::DescriptorPool _imGuiDescriptorPool;


			void CreateInstance(SDL_Window* sdlWindow);
			void CreateCommandBuffers();
			void CreateSyncObjects();
			void InitializeImGui();
	};
}
