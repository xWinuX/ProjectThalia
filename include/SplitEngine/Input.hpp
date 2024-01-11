#pragma once

#include "glm/common.hpp"
#include "glm/fwd.hpp"
#include "glm/vec4.hpp"
#include <SDL_events.h>
#include <algorithm>
#include <array>
#include <unordered_map>

namespace SplitEngine
{
	class Application;

	class Input
	{
			friend Application;

		public:
			struct PressedAxis
			{
					bool Positive;
					bool Negative;
			};

			struct Axis
			{
				public:
					SDL_KeyCode NegativeKey = SDLK_UNKNOWN;
					SDL_KeyCode PositiveKey = SDLK_UNKNOWN;
			};

			struct ButtonAction
			{
				public:
					std::vector<SDL_KeyCode> KeyCodes {};
					bool                     Value  = false;
					bool                     Cached = false;
			};

			struct AxisAction
			{
				public:
					struct AxisVector
					{
						public:
							std::array<Axis, 4> Axes {};

							Axis& operator[](size_t index) { return Axes[index]; }

							const Axis& operator[](size_t index) const { return Axes[index]; }
					};

					std::vector<AxisVector> Axes {};

					bool       PressedCached = false;
					glm::ivec4 PressedValue {};

					bool      DownCached = false;
					glm::vec4 DownValue {};
			};

			template<typename T>
			static float GetAxisActionDown(T actionKey)
			{
				return GetAxisActionDownDynamic<1>(actionKey).x;
			}

			template<typename T>
			static glm::vec2 GetAxis2DActionDown(T actionKey)
			{
				return GetAxisActionDownDynamic<2>(actionKey);
			}

			template<typename T>
			static PressedAxis GetAxisActionPressed(T actionKey)
			{
				return GetAxisActionPressedDynamic<1>(actionKey).x;
			}

			template<typename T>
			static glm::vec<2, PressedAxis> GetAxis2DActionPressed(T actionKey)
			{
				return GetAxisActionPressedDynamic<2>(actionKey);
			}

			template<typename T>
			static bool GetButtonActionDown(T actionKey)
			{
				ButtonAction& buttonAction = _buttonActions[static_cast<int>(actionKey)];

				if (!buttonAction.Cached)
				{
					bool value = false;
					for (const auto& keyCode : buttonAction.KeyCodes)
					{
						if (_keyDownStates[keyCode])
						{
							value = true;
							break;
						}
					}
					buttonAction.Cached = true;
					buttonAction.Value  = value;
				}

				return buttonAction.Value;
			}

			template<typename T>
			static bool GetButtonActionPressed(T actionKey)
			{
				ButtonAction& buttonAction = _buttonActions[static_cast<int>(actionKey)];

				if (!buttonAction.Cached)
				{
					bool value = false;
					for (const auto& keyCode : buttonAction.KeyCodes)
					{
						if (_keyPressedStates[keyCode] == PressedState::Pressed)
						{
							value = true;
							break;
						}
					}
					buttonAction.Cached = true;
					buttonAction.Value  = value;
				}

				return buttonAction.Value;
			}

			template<typename T>
			static void RegisterButtonAction(T actionKey, SDL_KeyCode key)
			{
				_buttonActions[static_cast<int>(actionKey)].KeyCodes.push_back(key);
			}

			template<typename T>
			static void RegisterAxis(T actionKey, Axis axis)
			{
				_axisActions[static_cast<int>(actionKey)].Axes.push_back({axis});
			}

			template<typename T>
			static void RegisterAxis2D(T actionKey, Axis xAxis, Axis yAxis)
			{
				_axisActions[static_cast<int>(actionKey)].Axes.push_back({xAxis, yAxis});
			}

			static bool GetDown(SDL_KeyCode keyCode);

			static bool GetPressed(SDL_KeyCode keyCode);

		private:
			enum PressedState
			{
				Ready,
				Pressed,
				Waiting,
			};

			static std::unordered_map<int, ButtonAction> _buttonActions;
			static std::unordered_map<int, AxisAction>   _axisActions;

			static std::unordered_map<int, bool>         _keyDownStates;
			static std::unordered_map<int, PressedState> _keyPressedStates;

			static void Update(const SDL_Event& event);

			static void Reset();

			template<int TNum, typename T>
			static glm::vec<TNum, float> GetAxisActionDownDynamic(T actionKey)
			{
				AxisAction& axisAction = _axisActions[static_cast<int>(actionKey)];

				if (!axisAction.DownCached)
				{
					glm::vec<TNum, float> value {};
					for (const auto& axisVector : axisAction.Axes)
					{
						for (int i = 0; i < TNum; ++i)
						{
							value[i] += static_cast<float>(_keyDownStates[axisVector[i].PositiveKey]) -
										static_cast<float>(_keyDownStates[axisVector[i].NegativeKey]);
						}
					}

					for (int i = 0; i < TNum; ++i)
					{
						value[i]                = std::clamp(value[i], -1.0f, 1.0f);
						axisAction.DownValue[i] = value[i];
					}

					axisAction.DownCached = true;
				}

				return axisAction.DownValue;
			}

			template<int TNum, typename T>
			static glm::vec<TNum, PressedAxis> GetAxisActionPressedDynamic(T actionKey)
			{
				AxisAction& axisAction = _axisActions[static_cast<int>(actionKey)];

				if (!axisAction.PressedCached)
				{
					glm::vec<TNum, int> value {};
					for (const auto& axisVector : axisAction.Axes)
					{
						for (int i = 0; i < TNum; ++i)
						{
							value[i] += static_cast<int>(_keyPressedStates[axisVector[i].PositiveKey] == PressedState::Pressed) -
										static_cast<int>(_keyPressedStates[axisVector[i].NegativeKey] == PressedState::Pressed);
						}
					}

					for (int i = 0; i < TNum; ++i)
					{
						value[i]                   = std::clamp(value[i], -1, 1);
						axisAction.PressedValue[i] = value[i];
					}

					axisAction.PressedCached = true;
				}

				glm::vec<TNum, PressedAxis> ret {};
				for (int i = 0; i < TNum; ++i) { ret[i] = {axisAction.PressedValue[i] > 0, axisAction.PressedValue[i] < 0}; }

				return ret;
			}
	};

}