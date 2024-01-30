#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include "SplitEngine/Rendering/Vulkan/Context.hpp"
#include "SplitEngine/Rendering/Vulkan/Device.hpp"

#include "SplitEngine/Application.hpp"
#include "SplitEngine/ErrorHandler.hpp"


#include <chrono>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_vulkan.h>
#include <vector>
#include <SDL2/SDL_vulkan.h>

namespace SplitEngine::Rendering::Vulkan
{
	std::unique_ptr<Device> Context::_device;

	void Context::Initialize(Window* window)
	{
		_window = window; // TODO: Move to renderer

		CreateInstance(_window->GetSDLWindow());

		_physicalDevice = PhysicalDevice(_instance.GetVkInstance(), _instance.GetVkSurface(), _deviceExtensions, _validationLayers);

		_device = std::make_unique<Device>(Device(_physicalDevice));

		_device->CreateAllocator(_instance);

		_device->CreateGraphicsCommandPool();

		CreateCommandBuffers();

		_device->CreateRenderPass();

		_device->CreateSwapchain(_instance.GetVkSurface(), _window->GetSize());

		_device->CreateDefaultResources();

		CreateSyncObjects();

		InitializeImGui();
	}

	void Context::CreateSyncObjects()
	{
		constexpr vk::SemaphoreCreateInfo semaphoreCreateInfo = vk::SemaphoreCreateInfo();
		constexpr vk::FenceCreateInfo     fenceCreateInfo     = vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled);

		std::vector<vk::Semaphore> imageAvailableSemaphores;
		std::vector<vk::Semaphore> renderingFinishedSemaphores;
		std::vector<vk::Fence>     inFlightFences;

		for (int i = 0; i < Device::MAX_FRAMES_IN_FLIGHT; ++i)
		{
			imageAvailableSemaphores.push_back(_device->GetVkDevice().createSemaphore(semaphoreCreateInfo));
			renderingFinishedSemaphores.push_back(_device->GetVkDevice().createSemaphore(semaphoreCreateInfo));
			inFlightFences.push_back(_device->GetVkDevice().createFence(fenceCreateInfo));
		}

