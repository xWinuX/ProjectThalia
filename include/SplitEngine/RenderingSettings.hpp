#pragma once

#include <string>
#include <vector>

namespace SplitEngine
{
	struct RenderingSettings
	{
		bool UseVulkanValidationLayers = false;
		uint32_t GPUDeviceID = -1u;
	};

	struct ShaderParserSettings
	{
		std::string VertexShaderFileExtension   = ".vert";
		std::string FragmentShaderFileExtension = ".frag";
		std::string ComputeShaderFileExtension  = ".comp";
		std::string SpirvFileExtension          = ".spv";
		char        ShaderBufferModDelimiter    = '_';

		/**
		 * This Buffer will not be double buffered
		 */
		std::vector<std::string> ShaderPropertySingleInstanceModPrefixes = { "singleInst", "si" };

		/**
		 * This Buffer is only visible to the device and data needs to be explicitly staged
		 */
		std::vector<std::string> ShaderBufferDeviceLocalModPrefixes = { "deviceLocal", "dl" };

		/**
		 * This buffer is on device memory but still visible from cpu. On most dedicated gpus this will allocate it
		 * on to a rather small heap on the gpu (214MB on a rtx 3070) so use this with caution.
		 * This buffer is rather slow to read from for obvious reasons.
		 */
		std::vector<std::string> ShaderBufferDeviceLocalHostVisibleModPrefixes = { "deviceLocalHostVisisble", "dlhv" };

		/**
		 * Will host cache the buffer
		 * This should be used if there will be a lot of read/writes on the cpu side
		 */
		std::vector<std::string> ShaderBufferCacheModPrefixes = { "chache", "c" };

		/**
		 * Shared will cause the underlying buffer of the attribute to only be created once
		 * This means buffer data can be shared across multiple shaders without it being in the global set
		 * Note: this only works if the name of the property is exactly the same.
		 */
		std::vector<std::string> ShaderPropertySharedModPrefixes = { "shared", "s" };


		/**
		 * NoAlloc will still create the property and descriptors like they should with all offsets etc...
		 * But no Buffers will actually be created for them and they need to be supplied manually via the OverrideBuffer/SetBuffer functions
		 */
		std::vector<std::string> ShaderBufferNoAllocModPrefixes = { "noAlloc", "na" };
	};
}
