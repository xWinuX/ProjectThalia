#include "SplitEngine/Rendering/Vulkan/Instance.hpp"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_vulkan.h>
#include <SDL_vulkan.h>

#include "SplitEngine/ErrorHandler.hpp"
#include "SplitEngine/Window.hpp"
#include "SplitEngine/Rendering/Vulkan/Device.hpp"
#include "SplitEngine/Rendering/Vulkan/PhysicalDevice.hpp"

namespace SplitEngine::Rendering::Vulkan
{
	Instance* Instance::_instance = nullptr;

	void Instance::CreateAllocator() { _allocator = std::make_unique<Allocator>(*this); }

	Instance::Instance(Window& window, ApplicationInfo& applicationInfo, RenderingSettings&& renderingSettings):
		_renderingSettings(std::move(renderingSettings))
	{
		const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };

		uint32_t extensionCount = 0;
		SDL_Vulkan_GetInstanceExtensions(window.GetSDLWindow(), &extensionCount, nullptr);

		std::vector<const char*> extensionNames = std::vector<const char*>(extensionCount, nullptr);
		SDL_Vulkan_GetInstanceExtensions(window.GetSDLWindow(), &extensionCount, extensionNames.data());

		const vk::ApplicationInfo vkApplicationInfo = vk::ApplicationInfo(applicationInfo.Name.c_str(),
		                                                                  VK_MAKE_VERSION(applicationInfo.MajorVersion, applicationInfo.MinorVersion, applicationInfo.PatchVersion),
		                                                                  "Split Engine",
		                                                                  VK_MAKE_VERSION(1, 0, 0),
		                                                                  VK_API_VERSION_1_3);

		_vkInstance = vk::createInstance(vk::InstanceCreateInfo({}, &vkApplicationInfo, validationLayers, extensionNames));

		VkSurfaceKHR   surfaceHandle         = VK_NULL_HANDLE;
		const SDL_bool surfaceCreationResult = SDL_Vulkan_CreateSurface(window.GetSDLWindow(), _vkInstance, &surfaceHandle);
		if (surfaceCreationResult == SDL_FALSE) { ErrorHandler::ThrowRuntimeError("Failed to create SDL Vulkan surface!"); }

		_vkSurface = surfaceHandle;

		const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME };

		_physicalDevice = std::make_unique<PhysicalDevice>(*this, deviceExtensions, validationLayers);

		CreateAllocator();

		_defaultSampler = _allocator->AllocateSampler({});

		// ImageLoader
		constexpr std::byte fuchsia[] = { static_cast<const std::byte>(255), static_cast<const std::byte>(0), static_cast<const std::byte>(255) };

		_defaultImage = Image(&_physicalDevice->GetDevice(), std::begin(fuchsia), 4, { 1, 1, 1 }, {});

		_physicalDevice->GetDevice().CreateSwapchain(_vkSurface, window.GetSize());

		InitializeImGui(window);

		_instance = this;
	}

	const vk::Instance& Instance::GetVkInstance() const { return _vkInstance; }

	const vk::SurfaceKHR& Instance::GetVkSurface() const { return _vkSurface; }

	void Instance::Destroy()
	{
		_physicalDevice->GetDevice().WaitForIdle();

		Pipeline::_globalDescriptorManager.DeallocateDescriptorSet(Pipeline::_globalDescriptorSetAllocation);
		Pipeline::_globalDescriptorManager.Destroy();

		ImGui_ImplVulkan_Shutdown();

		_physicalDevice->GetDevice().GetVkDevice().destroy(_imGuiDescriptorPool);

		_defaultImage.Destroy();
		_physicalDevice->GetDevice().GetVkDevice().destroy(*_defaultSampler);
		_physicalDevice->GetDevice().DestroySwapchain();

		_allocator->Destroy();
		_physicalDevice->Destroy();

		_vkInstance.destroy(_vkSurface);
		_vkInstance.destroy();
	}

	Instance&          Instance::Get() { return *_instance; }
	const Image&       Instance::GetDefaultImage() const { return _defaultImage; }
	const vk::Sampler* Instance::GetDefaultSampler() const { return _defaultSampler; }

	const RenderingSettings& Instance::GetRenderingSettings() const { return _renderingSettings; }

	PhysicalDevice& Instance::GetPhysicalDevice() const { return *_physicalDevice; }

	Allocator& Instance::GetAllocator() const { return *_allocator; }

	void Instance::InitializeImGui(Window& window)
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

		Device& device       = GetPhysicalDevice().GetDevice();
		_imGuiDescriptorPool = device.GetVkDevice().createDescriptorPool(descriptorPoolCreateInfo);

		ImGui::CreateContext();

		ImGui_ImplSDL2_InitForVulkan(window.GetSDLWindow());

		ImGui_ImplVulkan_InitInfo initInfo = {};
		initInfo.Instance                  = _vkInstance;
		initInfo.PhysicalDevice            = _physicalDevice->GetVkPhysicalDevice();
		initInfo.Device                    = device.GetVkDevice();
		initInfo.Queue                     = device.GetGraphicsQueue();
		initInfo.DescriptorPool            = _imGuiDescriptorPool;
		initInfo.MinImageCount             = 3;
		initInfo.ImageCount                = 3;
		initInfo.MSAASamples               = VK_SAMPLE_COUNT_1_BIT;

		ImGui_ImplVulkan_Init(&initInfo, device.GetRenderPass().GetVkRenderPass());

		const vk::CommandBuffer commandBuffer = device.BeginOneshotCommands();

		ImGui_ImplVulkan_CreateFontsTexture();

		device.EndOneshotCommands(commandBuffer);

		ImGui_ImplVulkan_DestroyFontsTexture();
	}
}
