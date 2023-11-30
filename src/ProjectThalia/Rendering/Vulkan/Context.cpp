#include "ProjectThalia/Rendering/Vulkan/Context.hpp"

#include "ProjectThalia/Debug/Log.hpp"
#include "ProjectThalia/ErrorHandler.hpp"
#include "ProjectThalia/IO/ImageFile.hpp"
#include "ProjectThalia/IO/Stream.hpp"
#include "ProjectThalia/Rendering/Vertex.hpp"

#include <SDL2/SDL_vulkan.h>
#include <chrono>
#include <filesystem>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_vulkan.h>
#include <vector>

namespace ProjectThalia::Rendering::Vulkan
{
	void Context::Initialize(Window* window)
	{
		_window = window; // TODO: Move to renderer

		CreateInstance(_window->GetSDLWindow());

		_physicalDevice = PhysicalDevice(_instance.GetVkInstance(), _instance.GetVkSurface(), _deviceExtensions, _validationLayers);

		_device = std::make_unique<Device>(Device(_physicalDevice));

		_device->CreateRenderPass();
		_device->CreateSwapchain(_instance.GetVkSurface(), _window->GetSize());

		_device->CreateGraphicsCommandPool();

		_device->CreateAllocator(_instance);

		CreateCommandBuffers();

		IO::ImageFile textureImage = IO::ImageFile("res/textures/floppa.png", IO::ImageFile::RGBA);

		_image = Image(_device.get(),
					   reinterpret_cast<const char*>(textureImage.GetPixels()),
					   textureImage.GetTotalImageSize(),
					   {static_cast<uint32_t>(textureImage.GetWidth()), static_cast<uint32_t>(textureImage.GetHeight()), 1});


		_device->CreatePipeline("main",
								{{"res/shaders/Debug.vert.spv", vk::ShaderStageFlagBits::eVertex},
								 {"res/shaders/Debug.frag.spv", vk::ShaderStageFlagBits::eFragment}});

		CreateDescriptorSets();

		CreateSyncObjects();

		const std::vector<VertexPosition2DColorUV> vertices = {{{-0.25f, 0.25f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},  // Top Left
															   {{0.25f, 0.25f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},   // Top Right
															   {{-0.25f, -0.25f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}, // Bottom Left
															   {{0.25f, -0.25f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}}; // Bottom Right

		const std::vector<uint16_t> indices = {0, 1, 2, 2, 1, 3};

		_quadModelBuffer = Buffer::CreateStagedModelBuffer(_device.get(), vertices, indices);


		//TransformStorageBuffer transformStorageBuffer {};
		/*
        for (int i = 0; i < Device::MAX_FRAMES_IN_FLIGHT; ++i)
        {
            _modelMatrixStorageBuffers[i] = Buffer::CreateStorageBuffer(_device.get(), &transformStorageBuffer);
        }*/

		InitializeImGui();

		_window->OnResize.Add([this](int width, int height) {
			_frameBufferResized = true;
		});
	}

	void Context::CreateDescriptorSets()
	{
		descriptorSetAllocation = _device->GetPipeline().GetDescriptorSetManager().AllocateDescriptorSet();

		_sampler = Sampler(_device.get(), {});

		_uniformBuffer     = Buffer::CreateUniformBuffer<CameraUBO>(_device.get(), nullptr);
		_uniformBufferData = _uniformBuffer.GetMappedData<CameraUBO>();

		vk::DescriptorBufferInfo uniformDescriptorBufferInfo = vk::DescriptorBufferInfo(_uniformBuffer.GetVkBuffer(), 0, sizeof(CameraUBO));
		vk::DescriptorImageInfo  descriptorImageInfo         = vk::DescriptorImageInfo(_sampler.GetVkSampler(), _image.GetView(), _image.GetLayout());

		std::vector<vk::WriteDescriptorSet> writeDescriptorSets = {
				vk::WriteDescriptorSet(descriptorSetAllocation.DescriptorSet,
									   0,
									   0,
									   vk::DescriptorType::eUniformBuffer,
									   nullptr,
									   uniformDescriptorBufferInfo,
									   nullptr),
				vk::WriteDescriptorSet(descriptorSetAllocation.DescriptorSet,
									   1,
									   0,
									   vk::DescriptorType::eCombinedImageSampler,
									   descriptorImageInfo,
									   nullptr,
									   nullptr),
		};

		_device->GetVkDevice().updateDescriptorSets(writeDescriptorSets, nullptr);
	}

	void Context::CreateSyncObjects()
	{
		vk::SemaphoreCreateInfo semaphoreCreateInfo = vk::SemaphoreCreateInfo();
		vk::FenceCreateInfo     fenceCreateInfo     = vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled);

		_imageAvailableSemaphore = _device->GetVkDevice().createSemaphore(semaphoreCreateInfo);
		_renderFinishedSemaphore = _device->GetVkDevice().createSemaphore(semaphoreCreateInfo);
		_inFlightFence           = _device->GetVkDevice().createFence(fenceCreateInfo);
	}

	void Context::RecordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex)
	{
		vk::CommandBufferBeginInfo commandBufferBeginInfo = vk::CommandBufferBeginInfo({}, nullptr);

		commandBuffer.begin(commandBufferBeginInfo);

		vk::ClearValue          clearColor          = vk::ClearValue({0.0f, 0.2f, 0.5f, 1.0f});
		vk::RenderPassBeginInfo renderPassBeginInfo = vk::RenderPassBeginInfo(_device->GetRenderPass().GetVkRenderPass(),
																			  _device->GetSwapchain().GetFrameBuffers()[imageIndex],
																			  {{0, 0}, _device->GetSwapchain().GetExtend()},
																			  1,
																			  &clearColor);

		commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _device->GetPipeline().GetVkPipeline());

		vk::Buffer     vertexBuffers[] = {_quadModelBuffer.GetVkBuffer()};
		vk::DeviceSize offsets[]       = {0};

		commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
		commandBuffer.bindIndexBuffer(_quadModelBuffer.GetVkBuffer(), _quadModelBuffer.GetSizeInBytes(0), vk::IndexType::eUint16);

		vk::Viewport viewport = vk::Viewport(0,
											 static_cast<float>(_device->GetSwapchain().GetExtend().height),
											 static_cast<float>(_device->GetSwapchain().GetExtend().width),
											 -static_cast<float>(_device->GetSwapchain().GetExtend().height),
											 0.0f,
											 1.0f);
		commandBuffer.setViewport(0, 1, &viewport);

		vk::Rect2D scissor = vk::Rect2D({0, 0}, _device->GetSwapchain().GetExtend());
		commandBuffer.setScissor(0, 1, &scissor);

		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
										 _device->GetPipeline().GetLayout(),
										 0,
										 1,
										 &descriptorSetAllocation.DescriptorSet,
										 0,
										 nullptr);
		commandBuffer.drawIndexed(_quadModelBuffer.GetBufferElementNum(1), 1, 0, 0, 0);

		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

		commandBuffer.endRenderPass();
		commandBuffer.end();
	}

