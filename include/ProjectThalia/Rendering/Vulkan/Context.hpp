#pragma once
#include "Buffer.hpp"
#include "Device.hpp"
#include "Image.hpp"
#include "Instance.hpp"
#include "PhysicalDevice.hpp"
#include "ProjectThalia/Window.hpp"
#include "Sampler.hpp"

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
			void Destroy();
			void DrawFrame();

		private:

			struct UniformBufferObject
			{

			};

			struct CameraUBO
			{
					glm::mat4 model;
					glm::mat4 view;
					glm::mat4 proj;
			};

			struct TransformStorageBuffer
			{
					std::array<glm::mat4, 100> ModelMatrix {};
			};

			const std::vector<const char*> _validationLayers = {"VK_LAYER_KHRONOS_validation"};
			const std::vector<const char*> _deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

			Window* _window = nullptr; // TODO: Move to renderer

			std::unique_ptr<Device> _device;


			Instance       _instance;
			PhysicalDevice _physicalDevice;

			Buffer _quadModelBuffer;

			Image _image;

			Sampler _sampler;

			Buffer               _uniformBuffer;
			CameraUBO* _uniformBufferData;

			std::vector<Buffer> _modelMatrixStorageBuffers = std::vector<Buffer>(Device::MAX_FRAMES_IN_FLIGHT);

			//vk::DescriptorSetLayout _descriptorSetLayout;
			DescriptorSetManager::DescriptorSetAllocation descriptorSetAllocation;

			vk::CommandBuffer _commandBuffer;
			vk::Semaphore     _imageAvailableSemaphore;
			vk::Semaphore     _renderFinishedSemaphore;
			vk::Fence         _inFlightFence;

			vk::DescriptorPool _imGuiDescriptorPool;

			bool _frameBufferResized = false;

			void CreateInstance(SDL_Window* sdlWindow);
			void CreateCommandBuffers();
			void RecordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex);
			void CreateSyncObjects();
			void CreateDescriptorSets();
			void InitializeImGui();
	};
}
