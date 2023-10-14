#pragma once
#include "Buffer.hpp"
#include "Device.hpp"
#include "Image.hpp"
#include "Instance.hpp"
#include "PhysicalDevice.hpp"
#include "ProjectThalia/Window.hpp"

#include <SDL2/SDL.h>
#include <vulkan/vulkan.hpp>
#include <optional>
#include <vector>

namespace ProjectThalia::Rendering::Vulkan
{
	class Context
	{
		public:
			Context() = default;

			void Initialize(Window* window);
			void Destroy();
			void DrawFrame();

		private:
			struct UniformBufferObject
			{
					glm::mat4 model;
					glm::mat4 view;
					glm::mat4 proj;
			};

			const int MAX_FRAMES_IN_FLIGHT = 1;

			const std::vector<const char*> _validationLayers = {"VK_LAYER_KHRONOS_validation"};
			const std::vector<const char*> _deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

			Window* _window = nullptr; // TODO: Move to renderer

			std::unique_ptr<Device> _device;

			Instance       _instance;
			PhysicalDevice _physicalDevice;

			Buffer _quadModelBuffer;

			Image _image;

			vk::Sampler _sampler;

			std::vector<Buffer>               _uniformBuffers    = std::vector<Buffer>(MAX_FRAMES_IN_FLIGHT);
			std::vector<UniformBufferObject*> _uniformBufferData = std::vector<UniformBufferObject*>(MAX_FRAMES_IN_FLIGHT);

			vk::DescriptorSetLayout        _descriptorSetLayout;
			vk::DescriptorPool             _descriptorPool;
			std::vector<vk::DescriptorSet> _descriptorSets = std::vector<vk::DescriptorSet>(MAX_FRAMES_IN_FLIGHT);

			std::vector<vk::CommandBuffer> _commandBuffer           = std::vector<vk::CommandBuffer>(MAX_FRAMES_IN_FLIGHT);
			std::vector<vk::Semaphore>     _imageAvailableSemaphore = std::vector<vk::Semaphore>(MAX_FRAMES_IN_FLIGHT);
			std::vector<vk::Semaphore>     _renderFinishedSemaphore = std::vector<vk::Semaphore>(MAX_FRAMES_IN_FLIGHT);
			std::vector<vk::Fence>         _inFlightFence           = std::vector<vk::Fence>(MAX_FRAMES_IN_FLIGHT);

			vk::DescriptorPool _imGuiDescriptorPool;

			uint32_t _currentFrame       = 0;
			bool     _frameBufferResized = false;

			void CreateInstance(SDL_Window* sdlWindow);
			void CreateCommandBuffers();
			void RecordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex);
			void CreateSyncObjects();
			void CreateDescriptorSets();
			void InitializeImGui();
	};
}
