#pragma once

#include "Buffer.hpp"

#include <vector>

namespace ProjectThalia::Rendering::Vulkan
{

	class DynamicBuffer
	{
		public:
			const Buffer& GetBuffer(uint32_t index);

		private:
			std::vector<Buffer> _buffers;
	};
}
