#pragma once

namespace ProjectThalia::Rendering::Vulkan
{
	class Device;

	class DeviceObject
	{
		public:
			DeviceObject() = default;
			explicit DeviceObject(const Device* device);

			virtual void Destroy() = 0;

		protected:
			const Device* GetDevice();

		private:
			const Device* _device;
	};
}
