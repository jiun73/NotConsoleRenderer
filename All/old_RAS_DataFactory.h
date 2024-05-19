#pragma once
#include "RAS_Typedef.h"

struct DataTypeFactory
{
	virtual void construct(RawData data) = 0;
	virtual void destruct(RawData data) = 0;
	virtual void move(RawData source, RawData destination) = 0;
	virtual size_t size() const = 0;
	virtual const type_info& type() = 0;
};

template<typename T>
class DataType : public DataTypeFactory
{
	const type_info& type() override
	{
		return typeid(T);
	}

	void construct(RawData data) override
	{
		new (&data[0]) T();
	}

	void destruct(RawData data) override
	{
		T* location = launder(reinterpret_cast<T*>(data));

		location->~T();
	}

	void move(RawData source, RawData destination) override
	{
		new (&destination[0]) T(std::move(*reinterpret_cast<T*>(source)));
	}

	size_t size() const override { return sizeof(T); }
};