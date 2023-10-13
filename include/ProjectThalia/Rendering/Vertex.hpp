#pragma once

#include <array>
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

#define PROTECT_ARRAY(...) __VA_ARGS__

#define STATIC_VARS(name, num, offsets, formats)                                                                                             \
	inline const size_t     name::_offsets[] = offsets;                                                                                      \
	inline const vk::Format name::_formats[] = formats;                                                                                      \
	inline const std::array<vk::VertexInputAttributeDescription, num>                                                                        \
			name::VertexInputAttributeDescriptions = ProjectThalia::Rendering::GetVertexInputAttributeDescriptions<num>(_offsets, _formats); \
	inline const vk::VertexInputBindingDescription name::VertexInputBindingDescription = GetBindingDescription<name>();

#define DEFINE_VERTEX_FORMAT_BEGIN(name, num)                                                                   \
	struct name                                                                                                 \
	{                                                                                                           \
		public:                                                                                                 \
			static const std::array<vk::VertexInputAttributeDescription, num> VertexInputAttributeDescriptions; \
			static const vk::VertexInputBindingDescription                    VertexInputBindingDescription;    \
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
	STATIC_VARS(name, 1, PROTECT_ARRAY({offsetof(name, name0)}), PROTECT_ARRAY({format0}))


#define DEFINE_VERTEX_FORMAT_2(name, type0, name0, format0, type1, name1, format1) \
	DEFINE_VERTEX_FORMAT_BEGIN(name, 2)                                            \
	type0 name0;                                                                   \
	type1 name1;                                                                   \
	DEFINE_VERTEX_FORMAT_END()                                                     \
	STATIC_VARS(name, 2, PROTECT_ARRAY({offsetof(name, name0), offsetof(name, name1)}), PROTECT_ARRAY({format0, format1}))

#define DEFINE_VERTEX_FORMAT_3(name, type0, name0, format0, type1, name1, format1, type2, name2, format2) \
	DEFINE_VERTEX_FORMAT_BEGIN(name, 3)                                                                   \
	type0 name0;                                                                                          \
	type1 name1;                                                                                          \
	type2 name2;                                                                                          \
	DEFINE_VERTEX_FORMAT_END()                                                                            \
	STATIC_VARS(name, 3, PROTECT_ARRAY({offsetof(name, name0), offsetof(name, name1), offsetof(name, name2)}), PROTECT_ARRAY({format0, format1, format2}))

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

	template<typename T>
	vk::VertexInputBindingDescription GetBindingDescription()
	{
		vk::VertexInputBindingDescription vertexInputBindingDescription = vk::VertexInputBindingDescription(0, sizeof(T), vk::VertexInputRate::eVertex);
		return vertexInputBindingDescription;
	}

	DEFINE_VERTEX_FORMAT_3(VertexPosition2DColorUV,
						   glm::vec2,
						   Position,
						   vk::Format::eR32G32Sfloat,
						   glm::vec3,
						   Color,
						   vk::Format::eR32G32B32Sfloat,
						   glm::vec2,
						   UV,
						   vk::Format::eR32G32Sfloat)
}