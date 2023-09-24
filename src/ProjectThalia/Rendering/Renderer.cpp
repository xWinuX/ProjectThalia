#include "ProjectThalia/Rendering/Renderer.hpp"

namespace ProjectThalia::Rendering
{
	void Renderer::DrawFrame() { _vulkanContext.DrawFrame(); }

	void Renderer::Initialize(Window& window) { _vulkanContext.Initialize(window); }

	void Renderer::Destroy() { _vulkanContext.Destroy(); }
}
