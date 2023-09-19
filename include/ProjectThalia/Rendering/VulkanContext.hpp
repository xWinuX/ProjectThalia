#pragma once

#include "SDL2/SDL.h"
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
			void Initialize(SDL_Window* sdlWindow);
			void Destroy();
			void DrawFrame();

			[[nodiscard]] static const vk::Device& GetDevice();

		private:
			const std::vector<const char*> _validationLayers = {"VK_LAYER_KHRONOS_validation"};
			const std::vector<const char*> _deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

			static vk::Device _device;

			vk::PhysicalDevice _physicalDevice          = VK_NULL_HANDLE;
			vk::Queue          _graphicsQueue           = VK_NULL_HANDLE;
			vk::Queue          _presentQueue            = VK_NULL_HANDLE;
			vk::Instance       _instance                = VK_NULL_HANDLE;
			vk::SurfaceKHR     _surface                 = VK_NULL_HANDLE;
			vk::SwapchainKHR   _swapChain               = VK_NULL_HANDLE;
			vk::PipelineLayout _pipelineLayout          = VK_NULL_HANDLE;
			vk::Pipeline       _pipeline                = VK_NULL_HANDLE;
			vk::RenderPass     _renderPass              = VK_NULL_HANDLE;
			vk::CommandPool    _commandPool             = VK_NULL_HANDLE;
			vk::CommandBuffer  _commandBuffer           = VK_NULL_HANDLE;
			vk::Semaphore      _imageAvailableSemaphore = VK_NULL_HANDLE;
			vk::Semaphore      _renderFinishedSemaphore = VK_NULL_HANDLE;
			vk::Fence          _inFlightFence           = VK_NULL_HANDLE;

			vk::SurfaceFormatKHR       _swapChainImageFormat;
			vk::Extent2D               _swapChainExtent;
			std::vector<vk::Image>     _swapChainImages;
			std::vector<vk::ImageView> _swapChainImageViews;

			std::vector<VkFramebuffer> _swapChainFrameBuffers;

			QueueFamilyIndices      _queueFamilyIndices;
			SwapChainSupportDetails _swapChainSupportDetails;

			void             CreateInstance(SDL_Window* sdlWindow);
			void             CreateSurface(SDL_Window* sdlWindow);
			void             SelectPhysicalDevice();
			void             CreateLogicalDevice();
			void             CreateSwapChain(SDL_Window* sdlWindow);
			void             CreateImageViews();
			vk::ShaderModule CreateShaderModule(const std::vector<char>& code);
			void             CreateGraphicsPipeline();
			void             CreateFrameBuffers();
			void             CreateCommandBuffers();
			void             RecordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex);
			void             CreateSyncObjects();
	};

	inline vk::Device VulkanContext::_device = VK_NULL_HANDLE;
}
