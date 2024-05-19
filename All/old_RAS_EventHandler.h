#pragma once
#include "RAS_Typedef.h"
#include "RAS_EventParam.h"

struct ModifierHandler
{
	virtual ModiferParameter* param() = 0;
	virtual void apply(TimeMs time, const Modifier& event, RawData data, RawData args) = 0;
};

template<typename T, typename D, typename... Args>
class EventHandlerType : public ModifierHandler
{
	T system;
	EventParameterType<Args...> params;

	ModiferParameter* param() override
	{
		return &params;
	};

	template<size_t I, typename... Ts>
	void apply_unfold(TimeMs time, const Modifier& event, RawData data, RawData raw_args, size_t& index, const Ts&... args)
	{
		if constexpr (I < sizeof...(Args))
		{
			using type = std::tuple_element_t<I, tuple<Args...>>;
			size_t index_before = index;
			index += sizeof(type);
			apply_unfold<I + 1>(time, event, data, raw_args, index, args..., *(type*)(raw_args + index_before));

		}
		else
		{
			system.apply(time, event, *(D*)(data), args...);
		}
		//TODO pass real args;
	}


	void apply(TimeMs time, const Modifier& event, RawData data, RawData args) override
	{
		if constexpr (sizeof...(Args) == 0)
		{
			system.apply(time, event, *(D*)(data)); //TODO pass real args;
		}
		else
		{
			size_t index = 0;
			apply_unfold<0>(time, event, data, args, index);
		}

	}
};
