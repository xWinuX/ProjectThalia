#pragma once

#include <vulkan/vulkan.hpp>

namespace ProjectThalia::Rendering
{

	class Instance
	{
		public:
			Instance() = default;
			Instance(std::vector<const char*> extensionNames, std::vector<const char*> validationLayers, vk::ApplicationInfo applicationInfo);

			void Destroy();

			[[nodiscard]] const vk::Instance&   GetVkInstance() const;
			[[nodiscard]] const vk::SurfaceKHR& GetVkSurface() const;

			void SetVkSurface(const vk::SurfaceKHR& vkSurface);

		private:
			vk::Instance   _vkInstance;
			vk::SurfaceKHR _vkSurface;
	};

}
