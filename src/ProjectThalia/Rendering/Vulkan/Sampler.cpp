#include "ProjectThalia/Rendering/Vulkan/Sampler.hpp"
#include "ProjectThalia/Rendering/Vulkan/Utility.hpp"

namespace ProjectThalia::Rendering::Vulkan
{
	Sampler::Sampler(Device* device, const TextureSettings& textureSettings) :
		DeviceObject(device),
		_textureSettings(textureSettings)
	{
		vk::SamplerCreateInfo samplerCreateInfo = vk::SamplerCreateInfo({},
																		static_cast<vk::Filter>(textureSettings.MagnificationFilter),
																		static_cast<vk::Filter>(textureSettings.MinificationFilter),
																		static_cast<vk::SamplerMipmapMode>(textureSettings.MipmapMode),
																		static_cast<vk::SamplerAddressMode>(textureSettings.WrapMode.x),
																		static_cast<vk::SamplerAddressMode>(textureSettings.WrapMode.y),
																		static_cast<vk::SamplerAddressMode>(textureSettings.WrapMode.z),
																		textureSettings.MipLodBias,
																		textureSettings.MaxAnisotropy > 0.0f,
																		textureSettings.MaxAnisotropy,
																		vk::False,
																		vk::CompareOp::eNever,
																		textureSettings.MinLod,
																		textureSettings.MaxLod,
																		vk::BorderColor::eIntOpaqueBlack,
																		vk::False);

		_vkSampler = GetDevice()->GetVkDevice().createSampler(samplerCreateInfo);
	}

	void Sampler::Destroy() { Utility::DeleteDeviceHandle(GetDevice(), _vkSampler); }

	const vk::Sampler& Sampler::GetVkSampler() const { return _vkSampler; }
}
