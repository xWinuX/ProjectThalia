#pragma once

namespace ProjectThalia::Rendering::Vulkan
{
	template<typename T>
	concept ValidDeviceHandles = requires(T vkHandle) {
		requires std::same_as<T, vk::Pipeline> || std::same_as<T, vk::PipelineLayout> || std::same_as<T, vk::ShaderModule> || std::same_as<T, vk::RenderPass> ||
						 std::same_as<T, vk::SwapchainKHR> || std::same_as<T, vk::ImageView> || std::same_as<T, vk::Framebuffer> ||
						 std::same_as<T, vk::Buffer> || std::same_as<T, vk::DeviceMemory>;
	};

	class Utility
	{
		public:
			template<ValidDeviceHandles T>
			static void DeleteDeviceHandle(vk::Device device, T vkDeviceHandle);
	};

	template<ValidDeviceHandles T>
	void Utility::DeleteDeviceHandle(vk::Device device, T vkDeviceHandle)
	{
		if (vkDeviceHandle != VK_NULL_HANDLE) { device.destroy(vkDeviceHandle); }
	}
}
