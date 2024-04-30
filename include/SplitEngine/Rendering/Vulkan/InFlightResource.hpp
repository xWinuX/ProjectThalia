#pragma once

#include <vector>
#include "SplitEngine/Debug/Log.hpp"

namespace SplitEngine::Rendering::Vulkan
{
	template<typename T>
	class InFlightResource
	{
		public:
			InFlightResource() = default;

			InFlightResource(uint32_t* currentFrame, std::vector<T>&& data) :
				_currentFrame(currentFrame),
				_data(std::move(data)) {}

			[[nodiscard]] bool IsValid() const { return _currentFrame != nullptr; }

			T& Get() { return _data[*_currentFrame]; }

			void Set(T&& value) { _data[*_currentFrame] = value; }

			void SetFramePtr(uint32_t* ptr) { _currentFrame = ptr; }

			const T& Get() const { return _data[*_currentFrame]; }

			T& operator->() { return _data[*_currentFrame]; }

			T& operator[](size_t index) { return _data[index]; }

			T& operator[](size_t index) const { return _data[index]; }

			std::vector<T>& GetDataVector() { return _data; }

			T* GetDataPtr() { return _data.data(); }

		private:
			uint32_t*      _currentFrame = nullptr;
			std::vector<T> _data{};
	};
}
