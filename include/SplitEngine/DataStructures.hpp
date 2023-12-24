#pragma once

#include <vector>

#define PRIVATE_COMPARE_SIZE(var) \
	if (_numBits != var._numBits) { LOG_FATAL("can't compare 2 signatures that have different sizes"); }

namespace SplitEngine
{
	class DynamicBitSet
	{
		public:
			DynamicBitSet() = default;

			DynamicBitSet(uint64_t size) { ExtendSizeBy(size); }

			void ExtendSizeBy(uint64_t amountToIncrease = 1)
			{
				_numBits += amountToIncrease;
				while (_numBits > _masks.size() * MASK_SIZE) { _masks.push_back(0); }
			}

			void SetBit(uint64_t bitIndex)
			{
				uint64_t index         = bitIndex / MASK_SIZE;
				uint64_t relativeIndex = bitIndex - (index * MASK_SIZE);

				_masks[index] |= static_cast<uint64_t>(1) << relativeIndex;
			}

			void UnsetBit(uint64_t bitIndex)
			{
				uint64_t index         = bitIndex / MASK_SIZE;
				uint64_t relativeIndex = bitIndex - (index * MASK_SIZE);

				_masks[index] &= ~(static_cast<uint64_t>(1) << relativeIndex);
			}

			/**
			 * Checks if this bitset matches given bitset EXACTLY
			 */
			bool Matches(const DynamicBitSet& other)
			{
				PRIVATE_COMPARE_SIZE(other)

				for (uint64_t i = 0; i < _masks.size(); ++i)
				{
					if (_masks[i] != other._masks[i]) { return false; }
				}

				return true;
			}

			/**
			 * Determines whether this DynamicBitSet fuzzily matches another DynamicBitSet.
			 * Fuzzy matching requires that each set bit in this DynamicBitSet is also set in the given DynamicBitSet,
			 * without concern for additional bits set in the given bitset.
			 * Comment written by ChatGPT because I'm to dumb to explain this concisely
			 */
			bool FuzzyMatches(const DynamicBitSet& other)
			{
				PRIVATE_COMPARE_SIZE(other)

				for (uint64_t i = 0; i < _masks.size(); ++i)
				{
					if ((_masks[i] & other._masks[i]) != _masks[i]) { return false; }
				}

				return true;
			}

		private:
			static constexpr uint64_t MASK_SIZE = sizeof(uint64_t) * CHAR_BIT;

			uint64_t              _numBits = 0;
			std::vector<uint64_t> _masks;
	};

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

#undef PRIVATE_COMPARE_SIZE