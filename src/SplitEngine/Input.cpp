#include "SplitEngine/Input.hpp"
#include "SplitEngine/Debug/Log.hpp"
#include "glm/gtx/string_cast.hpp"

namespace SplitEngine
{
	std::unordered_map<int, bool>                Input::_keyDownStates {};
	std::unordered_map<int, Input::PressedState> Input::_keyPressedStates {};

	std::unordered_map<int, Input::ButtonAction> Input::_buttonActions {};
	std::unordered_map<int, Input::AxisAction>   Input::_axisActions {};

	glm::ivec2 Input::_mousePosition {};
	glm::ivec2 Input::_mousePositionWorldOffset {};

	bool Input::GetDown(SDL_KeyCode keyCode) { return _keyDownStates[keyCode]; }

	bool Input::GetPressed(SDL_KeyCode keyCode) { return _keyPressedStates[keyCode] == PressedState::Pressed; }

	void Input::Update(const SDL_Event& event)
	{
		switch (event.type)
		{
			case SDL_KEYDOWN:
				_keyDownStates[event.key.keysym.sym] = true;
				if (_keyPressedStates[event.key.keysym.sym] == Ready) { _keyPressedStates[event.key.keysym.sym] = Pressed; }
				break;
			case SDL_KEYUP:
				_keyDownStates[event.key.keysym.sym] = false;

				if (_keyPressedStates[event.key.keysym.sym] == Waiting) { _keyPressedStates[event.key.keysym.sym] = Ready; }
				break;
			case SDL_MOUSEBUTTONDOWN:
				break;
			case SDL_MOUSEBUTTONUP:
				break;
			case SDL_MOUSEMOTION:
				SDL_GetMouseState(&_mousePosition.x, &_mousePosition.y);
				_mousePosition += _mousePositionWorldOffset;
				LOG(glm::to_string(_mousePosition));
				LOG(glm::to_string(_mousePositionWorldOffset));
				break;
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

	const glm::ivec2 Input::GetMousePosition() { return _mousePosition; }

	void Input::ProvideWorldMouseOffset(glm::ivec2 offset) { _mousePositionWorldOffset = offset; }
}