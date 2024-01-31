#include "SplitEngine/Rendering/Vulkan/DeviceObject.hpp"

namespace SplitEngine::Rendering::Vulkan
{
	DeviceObject::DeviceObject(Device* device) :
		_device(device)
	{}

	Device* DeviceObject::GetDevice() const { return _device; }

}
