#include "ProjectThalia/Input.hpp"

namespace ProjectThalia
{
	std::unordered_map<int, bool> Input::_keyStates = std::unordered_map<int, bool>();

	void Input::Update(const SDL_Event& event)
	{
		if (event.type == SDL_KEYDOWN) { _keyStates[event.key.keysym.sym] = true; }
		if (event.type == SDL_KEYUP) { _keyStates[event.key.keysym.sym] = false; }
	}

	bool Input::GetDown(SDL_KeyCode keyCode)
	{
		if (_keyStates.find(keyCode) == _keyStates.end()) { _keyStates[keyCode] = false; }

		return _keyStates[keyCode];
	}
}