#include "SplitEngine/Input.hpp"

namespace SplitEngine
{
	std::unordered_map<int, bool>                Input::_keyDownStates {};
	std::unordered_map<int, Input::PressedState> Input::_keyPressedStates {};

	std::unordered_map<int, Input::ButtonAction> Input::_buttonActions {};
	std::unordered_map<int, Input::AxisAction>   Input::_axisActions {};

	bool Input::GetDown(SDL_KeyCode keyCode) { return _keyDownStates[keyCode]; }

	bool Input::GetPressed(SDL_KeyCode keyCode) { return _keyPressedStates[keyCode] == PressedState::Pressed; }

	void Input::Update(const SDL_Event& event)
	{
		if (event.type == SDL_KEYDOWN)
		{
			_keyDownStates[event.key.keysym.sym] = true;

			if (_keyPressedStates[event.key.keysym.sym] == Ready) { _keyPressedStates[event.key.keysym.sym] = Pressed; }
		}
		if (event.type == SDL_KEYUP)
		{
			_keyDownStates[event.key.keysym.sym] = false;

			if (_keyPressedStates[event.key.keysym.sym] == Waiting) { _keyPressedStates[event.key.keysym.sym] = Ready; }
		}
	}

	void Input::Reset()
	{
		for (const auto& pair : _keyPressedStates)
		{
			if (pair.second == PressedState::Pressed) { _keyPressedStates[pair.first] = PressedState::Waiting; }
		}

		for (auto& [key, buttonAction] : _buttonActions) { buttonAction.Cached = false; }

		for (auto& [key, axisAction] : _axisActions)
		{
			axisAction.PressedCached = false;
			axisAction.DownCached    = false;
		}
	}
}