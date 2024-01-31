#pragma once

namespace SplitEngine::Rendering::Vulkan
{
	class Device;

	class DeviceObject
	{
		public:
			DeviceObject() = default;
			explicit DeviceObject(Device* device);

			virtual void Destroy() = 0;

		protected:
			[[nodiscard]] Device* GetDevice() const;

		private:
			Device* _device = nullptr;
	};
}