		_imageAvailableSemaphore = InFlightResource<vk::Semaphore>(_device->GetCurrentFramePtr(), std::move(imageAvailableSemaphores));
		_renderFinishedSemaphore = InFlightResource<vk::Semaphore>(_device->GetCurrentFramePtr(), std::move(renderingFinishedSemaphores));
		_inFlightFence           = InFlightResource<vk::Fence>(_device->GetCurrentFramePtr(), std::move(inFlightFences));
	}

	void Context::CreateCommandBuffers()
	{
		const vk::CommandBufferAllocateInfo commandBufferAllocateInfo = vk::CommandBufferAllocateInfo(_device->GetGraphicsCommandPool(),
		                                                                                              vk::CommandBufferLevel::ePrimary,
		                                                                                              Device::MAX_FRAMES_IN_FLIGHT);

		std::vector<vk::CommandBuffer> commandBuffers = _device->GetVkDevice().allocateCommandBuffers(commandBufferAllocateInfo);
		_commandBuffer                                = InFlightResource<vk::CommandBuffer>(_device->GetCurrentFramePtr(), std::move(commandBuffers));
	}

	void Context::Destroy()
	{
		WaitForIdle();

		// Deallocate global descriptor set
		Pipeline::_globalDescriptorManager.DeallocateDescriptorSet(Pipeline::_globalDescriptorSetAllocation);
		Pipeline::_globalDescriptorManager.Destroy();

		for (int i = 0; i < Device::MAX_FRAMES_IN_FLIGHT; ++i)
		{
			_device->GetVkDevice().destroy(_imageAvailableSemaphore[i]);
			_device->GetVkDevice().destroy(_renderFinishedSemaphore[i]);
			_device->GetVkDevice().destroy(_inFlightFence[i]);
		}

		ImGui_ImplVulkan_Shutdown();

		_device->GetVkDevice().destroy(_imGuiDescriptorPool);

		_device->Destroy();
		_instance.Destroy();
	}

	void Context::WaitForIdle() { _device->GetVkDevice().waitIdle(); }

	void Context::CreateInstance(SDL_Window* sdlWindow)
	{
		uint32_t extensionCount = 0;
		SDL_Vulkan_GetInstanceExtensions(sdlWindow, &extensionCount, nullptr);

		std::vector<const char*> extensionNames = std::vector<const char*>(extensionCount, nullptr);
		SDL_Vulkan_GetInstanceExtensions(sdlWindow, &extensionCount, extensionNames.data());

		const vk::ApplicationInfo applicationInfo = vk::ApplicationInfo(Application::GetApplicationInfo().Name.c_str(),
		                                                                VK_MAKE_VERSION(Application::GetApplicationInfo().MajorVersion,
		                                                                                Application::GetApplicationInfo().MinorVersion,
		                                                                                Application::GetApplicationInfo().PatchVersion),
		                                                                "Split Engine",
		                                                                VK_MAKE_VERSION(1, 0, 0),
		                                                                VK_API_VERSION_1_3);

		_instance = Instance(extensionNames, _validationLayers, applicationInfo);

		// Create surface from sdl
		VkSurfaceKHR   surfaceHandle         = VK_NULL_HANDLE;
		const SDL_bool surfaceCreationResult = SDL_Vulkan_CreateSurface(sdlWindow, _instance.GetVkInstance(), &surfaceHandle);
		if (surfaceCreationResult == SDL_FALSE) { ErrorHandler::ThrowRuntimeError("Failed to create SDL Vulkan surface!"); }

		_instance.SetVkSurface(surfaceHandle);
	}

	void Context::InitializeImGui()
	{
		vk::DescriptorPoolSize poolSizes[] = {
			{ vk::DescriptorType::eSampler, 1000 },
			{ vk::DescriptorType::eCombinedImageSampler, 1000 },
			{ vk::DescriptorType::eSampledImage, 1000 },
			{ vk::DescriptorType::eStorageImage, 1000 },
			{ vk::DescriptorType::eUniformTexelBuffer, 1000 },
			{ vk::DescriptorType::eStorageTexelBuffer, 1000 },
			{ vk::DescriptorType::eUniformBuffer, 1000 },
			{ vk::DescriptorType::eStorageBuffer, 1000 },
			{ vk::DescriptorType::eUniformBufferDynamic, 1000 },
			{ vk::DescriptorType::eStorageBufferDynamic, 1000 },
			{ vk::DescriptorType::eInputAttachment, 1000 }
		};

		const vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo = vk::DescriptorPoolCreateInfo(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, 1000, poolSizes);

		_imGuiDescriptorPool = _device->GetVkDevice().createDescriptorPool(descriptorPoolCreateInfo);

		ImGui::CreateContext();

		ImGui_ImplSDL2_InitForVulkan(_window->GetSDLWindow());

		ImGui_ImplVulkan_InitInfo initInfo = {};
		initInfo.Instance                  = _instance.GetVkInstance();
		initInfo.PhysicalDevice            = _device->GetPhysicalDevice().GetVkPhysicalDevice();
		initInfo.Device                    = _device->GetVkDevice();
		initInfo.Queue                     = _device->GetGraphicsQueue();
		initInfo.DescriptorPool            = _imGuiDescriptorPool;
		initInfo.MinImageCount             = 3;
		initInfo.ImageCount                = 3;
		initInfo.MSAASamples               = VK_SAMPLE_COUNT_1_BIT;

		ImGui_ImplVulkan_Init(&initInfo, _device->GetRenderPass().GetVkRenderPass());

		const vk::CommandBuffer commandBuffer = _device->BeginOneshotCommands();

		ImGui_ImplVulkan_CreateFontsTexture();

		_device->EndOneshotCommands(commandBuffer);

		ImGui_ImplVulkan_DestroyFontsTexture();
	}

	Device* Context::GetDevice() { return _device.get(); }

	const vk::Semaphore& Context::GetImageAvailableSemaphore() const { return _imageAvailableSemaphore.Get(); }

	const vk::Semaphore& Context::GetRenderFinishedSemaphore() const { return _renderFinishedSemaphore.Get(); }

	const vk::Fence& Context::GetInFlightFence() const { return _inFlightFence.Get(); }

	const Instance& Context::GetInstance() const { return _instance; }

	const vk::CommandBuffer& Context::GetCommandBuffer() const { return _commandBuffer.Get(); }
}
