#pragma once

namespace ECSX {
	template<typename T>
	class TypeId
	{
	private:
		static size_t current;

	public:
		template<typename U>
		static const size_t id()
		{
			assert(current < MAX_COMPONENT);
			static const size_t count = current++;
			return count;
		}
	};

	template<class T> size_t TypeId<T>::current = 0;
}