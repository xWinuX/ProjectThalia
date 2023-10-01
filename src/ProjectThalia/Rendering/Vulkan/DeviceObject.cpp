#include "ProjectThalia/Rendering/Vulkan/DeviceObject.hpp"

namespace ProjectThalia::Rendering::Vulkan
{
	DeviceObject::DeviceObject(const Device* device) :
		_device(device)
	{}

	const Device* DeviceObject::GetDevice() { return _device; }

}
