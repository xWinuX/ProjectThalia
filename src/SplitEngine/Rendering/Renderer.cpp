#include "SplitEngine/Rendering/Renderer.hpp"
#include "SplitEngine/ErrorHandler.hpp"
#include <imgui_impl_sdl2.h>
#include <imgui_impl_vulkan.h>

#include <chrono>

namespace SplitEngine::Rendering
{
	void Renderer::Render()
	{
		if (_window.IsMinimized()) { return; }

		ImGui::Render();

		Vulkan::Device* _device = Vulkan::Context::GetDevice();

		_device->GetVkDevice().waitForFences(_vulkanContext.GetInFlightFence(), vk::True, UINT64_MAX);

		vk::ResultValue<uint32_t> imageIndexResult = _device->GetVkDevice().acquireNextImageKHR(_device->GetSwapchain().GetVkSwapchain(),
																								UINT64_MAX,
																								_vulkanContext.GetImageAvailableSemaphore(),
																								VK_NULL_HANDLE);

		if (imageIndexResult.result == vk::Result::eErrorOutOfDateKHR)
		{
			_device->GetVkDevice().waitIdle();
			_device->CreateSwapchain(_vulkanContext.GetInstance().GetVkSurface(), _window.GetSize());

			return;
		}
		else if (imageIndexResult.result != vk::Result::eSuccess && imageIndexResult.result != vk::Result::eSuboptimalKHR)
		{
			ErrorHandler::ThrowRuntimeError("failed to acquire swap chain image!");
		}

		_device->GetVkDevice().resetFences(_vulkanContext.GetInFlightFence());

		// TODO: Set global uniform buffer

		const vk::CommandBuffer& commandBuffer = _vulkanContext.GetCommandBuffer();

		commandBuffer.reset({});

		vk::CommandBufferBeginInfo commandBufferBeginInfo = vk::CommandBufferBeginInfo({}, nullptr);

		commandBuffer.begin(commandBufferBeginInfo);

		vk::ClearValue clearColor      = vk::ClearValue({0.0f, 0.2f, 0.5f, 1.0f});
		vk::ClearValue depthClearColor = vk::ClearValue({1.0f, 0});

		std::vector<vk::ClearValue> clearValues = {clearColor, depthClearColor};

		vk::RenderPassBeginInfo renderPassBeginInfo = vk::RenderPassBeginInfo(_device->GetRenderPass().GetVkRenderPass(),
																			  _device->GetSwapchain().GetFrameBuffers()[imageIndexResult.value],
																			  {{0, 0}, _device->GetSwapchain().GetExtend()},
																			  clearValues);

		commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

		for (auto& [material, models] : _modelsToRender)
		{
			commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, material->GetShader()->GetPipeline().GetVkPipeline());

			for (const Model* model : models)
			{
				vk::Buffer     vertexBuffers[] = {model->GetModelBuffer().GetVkBuffer()};
				vk::DeviceSize offsets[]       = {0};

				commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
				commandBuffer.bindIndexBuffer(model->GetModelBuffer().GetVkBuffer(), model->GetModelBuffer().GetSizeInBytes(0), vk::IndexType::eUint16);

				vk::Viewport viewport = vk::Viewport(0,
													 static_cast<float>(_device->GetSwapchain().GetExtend().height),
													 static_cast<float>(_device->GetSwapchain().GetExtend().width),
													 -static_cast<float>(_device->GetSwapchain().GetExtend().height),
													 0.0f,
													 1.0f);
				commandBuffer.setViewport(0, 1, &viewport);

				vk::Rect2D scissor = vk::Rect2D({0, 0}, _device->GetSwapchain().GetExtend());
				commandBuffer.setScissor(0, 1, &scissor);

				material->Update();

				commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
												 material->GetShader()->GetPipeline().GetLayout(),
												 0,
												 1,
												 &material->GetDescriptorSetAllocation().DescriptorSet,
												 0,
												 nullptr);

				commandBuffer.drawIndexed(model->GetModelBuffer().GetBufferElementNum(1), 1'024'000 / 5000, 0, 0, 0);
			}
		}

		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

		commandBuffer.endRenderPass();
		commandBuffer.end();

		vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
		vk::SubmitInfo         submitInfo   = vk::SubmitInfo(_vulkanContext.GetImageAvailableSemaphore(),
                                                   waitStages,
                                                   _vulkanContext.GetCommandBuffer(),
                                                   _vulkanContext.GetRenderFinishedSemaphore());

		_device->GetGraphicsQueue().submit(submitInfo, _vulkanContext.GetInFlightFence());

		vk::PresentInfoKHR presentInfo = vk::PresentInfoKHR(_vulkanContext.GetRenderFinishedSemaphore(),
															_device->GetSwapchain().GetVkSwapchain(),
															imageIndexResult.value,
															nullptr);

		vk::Result presentResult = _device->GetPresentQueue().presentKHR(presentInfo);

		if (presentResult == vk::Result::eErrorOutOfDateKHR || presentResult == vk::Result::eSuboptimalKHR || _frameBufferResized)
		{
			_device->GetVkDevice().waitIdle();
			_frameBufferResized = false;
			_device->CreateSwapchain(_vulkanContext.GetInstance().GetVkSurface(), _window.GetSize());
		}
		else if (presentResult != vk::Result::eSuccess) { ErrorHandler::ThrowRuntimeError("failed to present swap chain image!"); }

		// Clear out current models
		for (auto& [material, models] : _modelsToRender) { models.Clear(); }

		StartImGuiFrame();
	}

	Renderer::~Renderer()
	{
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

	void Renderer::SubmitModel(Material* material, const Model* model)
	{
		if (!_modelsToRender.contains(material)) { _modelsToRender[material] = IncrementVector<const Model*>(100); }

		_modelsToRender[material].PushBack(model);
	}

	void Renderer::HandleEvents(SDL_Event event)
	{
		ImGui_ImplSDL2_ProcessEvent(&event);

		switch (event.window.event)
		{
			case SDL_WINDOWEVENT_SIZE_CHANGED: _window.OnResize.Invoke(event.window.data1, event.window.data2); break;
			//case SDL_WINDOWEVENT_MINIMIZED: _window.SetMinimized(true); break;
			//case SDL_WINDOWEVENT_RESTORED: _window.SetMinimized(false); break;
		}
	}
}