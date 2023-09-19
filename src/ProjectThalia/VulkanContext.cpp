#include "ProjectThalia/VulkanContext.hpp"
#include "ProjectThalia/Debug/Log.hpp"
#include "ProjectThalia/ErrorHandler.hpp"

#include <SDL2/SDL_vulkan.h>
#include <filesystem>
#include <format>
#include <fstream>
#include <set>
#include <string>
#include <vector>

namespace ProjectThalia::Vulkan
{
	static std::vector<char> readFile(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		std::filesystem::path path = std::filesystem::current_path();

		Debug::Log::Info(std::format("{}", path.generic_string()));

		if (!file.is_open()) { ErrorHandler::ThrowRuntimeError("failed to open file!"); }

		size_t            fileSize = (size_t) file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	vk::ShaderModule VulkanContext::CreateShaderModule(const std::vector<char>& code)
	{
		vk::ShaderModuleCreateInfo createInfo = vk::ShaderModuleCreateInfo({}, code.size(), reinterpret_cast<const uint32_t*>(code.data()));

		return _device.createShaderModule(createInfo);
	}

	void VulkanContext::Initialize(SDL_Window* sdlWindow)
	{
		Debug::Log::Info("Initalize");

		CreateInstance(sdlWindow);

		CreateSurface(sdlWindow);

		SelectPhysicalDevice();

		CreateLogicalDevice();

		CreateSwapChain(sdlWindow);

		CreateImageViews();

		CreateGraphicsPipeline();

		CreateFrameBuffers();

		CreateCommandBuffers();

		CreateSyncObjects();
	}

	void VulkanContext::CreateSyncObjects()
	{
		vk::SemaphoreCreateInfo semaphoreCreateInfo = vk::SemaphoreCreateInfo();
		vk::FenceCreateInfo     fenceCreateInfo     = vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled);

		_imageAvailableSemaphore = _device.createSemaphore(semaphoreCreateInfo);
		_renderFinishedSemaphore = _device.createSemaphore(semaphoreCreateInfo);
		_inFlightFence           = _device.createFence(fenceCreateInfo);
	}

