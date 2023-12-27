#pragma once

#include "SplitEngine/DataStructures.hpp"

#include "Archetype.hpp"
#include "Component.hpp"
#include "Context.hpp"
#include "Entity.hpp"
#include "SystemBase.hpp"

#include <iterator>
#include <vector>

#ifndef SE_HEADLESS
namespace SplitEngine::Rendering::Vulkan
{
	class Context;
}
#endif

namespace SplitEngine::ECS
{
	class Registry
	{
		public:
			Registry();

			~Registry();

			void Update(float deltaTime);

			void Render(float deltaTime);

			template<typename... T>
			size_t CreateEntity(T&&... args)
			{
				Archetype<T...>& archetype = GetArchetype<T...>();

				uint64_t entityID = 0;
				if (!_entityGraveyard.IsEmpty())
				{
					entityID                                     = _entityGraveyard.Pop();
					uint64_t componentIndex                      = archetype.AddEntity(entityID, std::forward<T>(args)...);
					_sparseEntityLookup[entityID].archetypeIndex = archetype.ID;
					_sparseEntityLookup[entityID].componentIndex = componentIndex;
				}
				else
				{
					entityID                = _sparseEntityLookup.size();
					uint64_t componentIndex = archetype.AddEntity(entityID, std::forward<T>(args)...);
					_sparseEntityLookup.emplace_back(archetype.ID, componentIndex);
				}

				return entityID;
			}

			void DestroyEntity(uint64_t entityID);

			template<typename T>
			void RegisterComponent()
			{
				TypeIDGenerator<Component>::GetID<T>();
			}

			template<typename T, typename... TArgs>
			void RegisterGameplaySystem(TArgs&&... args)
			{
				static_assert(std::is_base_of<SystemBase, T>::value, "an ECS System needs to derive from SplitEngine::ECS::System");

				_gameplaySystems.emplace_back(new T(std::forward<TArgs>(args)...));
			}

			[[nodiscard]] std::vector<ArchetypeBase*> GetArchetypesWithSignature(const DynamicBitSet& signature);

			template<typename... T>
			Archetype<T...>& GetArchetype()
			{
				static Archetype<T...> archetype = Archetype<T...>(_sparseEntityLookup);
				return archetype;
			}

		private:
			std::vector<Entity> _sparseEntityLookup;

			AvailableStack<uint64_t> _entityGraveyard;

			std::vector<SystemBase*> _gameplaySystems;

			Context _context {};

#ifndef SE_HEADLESS
		public:
			void RegisterRenderingContext(SplitEngine::Rendering::Vulkan::Context* context) { _context.RenderingContext = context; }

			template<typename T, typename... TArgs>
			void RegisterRenderSystem(TArgs&&... args)
			{
				static_assert(std::is_base_of<SystemBase, T>::value, "an ECS System needs to derive from SplitEngine::ECS::System");

				_renderSystems.emplace_back(new T(std::forward<TArgs>(args)...));
			}

		private:
			std::vector<SystemBase*> _renderSystems;
#endif
	};


}
