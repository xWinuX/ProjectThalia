#include "ProjectThalia/Rendering/Vulkan/DeviceObject.hpp"

namespace ProjectThalia::Rendering::Vulkan
{
	DeviceObject::DeviceObject(Device* device) :
		_device(device)
	{}

	Device* DeviceObject::GetDevice() const { return _device; }

}
