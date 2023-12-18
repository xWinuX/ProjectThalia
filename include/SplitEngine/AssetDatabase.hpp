#pragma once

#include "ErrorHandler.hpp"

#include <format>
#include <functional>
#include <unordered_map>

namespace SplitEngine
{
	class AssetDatabase;

	template<class T>
	struct AssetHandle
	{
			friend AssetDatabase;

		public:
			AssetHandle() = default;

			T* operator->()
			{
				if (_asset == nullptr) { _asset = _assetDatabase->GetAsset<T>(*this); }
				return _asset;
			}

		private:
			explicit AssetHandle(AssetDatabase* assetDatabase, size_t id) :
				_assetDatabase(assetDatabase),
				_id(id)
			{}

			T*             _asset         = nullptr;
			AssetDatabase* _assetDatabase = nullptr;
			size_t         _id            = -1;
	};

	class AssetDatabase
	{
		public:
			~AssetDatabase()
			{
				for (auto& item : _assetDeletionList) { item(); }
			}

			template<class T>
			[[nodiscard]] AssetHandle<T> CreateAsset(size_t key, T::CreateInfo createInfo)
			{
				T* pointer = new T(createInfo);
				_assetMap.try_emplace(key, pointer);

				_assetDeletionList.push_back([pointer] {
					delete pointer;
				});

				return AssetHandle<T>(this, key);
			}

			template<class T>
			T* GetAsset(AssetHandle<T> assetHandle)
			{
				return (T*) _assetMap.at(assetHandle._id);
			}

		private:
			std::unordered_map<size_t, void*>  _assetMap;
			std::vector<std::function<void()>> _assetDeletionList;
	};

}
