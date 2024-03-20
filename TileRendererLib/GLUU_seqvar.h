#pragma once
#include "GLUU_expr.h"

#include <memory>

namespace GLUU {
	template<typename T>
	class SeqVar
	{
	private:
		shared_generic generic = nullptr;
		Expression seq;
		bool is_seq = false;

	public:
		SeqVar() : seq(std::make_shared<bool>()) {}
		SeqVar(const T& df);
		~SeqVar() {}

		template<typename ParserType>
		void set(string_ranges str, ParserType& parser);
		shared_generic get_gen() { return generic; }

		T& get()
		{
			if (generic == nullptr)
				generic = make_generic<T>();
			if (!is_seq)
				return *(T*)generic->raw_bytes();
			else
			{
				auto eval = seq.evaluate();

				if (eval == nullptr)
				{
					assert(false); //Fatal error! Check returns for your function
				}

				return *(T*)eval->raw_bytes();
			}
		}

		operator T() { return get(); }
		T& operator->() { return get(); }
		T& operator()() { return get(); }
	};

	template<typename T>
	inline SeqVar<T>::SeqVar(const T& df) : seq(std::make_shared<bool>())
	{
		get() = df;
	}

	template<typename T>
	template<typename ParserType>
	inline void SeqVar<T>::set(string_ranges str, ParserType& parser) {
		if (!str.empty() && str.begin() != str.end())
		{
			if (*str.begin() == parser.expr_open.at(0) && *prev(str.end()) == parser.expr_close.at(0))
			{
				seq = parser.parse_expression(str);
				is_seq = true;
				return;
			}
		}

		generic = make_generic<T>();
		generic->destringify(str.flat());
	}
}
