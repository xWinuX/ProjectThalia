#pragma once

#include "ProjectThalia/Rendering/Vulkan/Pipeline.hpp"
#include "ProjectThalia/Rendering/Vulkan/Buffer.hpp"

namespace ProjectThalia::Rendering
{
	class Material
	{

		private:
			const Vulkan::Pipeline& _pipeline;

			std::vector<Vulkan::Buffer> _shaderBuffers;


	};

	template<typename T>
	class MaterialInstance : Material
	{

	};

}