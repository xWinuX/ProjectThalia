#include "SplitEngine/Input.hpp"
#include "glm/gtx/string_cast.hpp"

namespace SplitEngine
{
	std::unordered_map<int, bool>                Input::_keyDownStates{};
	std::unordered_map<int, Input::PressedState> Input::_keyPressedStates{};

	std::unordered_map<int, Input::ButtonAction> Input::_buttonActions{};
	std::unordered_map<int, Input::AxisAction>   Input::_axisActions{};

	std::unordered_map<int, KeyCode> Input::_mouseToKeyCode{
		{ SDL_BUTTON_LEFT, KeyCode::MOUSE_LEFT },
		{ SDL_BUTTON_RIGHT, KeyCode::MOUSE_RIGHT },
		{ SDL_BUTTON_MIDDLE, KeyCode::MOUSE_MIDDLE },
		{ SDL_BUTTON_X1, KeyCode::MOUSE_X1 },
		{ SDL_BUTTON_X2, KeyCode::MOUSE_X2 },
	};

	glm::ivec2 Input::_mousePosition{};
	glm::ivec2 Input::_mouseDelta{};
	glm::vec2  Input::_mouseWheel{};

	bool Input::GetDown(const KeyCode keyCode) { return _keyDownStates[static_cast<int>(keyCode)]; }

	bool Input::GetPressed(const KeyCode keyCode) { return _keyPressedStates[static_cast<int>(keyCode)] == PressedState::Pressed; }

	void Input::Update(const SDL_Event& event)
	{
		switch (event.type)
		{
			case SDL_MOUSEBUTTONDOWN:
			{
				const int buttonCode       = static_cast<int>(_mouseToKeyCode[event.button.button]);
				_keyDownStates[buttonCode] = true;
				if (_keyPressedStates[buttonCode] == PressedState::Ready) { _keyPressedStates[buttonCode] = PressedState::Pressed; }
				break;
			}

			case SDL_MOUSEBUTTONUP:
			{
				const int buttonCode       = static_cast<int>(_mouseToKeyCode[event.button.button]);
				_keyDownStates[buttonCode] = false;
				if (_keyPressedStates[buttonCode] == PressedState::Waiting) { _keyPressedStates[buttonCode] = PressedState::Ready; }
				break;
			}

			case SDL_KEYDOWN:
				_keyDownStates[event.key.keysym.sym] = true;
				if (_keyPressedStates[event.key.keysym.sym] == PressedState::Ready) { _keyPressedStates[event.key.keysym.sym] = PressedState::Pressed; }
				break;

			case SDL_KEYUP:
				_keyDownStates[event.key.keysym.sym] = false;
				if (_keyPressedStates[event.key.keysym.sym] == PressedState::Waiting) { _keyPressedStates[event.key.keysym.sym] = PressedState::Ready; }
				break;

			case SDL_MOUSEMOTION:
				SDL_GetMouseState(&_mousePosition.x, &_mousePosition.y);
				SDL_GetRelativeMouseState(&_mouseDelta.x, &_mouseDelta.y);
				break;

			case SDL_MOUSEWHEEL:
				_mouseWheel = { event.wheel.x, event.wheel.y };
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

		_mouseWheel = glm::zero<glm::vec2>();
		_mouseDelta = glm::zero<glm::ivec2>();
	}

	glm::ivec2 Input::GetMousePosition() { return _mousePosition; }
	glm::ivec2 Input::GetMouseDelta() { return _mouseDelta; }
	glm::ivec2 Input::GetMouseWheel() { return _mouseWheel; }
}
