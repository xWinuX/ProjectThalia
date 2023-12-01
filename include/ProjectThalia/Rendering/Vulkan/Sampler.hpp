#pragma once

#include "DeviceObject.hpp"
#include "ProjectThalia/Rendering/TextureSettings.hpp"
#include "glm/vec3.hpp"

#include <vulkan/vulkan.hpp>

namespace ProjectThalia::Rendering::Vulkan
{
	class Device;

	class Sampler final : DeviceObject
	{
		public:
			Sampler() = default;
			Sampler(Device* device, const TextureSettings& textureSettings);

			void Destroy() override;

			[[nodiscard]] const vk::Sampler& GetVkSampler() const;

		private:
			vk::Sampler _vkSampler;
			TextureSettings _textureSettings;
	};

}