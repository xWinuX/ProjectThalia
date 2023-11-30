#include "ProjectThalia/Rendering/Vulkan/Sampler.hpp"
#include "ProjectThalia/Rendering/Vulkan/Utility.hpp"

namespace ProjectThalia::Rendering::Vulkan
{
	Sampler::Sampler(Device* device, const Sampler::CreateInfo& createInfo) :
		DeviceObject(device)
	{
		vk::SamplerCreateInfo samplerCreateInfo = vk::SamplerCreateInfo({},
																		static_cast<vk::Filter>(createInfo.MagnificationFilter),
																		static_cast<vk::Filter>(createInfo.MinificationFilter),
																		static_cast<vk::SamplerMipmapMode>(createInfo.MipmapMode),
																		static_cast<vk::SamplerAddressMode>(createInfo.WrapMode.x),
																		static_cast<vk::SamplerAddressMode>(createInfo.WrapMode.y),
																		static_cast<vk::SamplerAddressMode>(createInfo.WrapMode.z),
																		createInfo.MipLodBias,
																		createInfo.MaxAnisotropy > 0.0f,
																		createInfo.MaxAnisotropy,
																		vk::False,
																		vk::CompareOp::eNever,
																		createInfo.MinLod,
																		createInfo.MaxLod,
																		vk::BorderColor::eIntOpaqueBlack,
																		vk::False);

		_vkSampler = GetDevice()->GetVkDevice().createSampler(samplerCreateInfo);
	}

	void Sampler::Destroy() { Utility::DeleteDeviceHandle(GetDevice(), _vkSampler); }

	const vk::Sampler& Sampler::GetVkSampler() const { return _vkSampler; }
}
