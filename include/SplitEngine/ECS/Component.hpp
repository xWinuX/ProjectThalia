#pragma once
#include <cstddef>

namespace SplitEngine::ECS
{
	struct Component
	{
		typedef void(*DestructorFunc)(std::byte*);

		size_t Size;
		DestructorFunc Destructor;
	};
}
