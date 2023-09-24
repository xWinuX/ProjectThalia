#include "ProjectThalia/Rendering/VulkanContext.hpp"
#include "ProjectThalia/Debug/Log.hpp"
#include "ProjectThalia/ErrorHandler.hpp"

#include "ProjectThalia/IO/Stream.hpp"
#include "SDL2/SDL_vulkan.h"
#include <filesystem>
#include <vector>

namespace ProjectThalia::Rendering
{
	void VulkanContext::Initialize(Window& window)
	{
		Debug::Log::Info("Initalize");

		CreateInstance(window.GetSDLWindow());

		_physicalDevice = PhysicalDevice(_instance.GetVkInstance(), _instance.GetVkSurface(), _deviceExtensions, _validationLayers);

		_device = std::make_unique<Device>(Device(_physicalDevice));

		glm::ivec2 size = window.GetSize();

		_device->CreateSwapchain(_instance.GetVkSurface(), size);
		_device->CreateRenderPass();
		_device->CreatePipeline("main",
								{{"res/shaders/Debug.vert.spv", vk::ShaderStageFlagBits::eVertex},
								 {"res/shaders/Debug.frag.spv", vk::ShaderStageFlagBits::eFragment}});

		CreateFrameBuffers();

		CreateCommandBuffers();

		CreateSyncObjects();
	}

	void VulkanContext::CreateSyncObjects()
	{
		vk::SemaphoreCreateInfo semaphoreCreateInfo = vk::SemaphoreCreateInfo();
		vk::FenceCreateInfo     fenceCreateInfo     = vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled);

