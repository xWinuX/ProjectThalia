#pragma once

#include <vector>

namespace SplitEngine
{

	template<typename T>
	class IncrementVector
	{
		public:
			IncrementVector() = default;

			explicit IncrementVector(size_t capacity) :
				_size(capacity),
				_vector(std::vector<T>(capacity))
			{}

			void PushBack(T element)
			{
				size_t newCursor = _cursor + 1;
				if (newCursor == _size) { _vector.push_back(element); }
				else { _vector[_cursor] = element; }
				_cursor = newCursor;
			}

			void Clear() { _cursor = 0; }

			T* begin() { return &_vector[0]; }

			const T* begin() const { return &_vector[0]; }

			T* end() { return &_vector[_cursor]; }

			const T* end() const { return &_vector[_cursor]; }

			std::vector<T> _vector;
			size_t         _size   = 0;
			size_t         _cursor = 0;
	};

	template<typename T>
	class AvailableStack
	{
		public:
			AvailableStack() = default;

			explicit AvailableStack(size_t capacity) :
				_vector(std::vector<T>(capacity, 0)),
				_cursor(capacity - 1)
			{}

			T Pop() { return _vector[_cursor--]; }

			void Push(T value) { _vector[++_cursor] = value; }

			[[nodiscard]] bool IsEmpty() const { return _cursor == 0; }

			T* begin() { return &_vector[0]; }

			const T* begin() const { return &_vector[0]; }

			T* end() { return &_vector[_size]; }

			const T* end() const { return &_vector[_size]; }

		private:
			std::vector<uint32_t> _vector;
			size_t                _size   = 0;
			uint32_t              _cursor = -1;
	};
}
