#pragma once

#include <functional>
#include <vector>

namespace ProjectThalia
{
	template<typename... TArgs>
	class Event
	{
		public:
			void Invoke(TArgs... args) const
			{
				for (const std::function<void(TArgs...)>& function : _functions) { function(args...); }
			}

			void Add(std::function<void(TArgs...)> function) { _functions.push_back(function); }

			void Remove(std::function<void(TArgs...)> function) { std::remove(_functions.begin(), _functions.end(), function); }

		private:
			std::vector<std::function<void(TArgs...)>> _functions;
	};
}
