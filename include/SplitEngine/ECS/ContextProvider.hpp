#pragma once

#include <iostream>

#include "SplitEngine/DataStructures.hpp"
#include <vector>

namespace SplitEngine
{
	struct EngineContext;
}

namespace SplitEngine::ECS
{
	class Registry;

	struct ContextProvider
	{
		public:
			Registry* Registry;

			~ContextProvider() { for (int i = 0; i < _offsets.size(); ++i) { _destructorFuncs[i](_contexts.data() + _offsets[i]); } }

			template<typename T>
			T* GetContext() { return reinterpret_cast<T*>(_contexts.data() + _offsets[TypeIDGenerator<ContextProvider>::GetID<T>()]); }

			template<typename T>
			void RegisterContext(T&& context)
			{
				const uint64_t id = TypeIDGenerator<ContextProvider>::GetID<T>();

				const size_t oldSize = _contexts.size();

				_contexts.resize(oldSize + sizeof(context));

				T* contextPtr = reinterpret_cast<T*>(_contexts.data() + oldSize);
				*contextPtr   = std::forward<T>(context);

				_destructorFuncs.resize(id + 1);
				_destructorFuncs[id] = [](std::byte* rawContext)
				{
					T* c = reinterpret_cast<T*>(rawContext);
					c->~T();
				};

				_offsets.resize(id + 1);
				_offsets[id] = oldSize;
			}

		private:
			typedef void (*DestructorFunc)(std::byte*);

			std::vector<std::byte>      _contexts{};
			std::vector<size_t>         _offsets;
			std::vector<DestructorFunc> _destructorFuncs{};
	};
}
