#pragma once
#include <new>
#include <type_traits>
#include <vector>
#include <tuple>

namespace ECSX {
	using std::launder;
	using std::vector;
	using std::tuple;
	using std::tuple_element_t;
	typedef unsigned char* ComponentData;

	const size_t MAX_COMPONENT = 32;
	typedef uint32_t ComponentBytes;
	typedef size_t ComponentID;

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

	struct ComponentX
	{
		virtual void construct(ComponentData data) = 0;
		virtual void destruct(ComponentData data) = 0;
		virtual void move(ComponentData source, ComponentData destination) = 0;
		virtual size_t size() const = 0;
	};

	inline ComponentBytes get_bytes_from_list(const vector<ComponentID>& list)
	{
		ComponentBytes map = 0;
		for (auto& id : list)
		{
			map |= (1 << id);
		}
		return map;
	}

	inline vector<ComponentID> get_list_from_bytes(ComponentBytes bytes)
	{
		vector<ComponentID> ret;
		for (size_t i = 0; i < 32; i++)
		{
			if (bytes & (1 << i))
			{
				ret.push_back(i);
			}
		}
		return ret;
	}

	template<class C>
	struct ComponentXFactory : public ComponentX
	{
		void construct(ComponentData data) override
		{
			new (&data[0]) C();
		}

		void destruct(ComponentData data) override
		{
			C* location = launder(reinterpret_cast<C*>(data));

			location->~C();
		}

		void move(ComponentData source, ComponentData destination) override
		{
			new (&destination[0]) C(std::move(*reinterpret_cast<C*>(source)));
		}

		size_t size() const override { return sizeof(C); }
	};

	template<typename... Reqs>
	struct ComponentCollector
	{
		template<size_t I = 0>
		inline void collect_components(vector<ComponentID>& collector)
		{
			using tuple_type = tuple<Reqs...>;
			if constexpr (I < sizeof...(Reqs))
			{
				using element_t = tuple_element_t<I, tuple_type>;
				ComponentID id = TypeId<ComponentX>::id<element_t>();
				collector.push_back(id);
				collect_components<I + 1>(collector);
			}
		}
	};
}