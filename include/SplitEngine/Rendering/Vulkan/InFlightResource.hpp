#pragma once

#include <vector>

namespace SplitEngine::Rendering::Vulkan
{
	template<typename T>
	class InFlightResource
	{
		public:
			InFlightResource() = default;

			InFlightResource(uint32_t* currentFrame, std::vector<T>&& data) :
				_currentFrame(currentFrame),
				_data(std::move(data))
			{}

			[[nodiscard]] bool IsValid() const { return _currentFrame != nullptr; }

			T& Get() { return _data[*_currentFrame]; }

			const T& Get() const { return _data[*_currentFrame]; }

			T& operator->() { return _data[*_currentFrame]; }

			T& operator[](size_t index) { return _data[index]; }

		private:
			uint32_t*      _currentFrame = nullptr;
			std::vector<T> _data {};
	};

}
