#pragma once
#include <array>
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>


#define STATIC_VARS(name, num, offsets, formats)               \
	const size_t     name::_offsets[] = offsets;               \
	const vk::Format name::_formats[] = formats;               \
	const std::array<vk::VertexInputAttributeDescription, num> \
			name::VertexInputAttributeDescriptions = ProjectThalia::Rendering::GetVertexInputAttributeDescriptions<num>(_offsets, _formats);

#define DEFINE_VERTEX_FORMAT_BEGIN(name, num)                                                                   \
	struct name                                                                                                 \
	{                                                                                                           \
		public:                                                                                                 \
			static const std::array<vk::VertexInputAttributeDescription, num> VertexInputAttributeDescriptions; \
                                                                                                                \
		private:                                                                                                \
			static const vk::Format _formats[];                                                                 \
			static const size_t     _offsets[];                                                                 \
                                                                                                                \
		public:

#define DEFINE_VERTEX_FORMAT_END() \
	}                              \
	;


#define DEFINE_VERTEX_FORMAT_1(name, type0, name0, format0) \
	DEFINE_VERTEX_FORMAT_BEGIN(name, 1)                     \
	type0 name0;                                            \
	DEFINE_VERTEX_FORMAT_END()                              \
	STATIC_VARS(name, 1, {(offsetof(name, name0))}, {(format0)})


#define DEFINE_VERTEX_FORMAT_2(name, type0, name0, format0, type1, name1, format1) \
	DEFINE_VERTEX_FORMAT_BEGIN(name, 2)                                            \
	type0 name0;                                                                   \
	type1 name1;                                                                   \
	DEFINE_VERTEX_FORMAT_END()                                                     \
	STATIC_VARS(name, 2, {(offsetof(name, name0), offsetof(name, name1))}, {(format0, format1)})

namespace ProjectThalia::Rendering
{
	template<int TSize>
	std::array<vk::VertexInputAttributeDescription, TSize> GetVertexInputAttributeDescriptions(const size_t* offsets, const vk::Format* formats)
	{
		std::array<vk::VertexInputAttributeDescription, TSize> vertexInputAttributeDescriptions {};

		for (int i = 0; i < vertexInputAttributeDescriptions.size(); ++i)
		{
			vertexInputAttributeDescriptions[i].binding  = 0;
			vertexInputAttributeDescriptions[i].location = i;
			vertexInputAttributeDescriptions[i].format   = formats[i];
			vertexInputAttributeDescriptions[i].offset   = offsets[i];
		}

		return vertexInputAttributeDescriptions;
	}

	DEFINE_VERTEX_FORMAT_1(VertexPosition, glm::vec3, Position, vk::Format::eR32G32Sfloat)
	DEFINE_VERTEX_FORMAT_2(VertexPositionColor, glm::vec3, Position, vk::Format::eR32G32Sfloat, glm::vec3, Color, vk::Format::eR32G32Sfloat)
}