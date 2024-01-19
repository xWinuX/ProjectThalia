#include "SplitEngine/Rendering/Renderer.hpp"
#include "SplitEngine/Debug/Performance.hpp"
#include "SplitEngine/ErrorHandler.hpp"
#include <imgui_impl_sdl2.h>
#include <imgui_impl_vulkan.h>

#include <chrono>

namespace SplitEngine::Rendering
{
	Renderer::~Renderer()
	{
		LOG("Shutting down Renderer...");
		_vulkanContext.Destroy();
		_window.Close();
	}

	void Renderer::Initialize()
	{
		if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
		{
			ErrorHandler::ThrowRuntimeError(std::format("SDL could not initialize! SDL_Error: {0}\n", SDL_GetError()));
		}

		_window.Open();

		_vulkanContext.Initialize(&_window);

		_window.OnResize.Add([this](int width, int height) {
			_frameBufferResized = true;
		});

		StartImGuiFrame();
	}

	void Renderer::StartImGuiFrame() const
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplSDL2_NewFrame(_window.GetSDLWindow());

		ImGui::NewFrame();
	}

	void Renderer::HandleEvents(SDL_Event event) const
	{
		ImGui_ImplSDL2_ProcessEvent(&event);

		switch (event.window.event)
		{
			case SDL_WINDOWEVENT_SIZE_CHANGED:
				_window.OnResize.Invoke(event.window.data1, event.window.data2);
				break;
				//case SDL_WINDOWEVENT_MINIMIZED: _window.SetMinimized(true); break;
				//case SDL_WINDOWEVENT_RESTORED: _window.SetMinimized(false); break;
		}
	}

	void Renderer::BeginRender()
	{
		if (_window.IsMinimized())
		{
			_wasSkipped = true;
			return;
		}

		ImGui::Render();

		Vulkan::Device* device = Vulkan::Context::GetDevice();

		device->GetVkDevice().waitForFences(_vulkanContext.GetInFlightFence(), vk::True, UINT64_MAX);
		device->GetVkDevice().resetFences(_vulkanContext.GetInFlightFence());

		vk::ResultValue<uint32_t> imageIndexResult = device->GetVkDevice().acquireNextImageKHR(device->GetSwapchain().GetVkSwapchain(),
																							   UINT64_MAX,
																							   _vulkanContext.GetImageAvailableSemaphore(),
																							   VK_NULL_HANDLE);

		_latestImageIndexResult = imageIndexResult.value;

		if (imageIndexResult.result == vk::Result::eErrorOutOfDateKHR)
		{
			device->GetVkDevice().waitIdle();
			device->CreateSwapchain(_vulkanContext.GetInstance().GetVkSurface(), _window.GetSize());

			_wasSkipped = true;

			return;
		}
		else if (imageIndexResult.result != vk::Result::eSuccess && imageIndexResult.result != vk::Result::eSuboptimalKHR)
		{
			ErrorHandler::ThrowRuntimeError("failed to acquire swap chain image!");
		}


		const vk::CommandBuffer& commandBuffer = _vulkanContext.GetCommandBuffer();

		commandBuffer.reset({});

		vk::CommandBufferBeginInfo commandBufferBeginInfo = vk::CommandBufferBeginInfo({}, nullptr);

		commandBuffer.begin(commandBufferBeginInfo);

		vk::ClearValue clearColor      = vk::ClearValue({0.0f, 0.2f, 0.5f, 1.0f});
		vk::ClearValue depthClearColor = vk::ClearValue({1.0f, 0});

		std::vector<vk::ClearValue> clearValues = {clearColor, depthClearColor};

		vk::RenderPassBeginInfo renderPassBeginInfo = vk::RenderPassBeginInfo(device->GetRenderPass().GetVkRenderPass(),
																			  device->GetSwapchain().GetFrameBuffers()[imageIndexResult.value],
																			  {{0, 0}, device->GetSwapchain().GetExtend()},
																			  clearValues);

		vk::Viewport viewport = vk::Viewport(0,
											 static_cast<float>(device->GetSwapchain().GetExtend().height),
											 static_cast<float>(device->GetSwapchain().GetExtend().width),
											 -static_cast<float>(device->GetSwapchain().GetExtend().height),
											 0.0f,
											 1.0f);

		commandBuffer.setViewport(0, 1, &viewport);

		vk::Rect2D scissor = vk::Rect2D({0, 0}, device->GetSwapchain().GetExtend());
		commandBuffer.setScissor(0, 1, &scissor);

		commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
	}

	void Renderer::EndRender()
	{
		Vulkan::Device* device = Vulkan::Context::GetDevice();
		const vk::CommandBuffer& commandBuffer = _vulkanContext.GetCommandBuffer();

		if (_wasSkipped)
		{
			_wasSkipped = false;
			return;
		}

		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

		commandBuffer.endRenderPass();
		commandBuffer.end();

		vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
		vk::SubmitInfo         submitInfo   = vk::SubmitInfo(_vulkanContext.GetImageAvailableSemaphore(),
                                                   waitStages,
                                                   _vulkanContext.GetCommandBuffer(),
                                                   _vulkanContext.GetRenderFinishedSemaphore());



		device->GetGraphicsQueue().submit(submitInfo, _vulkanContext.GetInFlightFence());

		vk::PresentInfoKHR presentInfo = vk::PresentInfoKHR(_vulkanContext.GetRenderFinishedSemaphore(),
															device->GetSwapchain().GetVkSwapchain(),
															_latestImageIndexResult,
															nullptr);

		vk::Result presentResult = device->GetPresentQueue().presentKHR(presentInfo);


		if (presentResult == vk::Result::eErrorOutOfDateKHR || presentResult == vk::Result::eSuboptimalKHR || _frameBufferResized)
		{
			device->GetVkDevice().waitIdle();
			_frameBufferResized = false;
			device->CreateSwapchain(_vulkanContext.GetInstance().GetVkSurface(), _window.GetSize());
		}
		else if (presentResult != vk::Result::eSuccess) { ErrorHandler::ThrowRuntimeError("failed to present swap chain image!"); }

		device->AdvanceFrame();

		StartImGuiFrame();
	}

	Vulkan::Context& Renderer::GetContext() { return _vulkanContext; }
}
