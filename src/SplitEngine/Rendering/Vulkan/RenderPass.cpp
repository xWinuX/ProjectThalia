#include "SplitEngine/Rendering/Vulkan/RenderPass.hpp"
#include "SplitEngine/Rendering/Vulkan/Utility.hpp"

namespace SplitEngine::Rendering::Vulkan
{
	RenderPass::RenderPass(Device* device) :
		DeviceObject(device)
	{
		vk::AttachmentDescription colorAttachment = vk::AttachmentDescription({},
																			  device->GetPhysicalDevice().GetImageFormat().format,
																			  vk::SampleCountFlagBits::e1,
																			  vk::AttachmentLoadOp::eClear,
																			  vk::AttachmentStoreOp::eStore,
																			  vk::AttachmentLoadOp::eDontCare,
																			  vk::AttachmentStoreOp::eDontCare,
																			  vk::ImageLayout::eUndefined,
																			  vk::ImageLayout::ePresentSrcKHR);


		vk::AttachmentDescription depthAttachment = vk::AttachmentDescription({},
																			  device->GetPhysicalDevice().GetDepthImageFormat(),
																			  vk::SampleCountFlagBits::e1,
																			  vk::AttachmentLoadOp::eClear,
																			  vk::AttachmentStoreOp::eDontCare,
																			  vk::AttachmentLoadOp::eDontCare,
																			  vk::AttachmentStoreOp::eDontCare,
																			  vk::ImageLayout::eUndefined,
																			  vk::ImageLayout::eDepthStencilAttachmentOptimal);

		vk::AttachmentReference colorAttachmentReference = vk::AttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal);
		vk::AttachmentReference depthAttachmentReference = vk::AttachmentReference(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);

		vk::SubpassDescription subpass = vk::SubpassDescription({},
																vk::PipelineBindPoint::eGraphics,
																{},
																colorAttachmentReference,
																{},
																&depthAttachmentReference);


		vk::SubpassDependency subpassDependency = vk::SubpassDependency(vk::SubpassExternal,
																		{},
																		vk::PipelineStageFlagBits::eColorAttachmentOutput |
																				vk::PipelineStageFlagBits::eEarlyFragmentTests,
																		vk::PipelineStageFlagBits::eColorAttachmentOutput |
																				vk::PipelineStageFlagBits::eEarlyFragmentTests,
																		{},
																		vk::AccessFlagBits::eColorAttachmentWrite |
																				vk::AccessFlagBits::eDepthStencilAttachmentWrite);

		std::vector<vk::AttachmentDescription> attachments          = {colorAttachment, depthAttachment};
		vk::RenderPassCreateInfo               renderPassCreateInfo = vk::RenderPassCreateInfo({}, attachments, subpass, subpassDependency);

		_vkRenderPass = device->GetVkDevice().createRenderPass(renderPassCreateInfo);
	}

	const vk::RenderPass& RenderPass::GetVkRenderPass() const { return _vkRenderPass; }

	void RenderPass::Destroy() { Utility::DeleteDeviceHandle(GetDevice(), _vkRenderPass); }

}
