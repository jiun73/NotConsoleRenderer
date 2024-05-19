#pragma once
#include "RAS_Typedef.h"

struct ModiferParameter
{
	template<typename... Ts>
	RawData set(const Ts&... data)
	{
		if constexpr (sizeof...(Ts) > 0)
		{
			vector<type_index> types;
			size_t size = 0;
			get_types<Ts...>(types, size);
			RawData raw = new char[size];
			size_t index = 0;
			set_types<Ts...>(raw, index, data...);
			assert((internal_check(types)));
			return raw;
		}
		else
			return nullptr;


	}

protected:
	//void get_types(vector<type_index>& list, size_t& size) {}

	template<typename T>
	void get_types_single(vector<type_index>& list, size_t& size)
	{
		list.push_back(typeid(T));
		size += sizeof(T);
	}

	template<typename T, typename... Rs>
	void get_types(vector<type_index>& list, size_t& size)
	{
		get_types_single<T>(list, size);
		if constexpr (sizeof...(Rs) == 0) return;
		else {
			get_types<Rs...>(list, size);
		}
	}

	//void set_types(RawData data, size_t& index) {}

	template<typename T>
	void set_types_single(RawData data, size_t& index, const T& a)
	{

		*(T*)(data + index) = a;
		index += sizeof(T);
	}

	template<typename T, typename... Rs>
	void set_types(RawData data, size_t& index, const T& a, const Rs&... as)
	{

		set_types_single<T>(data, index, a);
		if constexpr (sizeof...(Rs) == 0) return;
		else
		{
			set_types<Rs...>(data, index, as...);
		}
	}

	virtual bool internal_check(vector<type_index> types) = 0;
};

template<typename... Ts>
struct EventParameterType : public ModiferParameter
{
	bool internal_check(vector<type_index> types) override
	{
		vector<type_index> self_types;
		size_t size = 0;
		if constexpr (sizeof...(Ts) > 0)
			ModiferParameter::get_types<Ts...>(self_types, size);
		return self_types == types;
	}
};