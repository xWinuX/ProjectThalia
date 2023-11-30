#pragma once

#include "ProjectThalia/Rendering/Vulkan/Pipeline.hpp"
#include "ProjectThalia/Rendering/Vulkan/Buffer.hpp"

namespace ProjectThalia::Rendering
{
	template<class T>
	class Material
	{

		private:
			const Vulkan::Pipeline& _pipeline;

			std::vector<Vulkan::Buffer> _shaderBuffers;


	};


}