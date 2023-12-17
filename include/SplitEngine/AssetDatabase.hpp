#pragma once

#include "ErrorHandler.hpp"
#include <unordered_map>
#include <utility>

#define MAP(type, var) \
	std::unordered_map<size_t, SplitEngine::type> _##var##Map {}

#define SPECIALIZE(type, var)                                                                                                                 \
	template<>                                                                                                                                \
	inline AssetHandle<SplitEngine::type> AssetDatabase::CreateAsset<SplitEngine::type>(size_t key, SplitEngine::type::CreateInfo createInfo) \
	{                                                                                                                                         \
		_##var##Map.try_emplace(key, createInfo);                                                                                             \
		return AssetHandle<type>(*this, key);                                                                                                 \
	}                                                                                                                                         \
                                                                                                                                              \
	template<>                                                                                                                                \
	inline SplitEngine::type* AssetDatabase::GetAsset<SplitEngine::type>(AssetHandle<SplitEngine::type> assetHandle)                          \
	{                                                                                                                                         \
		return &_##var##Map.at(assetHandle._id);                                                                                              \
	}

namespace SplitEngine
{
	class AssetDatabase;

	template<class T>
	struct AssetHandle
	{
			friend AssetDatabase;

		public:
			T* operator->()
			{
				if (_asset == nullptr) { _asset = _assetDatabase.GetAsset<T>(*this); }
				return _asset;
			}

		private:
			explicit AssetHandle(AssetDatabase& assetDatabase, size_t id) :
				_assetDatabase(assetDatabase),
				_id(id)
			{}

			T*             _asset = nullptr;
			AssetDatabase& _assetDatabase;
			size_t         _id;
	};

	class AssetDatabase
	{
		public:
			template<class T>
			[[nodiscard]] AssetHandle<T> CreateAsset(size_t key, T::CreateInfo createInfo)
			{
				ErrorHandler::ThrowRuntimeError(std::format("Creation of asset with type {0} is not supported", typeid(T).name()));
			}

			template<class T>
			T* GetAsset(AssetHandle<T> assetHandle)
			{
				ErrorHandler::ThrowRuntimeError(std::format("Retrieval of asset with type {0} is not supported", typeid(T).name()));
			}

		private:
			MAP(Rendering::Texture2D, texture);
	};

	SPECIALIZE(Rendering::Texture2D, texture);
}
