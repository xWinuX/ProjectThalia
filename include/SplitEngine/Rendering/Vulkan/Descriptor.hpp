#pragma once

#include "Buffer.hpp"
#include "InFlightResource.hpp"

namespace SplitEngine::Rendering::Vulkan
{
	struct Descriptor
	{
		enum class Type : uint8_t
		{
			Buffer,
			ImageSampler,
		};

		uint32_t SharedID = -1;
		Type     Type;
		Buffer   Buffer {};
		bool     SingleInstance = false;

		InFlightResource<std::vector<vk::DescriptorImageInfo>> ImageInfos;
		InFlightResource<vk::DescriptorBufferInfo>             BufferInfos;
		InFlightResource<std::byte*>                           BufferPtrs;
		InFlightResource<vk::WriteDescriptorSet>               WriteDescriptorSets;
	};
}