	void Context::CreateCommandBuffers()
	{
		vk::CommandBufferAllocateInfo commandBufferAllocateInfo = vk::CommandBufferAllocateInfo(_device->GetGraphicsCommandPool(),
																								vk::CommandBufferLevel::ePrimary,
																								1);

		_commandBuffer = _device->GetVkDevice().allocateCommandBuffers(commandBufferAllocateInfo)[0];
	}

	void Context::DrawFrame()
	{
		_device->GetVkDevice().waitForFences(_inFlightFence, vk::True, UINT64_MAX);

		vk::ResultValue<uint32_t> imageIndexResult = _device->GetVkDevice().acquireNextImageKHR(_device->GetSwapchain().GetVkSwapchain(),
																								UINT64_MAX,
																								_imageAvailableSemaphore,
																								VK_NULL_HANDLE);

		if (imageIndexResult.result == vk::Result::eErrorOutOfDateKHR)
		{
			_device->GetVkDevice().waitIdle();
			_device->CreateSwapchain(_instance.GetVkSurface(), _window->GetSize());

			return;
		}
		else if (imageIndexResult.result != vk::Result::eSuccess && imageIndexResult.result != vk::Result::eSuboptimalKHR)
		{
			ErrorHandler::ThrowRuntimeError("failed to acquire swap chain image!");
		}

		_device->GetVkDevice().resetFences(_inFlightFence);

		static auto startTime = std::chrono::high_resolution_clock::now();

		auto  currentTime = std::chrono::high_resolution_clock::now();
		float time        = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		_uniformBufferData->model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		_uniformBufferData->view  = glm::lookAt(glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		_uniformBufferData->proj  = glm::perspective(glm::radians(45.0f),
                                                    static_cast<float>(_device->GetSwapchain().GetExtend().width) /
                                                            -static_cast<float>(_device->GetSwapchain().GetExtend().height),
                                                    0.1f,
                                                    10.0f);

		_commandBuffer.reset({});
		RecordCommandBuffer(_commandBuffer, imageIndexResult.value);

		vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
		vk::SubmitInfo         submitInfo   = vk::SubmitInfo(_imageAvailableSemaphore, waitStages, _commandBuffer, _renderFinishedSemaphore);

		_device->GetGraphicsQueue().submit(submitInfo, _inFlightFence);

		vk::PresentInfoKHR presentInfo = vk::PresentInfoKHR(_renderFinishedSemaphore,
															_device->GetSwapchain().GetVkSwapchain(),
															imageIndexResult.value,
															nullptr);

		vk::Result presentResult = _device->GetPresentQueue().presentKHR(presentInfo);

		if (presentResult == vk::Result::eErrorOutOfDateKHR || presentResult == vk::Result::eSuboptimalKHR || _frameBufferResized)
		{
			_device->GetVkDevice().waitIdle();
			_frameBufferResized = false;
			_device->CreateSwapchain(_instance.GetVkSurface(), _window->GetSize());
		}
		else if (presentResult != vk::Result::eSuccess) { ErrorHandler::ThrowRuntimeError("failed to present swap chain image!"); }
	}

	void Context::Destroy()
	{
		_device->GetVkDevice().waitIdle();

		_device->GetVkDevice().destroy(_imageAvailableSemaphore);
		_device->GetVkDevice().destroy(_renderFinishedSemaphore);
		_device->GetVkDevice().destroy(_inFlightFence);

		_quadModelBuffer.Destroy();

		_uniformBuffer.Destroy();

		_image.Destroy();

		_sampler.Destroy();

		_device->GetVkDevice().destroy(_imGuiDescriptorPool);
		ImGui_ImplVulkan_Shutdown();

		_device->Destroy();
		_instance.Destroy();
	}

	void Context::CreateInstance(SDL_Window* sdlWindow)
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

	void Context::InitializeImGui()
	{
		vk::DescriptorPoolSize poolSizes[] = {{vk::DescriptorType::eSampler, 1000},
											  {vk::DescriptorType::eCombinedImageSampler, 1000},
											  {vk::DescriptorType::eSampledImage, 1000},
											  {vk::DescriptorType::eStorageImage, 1000},
											  {vk::DescriptorType::eUniformTexelBuffer, 1000},
											  {vk::DescriptorType::eStorageTexelBuffer, 1000},
											  {vk::DescriptorType::eUniformBuffer, 1000},
											  {vk::DescriptorType::eStorageBuffer, 1000},
											  {vk::DescriptorType::eUniformBufferDynamic, 1000},
											  {vk::DescriptorType::eStorageBufferDynamic, 1000},
											  {vk::DescriptorType::eInputAttachment, 1000}};

		vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo = vk::DescriptorPoolCreateInfo({}, 1000, poolSizes);

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

		vk::CommandBuffer commandBuffer = _device->BeginOneshotCommands();

		ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);

		_device->EndOneshotCommands(commandBuffer);

		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}
}