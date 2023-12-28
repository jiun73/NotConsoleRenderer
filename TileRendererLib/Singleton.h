#pragma once

//T'inquiète

template<typename T>
class Singleton
{
private:
	static T* manager;

public:
	static T* get()
	{
		if (manager == nullptr)
			manager = new T();

		return manager;
	}
};

template <class T> T* Singleton<T> ::manager;