	void VulkanContext::RecordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex)
	{
		vk::CommandBufferBeginInfo commandBufferBeginInfo = vk::CommandBufferBeginInfo({}, nullptr);

		commandBuffer.begin(commandBufferBeginInfo);

		vk::ClearValue          clearColor          = vk::ClearValue({0.0f, 0.0f, 0.0f, 1.0f});
		vk::RenderPassBeginInfo renderPassBeginInfo = vk::RenderPassBeginInfo(_renderPass,
																			  _swapChainFrameBuffers[imageIndex],
																			  {{0, 0}, _swapChainExtent},
																			  1,
																			  &clearColor);

		_commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
		_commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _pipeline);

		vk::Viewport viewport = vk::Viewport(0, 0, static_cast<float>(_swapChainExtent.width), static_cast<float>(_swapChainExtent.height), 0.0f, 1.0f);
		_commandBuffer.setViewport(0, 1, &viewport);

		vk::Rect2D scissor = vk::Rect2D({0, 0}, _swapChainExtent);
		_commandBuffer.setScissor(0, 1, &scissor);

		commandBuffer.draw(3, 1, 0, 0);

		commandBuffer.endRenderPass();
		commandBuffer.end();
	}

	void VulkanContext::CreateCommandBuffers()
	{
		vk::CommandPoolCreateInfo commandPoolCreateInfo = vk::CommandPoolCreateInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
																					_queueFamilyIndices.graphicsFamily.value());

		_commandPool = _device.createCommandPool(commandPoolCreateInfo);

		vk::CommandBufferAllocateInfo commandBufferAllocateInfo = vk::CommandBufferAllocateInfo(_commandPool, vk::CommandBufferLevel::ePrimary, 1);

		_commandBuffer = _device.allocateCommandBuffers(commandBufferAllocateInfo)[0];
	}

	void VulkanContext::CreateFrameBuffers()
	{
		_swapChainFrameBuffers.resize(_swapChainImageViews.size());

		for (size_t i = 0; i < _swapChainImageViews.size(); i++)
		{
			vk::ImageView attachments[] = {_swapChainImageViews[i]};

			vk::FramebufferCreateInfo
					framebufferInfo = vk::FramebufferCreateInfo({}, _renderPass, 1, attachments, _swapChainExtent.width, _swapChainExtent.height, 1);


			_swapChainFrameBuffers[i] = _device.createFramebuffer(framebufferInfo);
		}
	}

	void VulkanContext::CreateGraphicsPipeline()
	{
		auto vertShaderCode = readFile("res/shaders/Debug.vert.spv");
		auto fragShaderCode = readFile("res/shaders/Debug.frag.spv");

		vk::ShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
		vk::ShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);

		vk::PipelineShaderStageCreateInfo vertShaderStageCreateInfo = vk::PipelineShaderStageCreateInfo({},
																										vk::ShaderStageFlagBits::eVertex,
																										vertShaderModule,
																										"main");
		vk::PipelineShaderStageCreateInfo fragShaderStageCreateInfo = vk::PipelineShaderStageCreateInfo({},
																										vk::ShaderStageFlagBits::eFragment,
																										fragShaderModule,
																										"main");

		vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageCreateInfo, fragShaderStageCreateInfo};

		std::vector<vk::DynamicState> dynamicStates = {
				vk::DynamicState::eViewport,
				vk::DynamicState::eScissor,
		};

		vk::PipelineDynamicStateCreateInfo     dynamicStateCreateInfo     = vk::PipelineDynamicStateCreateInfo({}, dynamicStates);
		vk::PipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = vk::PipelineVertexInputStateCreateInfo({}, 0, nullptr, 0, nullptr);

		vk::PipelineInputAssemblyStateCreateInfo assemblyStateCreateInfo = vk::PipelineInputAssemblyStateCreateInfo({},
																													vk::PrimitiveTopology::eTriangleList,
																													vk::False);

		vk::Viewport viewport = vk::Viewport(0, 0, static_cast<float>(_swapChainExtent.width), static_cast<float>(_swapChainExtent.height), 0.0f, 1.0f);
		vk::Rect2D   scissor  = vk::Rect2D({0, 0}, _swapChainExtent);

		vk::PipelineViewportStateCreateInfo viewportStateCreateInfo = vk::PipelineViewportStateCreateInfo({}, 1, &viewport, 1, &scissor);

		vk::PipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = vk::PipelineRasterizationStateCreateInfo({},
																														 vk::False,
																														 vk::False,
																														 vk::PolygonMode::eFill,
																														 vk::CullModeFlagBits::eBack,
																														 vk::FrontFace::eClockwise,
																														 vk::False,
																														 0.0f,
																														 0.0f,
																														 0.0f,
																														 1.0f);

		vk::PipelineMultisampleStateCreateInfo multisampleStateCreateInfo = vk::PipelineMultisampleStateCreateInfo({},
																												   vk::SampleCountFlagBits::e1,
																												   vk::False,
																												   1.0f,
																												   nullptr,
																												   vk::False,
																												   vk::False);


		vk::PipelineColorBlendAttachmentState colorBlendAttachmentState = vk::PipelineColorBlendAttachmentState(vk::False,
																												vk::BlendFactor::eOne,
																												vk::BlendFactor::eZero,
																												vk::BlendOp::eAdd,
																												vk::BlendFactor::eOne,
																												vk::BlendFactor::eZero,
																												vk::BlendOp::eAdd,
																												vk::ColorComponentFlagBits::eR |
																														vk::ColorComponentFlagBits::eG |
																														vk::ColorComponentFlagBits::eB |
																														vk::ColorComponentFlagBits::eA);

		vk::PipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = vk::PipelineColorBlendStateCreateInfo({},
																												vk::False,
																												vk::LogicOp::eCopy,
																												1,
																												&colorBlendAttachmentState,
																												{0.0f, 0.0f, 0.0f, 0.0f});

		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo({}, 0, nullptr, 0, nullptr);

		_pipelineLayout = _device.createPipelineLayout(pipelineLayoutCreateInfo);

		vk::AttachmentDescription colorAttachment = vk::AttachmentDescription({},
																			  _swapChainImageFormat.format,
																			  vk::SampleCountFlagBits::e1,
																			  vk::AttachmentLoadOp::eClear,
																			  vk::AttachmentStoreOp::eStore,
																			  vk::AttachmentLoadOp::eDontCare,
																			  vk::AttachmentStoreOp::eDontCare,
																			  vk::ImageLayout::eUndefined,
																			  vk::ImageLayout::ePresentSrcKHR);

		vk::AttachmentReference colorAttachmentReference = vk::AttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal);
		vk::SubpassDescription  subpass                  = vk::SubpassDescription({}, vk::PipelineBindPoint::eGraphics, {}, {}, 1, &colorAttachmentReference);


		vk::SubpassDependency subpassDependency = vk::SubpassDependency(vk::SubpassExternal,
																		{},
																		vk::PipelineStageFlagBits::eColorAttachmentOutput,
																		vk::PipelineStageFlagBits::eColorAttachmentOutput,
																		{},
																		vk::AccessFlagBits::eColorAttachmentWrite);

		vk::RenderPassCreateInfo renderPassCreateInfo = vk::RenderPassCreateInfo({}, 1, &colorAttachment, 1, &subpass, 1, &subpassDependency);

		_renderPass = _device.createRenderPass(renderPassCreateInfo);

		vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo = vk::GraphicsPipelineCreateInfo({},
																								   2,
																								   shaderStages,
																								   &vertexInputStateCreateInfo,
																								   &assemblyStateCreateInfo,
																								   nullptr,
																								   &viewportStateCreateInfo,
																								   &rasterizationStateCreateInfo,
																								   &multisampleStateCreateInfo,
																								   nullptr,
																								   &colorBlendStateCreateInfo,
																								   &dynamicStateCreateInfo,
																								   _pipelineLayout,
																								   _renderPass,
																								   0,
																								   VK_NULL_HANDLE,
																								   -1);


		vk::ResultValue<vk::Pipeline> graphicsPipelineResult = _device.createGraphicsPipeline(VK_NULL_HANDLE, graphicsPipelineCreateInfo);
		if (graphicsPipelineResult.result != vk::Result::eSuccess) { ErrorHandler::ThrowRuntimeError("Failed to create graphics pipeline!"); }

		_pipeline = graphicsPipelineResult.value;
	}

	void VulkanContext::CreateImageViews()
	{
		_swapChainImageViews.resize(_swapChainImages.size());

		for (int i = 0; i < _swapChainImages.size(); ++i)
		{
			vk::ImageViewCreateInfo imageViewCreateInfo = vk::ImageViewCreateInfo({},
																				  _swapChainImages[i],
																				  vk::ImageViewType::e2D,
																				  _swapChainImageFormat.format,
																				  {vk::ComponentSwizzle::eIdentity,
																				   vk::ComponentSwizzle::eIdentity,
																				   vk::ComponentSwizzle::eIdentity,
																				   vk::ComponentSwizzle::eIdentity},
																				  {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});

			_swapChainImageViews[i] = _device.createImageView(imageViewCreateInfo);
		}
	}

	void VulkanContext::CreateSwapChain(SDL_Window* sdlWindow)
	{
		// Select surface format
		_swapChainImageFormat = _swapChainSupportDetails.formats[0];
		for (const auto& availableFormat : _swapChainSupportDetails.formats)
		{
			if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
			{
				_swapChainImageFormat = availableFormat;
			}
		}

		// Select present mode
		vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;
		for (const auto& availablePresentMode : _swapChainSupportDetails.presentModes)
		{
			if (availablePresentMode == vk::PresentModeKHR::eMailbox)
			{
				presentMode = vk::PresentModeKHR::eMailbox;
				break;
			}
		}

		// Select swap extend
		const vk::SurfaceCapabilitiesKHR& capabilities = _swapChainSupportDetails.capabilities;
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) { _swapChainExtent = capabilities.currentExtent; }
		else
		{
			int width, height;

			SDL_GetWindowSize(sdlWindow, &width, &height);

			_swapChainExtent = vk::Extent2D(width, height);

			_swapChainExtent.width  = std::clamp(_swapChainExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			_swapChainExtent.height = std::clamp(_swapChainExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
		}

		// Select image count
		uint32_t imageCount = _swapChainSupportDetails.capabilities.minImageCount + 1;
		if (_swapChainSupportDetails.capabilities.maxImageCount > 0 && imageCount > _swapChainSupportDetails.capabilities.maxImageCount)
		{
			imageCount = _swapChainSupportDetails.capabilities.maxImageCount;
		}

		// Create swap chain
		vk::SwapchainCreateInfoKHR swapChainCreateInfo = vk::SwapchainCreateInfoKHR({},
																					_surface,
																					imageCount,
																					_swapChainImageFormat.format,
																					_swapChainImageFormat.colorSpace,
																					_swapChainExtent,
																					1,
																					vk::ImageUsageFlagBits::eColorAttachment);

		if (_queueFamilyIndices.graphicsFamily != _queueFamilyIndices.presentFamily)
		{
			std::vector<uint32_t> queueFamilies = {_queueFamilyIndices.graphicsFamily.value(), _queueFamilyIndices.presentFamily.value()};
			swapChainCreateInfo.setImageSharingMode(vk::SharingMode::eConcurrent);
			swapChainCreateInfo.setQueueFamilyIndices(queueFamilies);
		}
		else { swapChainCreateInfo.setImageSharingMode(vk::SharingMode::eExclusive); }

		swapChainCreateInfo.setPreTransform(_swapChainSupportDetails.capabilities.currentTransform);
		swapChainCreateInfo.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
		swapChainCreateInfo.setPresentMode(presentMode);
		swapChainCreateInfo.setClipped(vk::True);
		swapChainCreateInfo.setOldSwapchain(VK_NULL_HANDLE);

		_swapChain = _device.createSwapchainKHR(swapChainCreateInfo);

		_swapChainImages = _device.getSwapchainImagesKHR(_swapChain);
	}

	void VulkanContext::CreateInstance(SDL_Window* sdlWindow)
	{
		// Get vulkan instance extensions
		uint32_t extensionCount;
		SDL_Vulkan_GetInstanceExtensions(sdlWindow, &extensionCount, nullptr);

		std::vector<const char*> extensionNames = std::vector<const char*>(extensionCount, nullptr);
		SDL_Vulkan_GetInstanceExtensions(sdlWindow, &extensionCount, extensionNames.data());

		// Create infos
		vk::ApplicationInfo    applicationInfo    = vk::ApplicationInfo("Project Thalia",
                                                                  VK_MAKE_VERSION(1, 0, 0),
                                                                  "No Engine",
                                                                  VK_MAKE_VERSION(1, 0, 0),
                                                                  VK_API_VERSION_1_3);
		vk::InstanceCreateInfo instanceCreateInfo = vk::InstanceCreateInfo({}, &applicationInfo, _validationLayers, extensionNames);

		// Create vulkan instance
		vk::Result instanceCreationResult = vk::createInstance(&instanceCreateInfo, nullptr, &_instance);
		if (instanceCreationResult != vk::Result::eSuccess) { ErrorHandler::ThrowRuntimeError("Failed to create Vulkan instance!"); }
	}

	void VulkanContext::CreateSurface(SDL_Window* sdlWindow)
	{
		// Create surface from sdl
		VkSurfaceKHR surfaceHandle         = VK_NULL_HANDLE;
		SDL_bool     surfaceCreationResult = SDL_Vulkan_CreateSurface(sdlWindow, static_cast<VkInstance>(_instance), &surfaceHandle);
		_surface                           = surfaceHandle;
		if (surfaceCreationResult == SDL_FALSE) { ErrorHandler::ThrowRuntimeError("Failed to create SDL Vulkan surface!"); }
	}

	void VulkanContext::SelectPhysicalDevice()
	{
		std::vector<vk::PhysicalDevice> physicalDevices = _instance.enumeratePhysicalDevices();
		for (const vk::PhysicalDevice& physicalDevice : physicalDevices)
		{
			// Check device type
			vk::PhysicalDeviceProperties deviceProperties = physicalDevice.getProperties();
			if (deviceProperties.deviceType != vk::PhysicalDeviceType::eDiscreteGpu) { continue; }

			// Check Extensions
			std::vector<vk::ExtensionProperties, std::allocator<vk::ExtensionProperties>> availableExtensions = physicalDevice
																														.enumerateDeviceExtensionProperties();

			std::set<std::string> requiredExtensions = std::set<std::string>(_deviceExtensions.begin(), _deviceExtensions.end());

			for (const auto& extension : availableExtensions)
			{
				requiredExtensions.erase(extension.extensionName);
			}

			if (!requiredExtensions.empty()) { continue; }

			// Check queues
			_queueFamilyIndices        = QueueFamilyIndices();
			auto queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
			int  i                     = 0;
			for (const vk::QueueFamilyProperties& queueFamily : queueFamilyProperties)
			{
				unsigned int presentSupport = physicalDevice.getSurfaceSupportKHR(i, _surface);

				if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics && !_queueFamilyIndices.graphicsFamily.has_value())
				{
					Debug::Log::Info(std::format("graphics index: {}", i));
					_queueFamilyIndices.graphicsFamily = i;
				}

				if (presentSupport && !_queueFamilyIndices.presentFamily.has_value())
				{
					Debug::Log::Info(std::format("present index: {}", i));
					_queueFamilyIndices.presentFamily = i;
				}

				if (_queueFamilyIndices.isComplete()) { break; }

				i++;
			}

			if (!_queueFamilyIndices.isComplete()) { continue; }


			// Check swap chain support
			_swapChainSupportDetails = SwapChainSupportDetails();

			_swapChainSupportDetails.capabilities = physicalDevice.getSurfaceCapabilitiesKHR(_surface);
			_swapChainSupportDetails.formats      = physicalDevice.getSurfaceFormatsKHR(_surface);
			_swapChainSupportDetails.presentModes = physicalDevice.getSurfacePresentModesKHR(_surface);

			if (_swapChainSupportDetails.formats.empty() || _swapChainSupportDetails.presentModes.empty()) { continue; }

			// Select device
			_physicalDevice = physicalDevice;
			Debug::Log::Info(std::format("Selected physical device: {}", deviceProperties.deviceName));
			break;
		}

		// Check if we found a compatible GPU
		if (_physicalDevice == VK_NULL_HANDLE) { ErrorHandler::ThrowRuntimeError("This device does not have any gpus meeting the applications requirements"); }
	}

	void VulkanContext::CreateLogicalDevice()
	{
		// Get queue info
		std::set<uint32_t> uniqueQueueFamilies = {_queueFamilyIndices.graphicsFamily.value(),
												  _queueFamilyIndices.presentFamily.value()}; // Needs to be a set to filter out same queue features

		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos = std::vector<vk::DeviceQueueCreateInfo>(uniqueQueueFamilies.size());

		float queuePriority = 1.0f;

		int i = 0;
		for (const auto& uniqueQueueFamily : uniqueQueueFamilies)
		{
			queueCreateInfos[i] = vk::DeviceQueueCreateInfo({}, uniqueQueueFamily, 1, &queuePriority);
			i++;
		}

		// Create logical device
		vk::PhysicalDeviceFeatures deviceFeatures   = _physicalDevice.getFeatures();
		vk::DeviceCreateInfo       deviceCreateInfo = vk::DeviceCreateInfo({}, queueCreateInfos, _validationLayers, _deviceExtensions, &deviceFeatures);

		vk::Result vulkanDeviceCreateResult = _physicalDevice.createDevice(&deviceCreateInfo, nullptr, &_device);
		if (vulkanDeviceCreateResult != vk::Result::eSuccess) { ErrorHandler::ThrowRuntimeError("Failed to create logical device!"); }

		_graphicsQueue = _device.getQueue(_queueFamilyIndices.graphicsFamily.value(), 0);
		_presentQueue  = _device.getQueue(_queueFamilyIndices.presentFamily.value(), 0);
	}

	void VulkanContext::Destroy()
	{
		_device.waitIdle();

		_device.destroy(_imageAvailableSemaphore);
		_device.destroy(_renderFinishedSemaphore);
		_device.destroy(_inFlightFence);
		_device.destroy(_commandPool);
		_device.destroy(_pipeline);
		_device.destroy(_pipelineLayout);
		_device.destroy(_swapChain);

		for (const vk::ImageView& imageView : _swapChainImageViews)
		{
			_device.destroy(imageView);
		}

		for (const vk::Framebuffer& frameBuffer : _swapChainFrameBuffers)
		{
			_device.destroy(frameBuffer);
		}

		_device.destroy();
		_instance.destroy(_surface);
		_instance.destroy();
	}

	void VulkanContext::DrawFrame()
	{
		Debug::Log::Info("Draw frame");
		vk::Result waitForFencesResult = _device.waitForFences(1, &_inFlightFence, vk::True, UINT64_MAX);
		vk::Result resetFencesResult   = _device.resetFences(1, &_inFlightFence);

		vk::ResultValue<uint32_t> imageIndex = _device.acquireNextImageKHR(_swapChain, UINT64_MAX, _imageAvailableSemaphore, VK_NULL_HANDLE);

		_commandBuffer.reset({});
		RecordCommandBuffer(_commandBuffer, imageIndex.value);

		vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
		vk::SubmitInfo         submitInfo   = vk::SubmitInfo(_imageAvailableSemaphore, waitStages, _commandBuffer, _renderFinishedSemaphore);

		_graphicsQueue.submit(submitInfo, _inFlightFence);

		vk::PresentInfoKHR presentInfo = vk::PresentInfoKHR(_renderFinishedSemaphore, _swapChain, imageIndex.value, nullptr);

		vk::Result presentResult = _presentQueue.presentKHR(presentInfo);
	}

}