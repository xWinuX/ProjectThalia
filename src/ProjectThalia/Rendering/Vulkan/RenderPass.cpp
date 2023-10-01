#include "ProjectThalia/Rendering/Vulkan/RenderPass.hpp"
#include "ProjectThalia/Rendering/Vulkan/Utility.hpp"

namespace ProjectThalia::Rendering::Vulkan
{
	RenderPass::RenderPass(const Device* device) :
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

		vk::AttachmentReference colorAttachmentReference = vk::AttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal);
		vk::SubpassDescription  subpass                  = vk::SubpassDescription({}, vk::PipelineBindPoint::eGraphics, {}, {}, 1, &colorAttachmentReference);


		vk::SubpassDependency subpassDependency = vk::SubpassDependency(vk::SubpassExternal,
																		{},
																		vk::PipelineStageFlagBits::eColorAttachmentOutput,
																		vk::PipelineStageFlagBits::eColorAttachmentOutput,
																		{},
																		vk::AccessFlagBits::eColorAttachmentWrite);

		vk::RenderPassCreateInfo renderPassCreateInfo = vk::RenderPassCreateInfo({}, 1, &colorAttachment, 1, &subpass, 1, &subpassDependency);

		_vkRenderPass = device->GetVkDevice().createRenderPass(renderPassCreateInfo);
	}

	const vk::RenderPass& RenderPass::GetVkRenderPass() const { return _vkRenderPass; }

	void RenderPass::Destroy() { Utility::DeleteDeviceHandle(GetDevice(), _vkRenderPass); }

}
