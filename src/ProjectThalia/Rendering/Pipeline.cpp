#include "ProjectThalia/Rendering/Pipeline.hpp"
#include "ProjectThalia/Application.hpp"


namespace ProjectThalia::Rendering
{
	Pipeline::Pipeline(const std::string& name, const std::vector<ShaderInfo>& shaderInfos)
	{
		/*
		std::vector<vk::PipelineShaderStageCreateInfo> _shaderStages = std::vector<vk::PipelineShaderStageCreateInfo>(2);


		for (const ShaderInfo& shaderInfo : shaderInfos)
		{
			IO::Stream        stream     = IO::Stream(shaderInfo.path);
			std::vector<char> shaderCode = stream.Read();
			stream.Close();

			vk::ShaderModuleCreateInfo createInfo = vk::ShaderModuleCreateInfo({}, shaderCode.size(), reinterpret_cast<const uint32_t*>(shaderCode.data()));

			vk::ShaderModule shaderModule = VulkanContext::GetDevice().createShaderModule(createInfo);
			_shaderModules.push_back(shaderModule);

			vk::PipelineShaderStageCreateInfo shaderStageCreateInfo = vk::PipelineShaderStageCreateInfo({},
																										shaderInfo.shaderStage,
																										_shaderModules[_shaderModules.size()],
																										name.c_str());
			_shaderStages.push_back(shaderStageCreateInfo);
		}*/
	}
}