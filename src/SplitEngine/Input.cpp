#include "SplitEngine/Input.hpp"
#include "glm/gtx/string_cast.hpp"

namespace SplitEngine
{
	std::unordered_map<int, bool>                Input::_keyDownStates{};
	std::unordered_map<int, Input::PressedState> Input::_keyPressedStates{};

	std::unordered_map<int, Input::ButtonAction> Input::_buttonActions{};
	std::unordered_map<int, Input::AxisAction>   Input::_axisActions{};

	std::unordered_map<int, KeyCode> Input::_mouseToKeyCode{ { SDL_BUTTON_LEFT, KeyCode::MOUSE_LEFT }, { SDL_BUTTON_RIGHT, KeyCode::MOUSE_RIGHT }, };

	glm::ivec2 Input::_mousePosition{};
	glm::ivec2 Input::_mousePositionWorldOffset{};

	bool Input::GetDown(const KeyCode keyCode) { return _keyDownStates[keyCode]; }

	bool Input::GetPressed(const KeyCode keyCode) { return _keyPressedStates[keyCode] == PressedState::Pressed; }

	void Input::Update(const SDL_Event& event)
	{
		switch (event.type)
		{
			case SDL_MOUSEBUTTONDOWN:
			{
				const int buttonCode       = _mouseToKeyCode[event.button.button];
				_keyDownStates[buttonCode] = true;
				if (_keyPressedStates[buttonCode] == Ready) { _keyPressedStates[buttonCode] = Pressed; }
				break;
			}

			case SDL_MOUSEBUTTONUP:
			{
				const int buttonCode       = _mouseToKeyCode[event.button.button];
				_keyDownStates[buttonCode] = false;
				if (_keyPressedStates[buttonCode] == Waiting) { _keyPressedStates[buttonCode] = Ready; }
				break;
			}

			case SDL_KEYDOWN:
				_keyDownStates[event.key.keysym.sym] = true;
				if (_keyPressedStates[event.key.keysym.sym] == Ready) { _keyPressedStates[event.key.keysym.sym] = Pressed; }
				break;

			case SDL_KEYUP:
				_keyDownStates[event.key.keysym.sym] = false;
				if (_keyPressedStates[event.key.keysym.sym] == Waiting) { _keyPressedStates[event.key.keysym.sym] = Ready; }
				break;

			case SDL_MOUSEMOTION:
				SDL_GetMouseState(&_mousePosition.x, &_mousePosition.y);
				_mousePosition += _mousePositionWorldOffset;
				break;
		}
	}

	void Input::Reset()
	{
		for (const std::pair<const int, PressedState>& pair: _keyPressedStates)
		{
			if (pair.second == PressedState::Pressed) { _keyPressedStates[pair.first] = PressedState::Waiting; }
		}

		for (auto& [key, buttonAction]: _buttonActions) { buttonAction.Cached = false; }

		for (auto& [key, axisAction]: _axisActions)
		{
			axisAction.PressedCached = false;
			axisAction.DownCached    = false;
		}
	}

	glm::ivec2 Input::GetMousePosition() { return _mousePosition; }

	void Input::ProvideWorldMouseOffset(glm::ivec2 offset) { _mousePositionWorldOffset = offset; }
}