		_imageAvailableSemaphore = _device->GetVkDevice().createSemaphore(semaphoreCreateInfo);
		_renderFinishedSemaphore = _device->GetVkDevice().createSemaphore(semaphoreCreateInfo);
		_inFlightFence           = _device->GetVkDevice().createFence(fenceCreateInfo);
	}

	void VulkanContext::RecordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex)
	{
		vk::CommandBufferBeginInfo commandBufferBeginInfo = vk::CommandBufferBeginInfo({}, nullptr);

		commandBuffer.begin(commandBufferBeginInfo);

		vk::ClearValue          clearColor          = vk::ClearValue({0.0f, 0.0f, 0.0f, 1.0f});
		vk::RenderPassBeginInfo renderPassBeginInfo = vk::RenderPassBeginInfo(_device->GetRenderPass().GetVkRenderPass(),
																			  _swapChainFrameBuffers[imageIndex],
																			  {{0, 0}, _device->GetSwapchain().GetExtend()},
																			  1,
																			  &clearColor);

		_commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
		_commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _device->GetPipeline().GetVkPipeline());

		vk::Viewport viewport = vk::Viewport(0,
											 0,
											 static_cast<float>(_device->GetSwapchain().GetExtend().width),
											 static_cast<float>(_device->GetSwapchain().GetExtend().height),
											 0.0f,
											 1.0f);
		_commandBuffer.setViewport(0, 1, &viewport);

		vk::Rect2D scissor = vk::Rect2D({0, 0}, _device->GetSwapchain().GetExtend());
		_commandBuffer.setScissor(0, 1, &scissor);

		commandBuffer.draw(3, 1, 0, 0);

		commandBuffer.endRenderPass();
		commandBuffer.end();
	}

	void VulkanContext::CreateCommandBuffers()
	{
		vk::CommandPoolCreateInfo commandPoolCreateInfo = vk::CommandPoolCreateInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
																					_physicalDevice.GetQueueFamilyIndices().graphicsFamily.value());

		_commandPool = _device->GetVkDevice().createCommandPool(commandPoolCreateInfo);

		vk::CommandBufferAllocateInfo commandBufferAllocateInfo = vk::CommandBufferAllocateInfo(_commandPool, vk::CommandBufferLevel::ePrimary, 1);

		_commandBuffer = _device->GetVkDevice().allocateCommandBuffers(commandBufferAllocateInfo)[0];
	}

	void VulkanContext::CreateFrameBuffers()
	{
		_swapChainFrameBuffers.resize(_device->GetSwapchain().GetImageViews().size());

		for (size_t i = 0; i < _device->GetSwapchain().GetImageViews().size(); i++)
		{
			vk::ImageView attachments[] = {_device->GetSwapchain().GetImageViews()[i]};

			vk::FramebufferCreateInfo framebufferInfo = vk::FramebufferCreateInfo({},
																				  _device->GetRenderPass().GetVkRenderPass(),
																				  1,
																				  attachments,
																				  _device->GetSwapchain().GetExtend().width,
																				  _device->GetSwapchain().GetExtend().height,
																				  1);


			_swapChainFrameBuffers[i] = _device->GetVkDevice().createFramebuffer(framebufferInfo);
		}
	}

	void VulkanContext::Destroy()
	{
		_device->GetVkDevice().waitIdle();

		_device->GetVkDevice().destroy(_imageAvailableSemaphore);
		_device->GetVkDevice().destroy(_renderFinishedSemaphore);
		_device->GetVkDevice().destroy(_inFlightFence);
		_device->GetVkDevice().destroy(_commandPool);

		for (const vk::Framebuffer& frameBuffer : _swapChainFrameBuffers) { _device->GetVkDevice().destroy(frameBuffer); }

		_device->Destroy();
		_instance.Destroy();
	}

	void VulkanContext::DrawFrame()
	{
		Debug::Log::Info("Draw frame");
		vk::Result waitForFencesResult = _device->GetVkDevice().waitForFences(1, &_inFlightFence, vk::True, UINT64_MAX);
		vk::Result resetFencesResult   = _device->GetVkDevice().resetFences(1, &_inFlightFence);

		vk::ResultValue<uint32_t> imageIndex = _device->GetVkDevice().acquireNextImageKHR(_device->GetSwapchain().GetVkSwapchain(),
																						  UINT64_MAX,
																						  _imageAvailableSemaphore,
																						  VK_NULL_HANDLE);

		_commandBuffer.reset({});
		RecordCommandBuffer(_commandBuffer, imageIndex.value);

		vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
		vk::SubmitInfo         submitInfo   = vk::SubmitInfo(_imageAvailableSemaphore, waitStages, _commandBuffer, _renderFinishedSemaphore);


		_device->GetGraphicsQueue().submit(submitInfo, _inFlightFence);

		vk::PresentInfoKHR presentInfo = vk::PresentInfoKHR(_renderFinishedSemaphore, _device->GetSwapchain().GetVkSwapchain(), imageIndex.value, nullptr);

		vk::Result presentResult = _device->GetPresentQueue().presentKHR(presentInfo);
	}

	const vk::Device& VulkanContext::GetDevice() { return _device->GetVkDevice(); }

	void VulkanContext::CreateInstance(SDL_Window* sdlWindow)
	{
		uint32_t extensionCount;
		SDL_Vulkan_GetInstanceExtensions(sdlWindow, &extensionCount, nullptr);

		std::vector<const char*> extensionNames = std::vector<const char*>(extensionCount, nullptr);
		SDL_Vulkan_GetInstanceExtensions(sdlWindow, &extensionCount, extensionNames.data());

		vk::ApplicationInfo applicationInfo = vk::ApplicationInfo("Project Thalia",
																  VK_MAKE_VERSION(1, 0, 0),
																  "No Engine",
																  VK_MAKE_VERSION(1, 0, 0),
																  VK_API_VERSION_1_3);

		_instance = Instance(extensionNames, _validationLayers, applicationInfo);

		// Create surface from sdl
		VkSurfaceKHR surfaceHandle         = VK_NULL_HANDLE;
		SDL_bool     surfaceCreationResult = SDL_Vulkan_CreateSurface(sdlWindow, static_cast<VkInstance>(_instance.GetVkInstance()), &surfaceHandle);
		if (surfaceCreationResult == SDL_FALSE) { ErrorHandler::ThrowRuntimeError("Failed to create SDL Vulkan surface!"); }

		_instance.SetVkSurface(surfaceHandle);
	}
}