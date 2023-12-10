#include "SplitEngine/Input.hpp"

namespace SplitEngine
{
	std::unordered_map<int, bool>                Input::_keyDownStates    = std::unordered_map<int, bool>();
	std::unordered_map<int, Input::PressedState> Input::_keyPressedStates = std::unordered_map<int, Input::PressedState>();

	void Input::Update(const SDL_Event& event)
	{
		if (event.type == SDL_KEYDOWN)
		{
			_keyDownStates[event.key.keysym.sym] = true;

			if (_keyPressedStates[event.key.keysym.sym] == Ready) { _keyPressedStates[event.key.keysym.sym] = Pressed; }
		}
		if (event.type == SDL_KEYUP)
		{
			if (_keyPressedStates[event.key.keysym.sym] == Waiting) { _keyPressedStates[event.key.keysym.sym] = Ready; }
		}
	}

	bool Input::GetDown(SDL_KeyCode keyCode)
	{
		if (_keyDownStates.find(keyCode) == _keyDownStates.end()) { _keyDownStates[keyCode] = false; }

		return _keyDownStates[keyCode];
	}

	bool Input::GetPressed(SDL_KeyCode keyCode)
	{
		if (_keyPressedStates.find(keyCode) == _keyPressedStates.end()) { _keyPressedStates[keyCode] = Ready; }

		return _keyPressedStates[keyCode] == PressedState::Pressed;
	}

	void Input::Reset()
	{
		for (const auto& pair : _keyPressedStates)
		{
			if (pair.second == PressedState::Pressed) { _keyPressedStates[pair.first] = PressedState::Waiting; }
		}
	}
}