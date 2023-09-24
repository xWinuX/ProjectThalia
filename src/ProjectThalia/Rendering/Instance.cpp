#include "ProjectThalia/Rendering/Instance.hpp"

namespace ProjectThalia::Rendering
{
	Instance::Instance(std::vector<const char*> extensionNames, std::vector<const char*> validationLayers, vk::ApplicationInfo applicationInfo)
	{
		vk::InstanceCreateInfo instanceCreateInfo = vk::InstanceCreateInfo({}, &applicationInfo, validationLayers, extensionNames);

		// Create vulkan instance
		_vkInstance = vk::createInstance(instanceCreateInfo);
	}

	const vk::Instance& Instance::GetVkInstance() const { return _vkInstance; }

	const vk::SurfaceKHR& Instance::GetVkSurface() const { return _vkSurface; }

	void Instance::SetVkSurface(const vk::SurfaceKHR& vkSurface) { _vkSurface = vkSurface; }
}
