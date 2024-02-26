#pragma once
#include "GLUU_expr.h"

namespace GLUU {
	template<typename T>
	class SeqVar
	{
	private:
		shared_generic generic = nullptr;
		Expression seq;
		bool is_seq = false;

	public:
		SeqVar() {}
		SeqVar(const T& df);
		~SeqVar() {}

		template<typename ParserType>
		void set(const string& str, ParserType& parser);
		shared_generic get_gen() { return generic; }

		T& get()
		{
			if (generic == nullptr)
				generic = make_generic<T>();
			if (!is_seq)
				return *(T*)generic->raw_bytes();
			else
				return *(T*)seq.evaluate()->raw_bytes();
		}

		operator T() { return get(); }
		T& operator->() { return get(); }
		T& operator()() { return get(); }
	};

	template<typename T>
	inline SeqVar<T>::SeqVar(const T& df)
	{
		get() = df;
	}

	template<typename T>
	template<typename ParserType>
	inline void SeqVar<T>::set(const string& str, ParserType& parser) {
		if (!str.empty() && str.begin() != str.end())
		{
			if (*str.begin() == parser.expr_open && *prev(str.end()) == parser.expr_close)
			{
				string strcop = str;
				seq = parser.parse_sequence(strcop);
				is_seq = true;
				return;
			}
		}

		generic = make_generic<T>();
		generic->destringify(str);
	}
}
