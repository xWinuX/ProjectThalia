#pragma once
#include "Device.hpp"
#include "PhysicalDevice.hpp"
#include "Pipeline.hpp"
#include "ProjectThalia/Window.hpp"
#include "SDL2/SDL.h"
#include "Swapchain.hpp"
#include "vulkan/vulkan.hpp"
#include <optional>
#include <vector>

namespace ProjectThalia::Rendering
{
	struct QueueFamilyIndices
	{
		public:
			std::optional<uint32_t> graphicsFamily;
			std::optional<uint32_t> presentFamily;

			[[nodiscard]] bool isComplete() const { return graphicsFamily.has_value() && presentFamily.has_value(); }
	};

	struct SwapChainSupportDetails
	{
		public:
			vk::SurfaceCapabilitiesKHR        capabilities;
			std::vector<vk::SurfaceFormatKHR> formats;
			std::vector<vk::PresentModeKHR>   presentModes;
	};

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

			PhysicalDevice _physicalDevice;
			Swapchain      _swapchain;
			Pipeline       _pipeline;

			vk::Instance   _instance;
			vk::SurfaceKHR _surface;

			vk::CommandPool   _commandPool;
			vk::CommandBuffer _commandBuffer;

			vk::Semaphore _imageAvailableSemaphore;
			vk::Semaphore _renderFinishedSemaphore;
			vk::Fence     _inFlightFence;

			std::vector<VkFramebuffer> _swapChainFrameBuffers;

			void CreateInstance(SDL_Window* sdlWindow);
			void CreateSurface(SDL_Window* sdlWindow);
			void CreateFrameBuffers();
			void CreateCommandBuffers();
			void RecordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex);
			void CreateSyncObjects();
	};

	inline std::unique_ptr<Device> VulkanContext::_device = nullptr;
}
