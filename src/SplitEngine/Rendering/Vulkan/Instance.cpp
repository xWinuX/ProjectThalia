#include "SplitEngine/Rendering/Vulkan/Instance.hpp"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include <SDL_vulkan.h>

#include "SplitEngine/ErrorHandler.hpp"
#include "SplitEngine/Window.hpp"
#include "SplitEngine/Rendering/Vulkan/Device.hpp"
#include "SplitEngine/Rendering/Vulkan/PhysicalDevice.hpp"

namespace SplitEngine::Rendering::Vulkan
{
	Instance* Instance::_instance = nullptr;

	void Instance::CreateAllocator() { _allocator = std::make_unique<Allocator>(*this); }

	Instance::Instance(Window& window, ApplicationInfo& applicationInfo, ShaderParserSettings&& shaderParserSettings, RenderingSettings&& renderingSettings):
		_shaderParserSettings(std::move(shaderParserSettings)),
		_renderingSettings(std::move(renderingSettings))
	{
		uint32_t extensionCount = 0;
		SDL_Vulkan_GetInstanceExtensions(window.GetSDLWindow(), &extensionCount, nullptr);

		std::vector<const char*> extensionNames = std::vector<const char*>(extensionCount, nullptr);
		SDL_Vulkan_GetInstanceExtensions(window.GetSDLWindow(), &extensionCount, extensionNames.data());

		const vk::ApplicationInfo vkApplicationInfo = vk::ApplicationInfo(applicationInfo.Name.c_str(),
		                                                                  VK_MAKE_VERSION(applicationInfo.MajorVersion, applicationInfo.MinorVersion, applicationInfo.PatchVersion),
		                                                                  "Split Engine",
		                                                                  VK_MAKE_VERSION(1, 0, 0),
		                                                                  VK_API_VERSION_1_3);

		std::vector<const char*> validationLayers;
		if (renderingSettings.UseVulkanValidationLayers) { validationLayers.push_back("VK_LAYER_KHRONOS_validation"); }
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

		_instance = this;
	}

	const vk::Instance& Instance::GetVkInstance() const { return _vkInstance; }

	const vk::SurfaceKHR& Instance::GetVkSurface() const { return _vkSurface; }

	void Instance::Destroy()
	{
		Pipeline::_globalDescriptorManager.DeallocateDescriptorSet(Pipeline::_globalDescriptorSetAllocation);
		Pipeline::_globalDescriptorManager.Destroy();

		for (auto& [name, descriptor]: DescriptorSetAllocator::_sharedDescriptors) { descriptor.Buffer.Destroy(); }

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

	const RenderingSettings&    Instance::GetRenderingSettings() const { return _renderingSettings; }
	const ShaderParserSettings& Instance::GetShaderParserSettings() const { return _shaderParserSettings; }

	PhysicalDevice& Instance::GetPhysicalDevice() const { return *_physicalDevice; }

	Allocator& Instance::GetAllocator() const { return *_allocator; }
}
