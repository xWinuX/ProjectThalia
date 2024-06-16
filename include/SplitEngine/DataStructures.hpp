#pragma once

#include "SplitEngine/Debug/Log.hpp"
#include <glm/exponential.hpp>

#include <vector>

#define PRIVATE_COMPARE_SIZE(var) \
	if (_numBits != var._numBits) { LOG_FATAL("can't compare 2 signatures that have different sizes"); }

namespace SplitEngine
{
	// Source: https://indiegamedev.net/2020/05/19/an-entity-component-system-with-data-locality-in-cpp/
	template<typename TBase>
	class TypeIDGenerator
	{
		private:
			static uint64_t _count;

		public:
			template<typename T>
			static uint64_t GetID()
			{
				static const uint64_t ID = _count++;
				return ID;
			}

			static uint64_t GetCount() { return _count; }
	};

	template<class T>
	uint64_t TypeIDGenerator<T>::_count = 0;

	class DynamicBitSet
	{
		public:
			DynamicBitSet() = default;

			explicit DynamicBitSet(const uint64_t size) { ExtendSizeBy(size); }

			void ExtendSizeTo(const uint64_t newSize)
			{
				_numBits = newSize;
				while (_numBits > _masks.size() * MASK_SIZE) { _masks.push_back(0); }
			}

			void ExtendSizeBy(const uint64_t amountToIncrease = 1)
			{
				_numBits += amountToIncrease;
				while (_numBits > _masks.size() * MASK_SIZE) { _masks.push_back(0); }
			}

			void SetBit(const uint64_t bitIndex)
			{
				const uint64_t index         = bitIndex / MASK_SIZE;
				const uint64_t relativeIndex = bitIndex - (index * MASK_SIZE);

				_masks[index] |= static_cast<uint64_t>(1) << relativeIndex;
			}

			void UnsetBit(const uint64_t bitIndex)
			{
				const uint64_t index         = bitIndex / MASK_SIZE;
				const uint64_t relativeIndex = bitIndex - (index * MASK_SIZE);

				_masks[index] &= ~(static_cast<uint64_t>(1) << relativeIndex);
			}

			/**
			 * Checks if this bitset matches given bitset EXACTLY
			 */
			[[nodiscard]] bool Matches(const DynamicBitSet& other) const
			{
				PRIVATE_COMPARE_SIZE(other)

				for (uint64_t i = 0; i < _masks.size(); ++i) { if (_masks[i] != other._masks[i]) { return false; } }

				return true;
			}

			/**
			 * Determines whether this DynamicBitSet fuzzily matches another DynamicBitSet.
			 * Fuzzy matching requires that each set bit in this DynamicBitSet is also set in the given DynamicBitSet,
			 * without concern for additional bits set in the given bitset.
			 * Comment written by ChatGPT because I'm to dumb to explain this concisely
			 */
			[[nodiscard]] bool FuzzyMatches(const DynamicBitSet& other) const
			{
				PRIVATE_COMPARE_SIZE(other)

				for (uint64_t i = 0; i < _masks.size(); ++i) { if ((_masks[i] & other._masks[i]) != _masks[i]) { return false; } }

				return true;
			}

		private:
			static constexpr uint64_t MASK_SIZE = sizeof(uint64_t) * CHAR_BIT;

			uint64_t              _numBits = 0;
			std::vector<uint64_t> _masks;
	};

	template<typename T>
	class BitSet
	{
		public:
			BitSet() = default;

			explicit BitSet(T bits) { Set(bits); }

			inline void Set(const T bits) { _mask |= bits; }

			inline void Unset(const T bits) { _mask &= ~bits; }

			inline bool Has(const T bits) const { return (_mask & bits) == bits; }

			/**
			 * Checks if this bitset matches given bitset EXACTLY
			 */
			[[nodiscard]] inline bool Matches(const BitSet<T>& other) const { return _mask == other._mask; }

			/**
			 * Determines whether this DynamicBitSet fuzzily matches another DynamicBitSet.
			 * Fuzzy matching requires that each set bit in this DynamicBitSet is also set in the given DynamicBitSet,
			 * without concern for additional bits set in the given bitset.
			 * Comment written by ChatGPT because I'm to dumb to explain this concisely
			 */
			[[nodiscard]] inline bool FuzzyMatches(const BitSet<T>& other) const { return (_mask & other._mask) == _mask; }

			[[nodiscard]] T GetMask() const { return _mask; }

		private:
			T _mask = 0;
	};

	template<typename T>
	class IncrementVector
	{
		public:
			IncrementVector() = default;

			explicit IncrementVector(size_t capacity) :
				_vector(std::vector<T>(capacity)),
				_size(capacity) {}

			void PushBack(T element)
			{
				const size_t newCursor = _cursor + 1;
				if (newCursor == _size) { _vector.push_back(element); }
				else { _vector[_cursor] = element; }
				_cursor = newCursor;
			}

			void Clear() { _cursor = 0; }

			T* begin() { return _vector.data(); }

			const T* begin() const { return _vector.data(); }

			T* end() { return _vector.data() + _cursor; }

			const T* end() const { return _vector.data() + _cursor; }

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
				_vector(std::vector<T>(capacity, {})),
				_cursor(capacity) {}

			T Pop() { return _vector[--_cursor]; }

			void Push(T value)
			{
				if (_vector.size() <= _cursor)
				{
					_vector.push_back(value);
					++_cursor;
				}
				else { _vector[_cursor++] = value; }
			}

			[[nodiscard]] bool IsEmpty() const { return _cursor == 0; }

			[[nodiscard]] size_t GetSize() const { return _cursor; }

			T* begin() { return _vector.data(); }

			const T* begin() const { return _vector.data(); }

			T* end() { return _vector.data() + _cursor; }

			const T* end() const { return _vector.data() + _cursor; }

		private:
			std::vector<T> _vector{};
			uint64_t       _cursor = 0;
	};

	struct Color
	{
		public:
			enum class ColorType
			{
				SRGB,
				Linear,
			};

			Color() = default;

			explicit Color(const uint32_t hex, ColorType inputColorType = ColorType::SRGB):
				Color(static_cast<float>((hex >> 24) & 0xFFu) / 255.0f,
				      static_cast<float>((hex >> 16) & 0xFFu) / 255.0f,
				      static_cast<float>((hex >> 8) & 0xFFu) / 255.0f,
				      static_cast<float>(hex & 0xFFu) / 255.0f,
				      inputColorType) {}

			explicit Color(float r, float g, float b, float a, ColorType inputColorType = ColorType::SRGB):
				R(r),
				G(g),
				B(b),
				A(a)
			{
				if (inputColorType == ColorType::SRGB)
				{
					R = SRGBToLinear(R);
					G = SRGBToLinear(G);
					B = SRGBToLinear(B);
					A = SRGBToLinear(A);
				}
			}

			float R = 0.0f;
			float G = 0.0f;
			float B = 0.0f;
			float A = 0.0f;

			template<typename T>
			T ConvertToType() { return { R, G, B, A }; }

			Color operator*(const float scalar) const
			{
				Color color{};
				color.R = R * scalar;
				color.G = G * scalar;
				color.B = B * scalar;
				color.A = A * scalar;

				return color;
			}

			static float SRGBToLinear(float x) { return x <= 0.04045f ? x / 12.92f : glm::pow((x + 0.055f) / 1.055f, 2.4f); }
			static float LinearToSRGB(float x) { return x <= 0.0031308f ? 12.92f * x : 1.055f * glm::pow(x, 1.0f / 2.4f) - 0.055f; }
	};
}

#undef PRIVATE_COMPARE_SIZE
