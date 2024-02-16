#pragma once
//#include "Functions.h"
#include <iostream>
#include <string>
#include <cstring>
#include <map>
#include "Vec2D.h"
#include "Rect.h"
#include "StringManip.h"
#include "Color.h"

enum Destringify_errors
{
	STRINGS_NO_TRANSLATION = -1,
	STRINGS_TRANSLATION_FAILED = 0,
	STRINGS_TRANSLATION_SUCCESS = 1
};

namespace strings_hidden //This is used by stringify to try and use the '<<' operator to convert to string
{
	template<typename T> std::ostream& operator<< (std::ostream&, const T&); //only defined within the namespace, it obviously would be bad if it wasn't

	template<typename T>
	std::ostream& operator<<(std::ostream& o, const T&)
	{
		std::string s = "{N/A}";

		for (auto c : s)
			o.put(c);
		return o;
	}
}

namespace strings
{
	//turn any type into a string! (if defined)
	template<typename T> inline std::string stringify(const T& x)
	{
		using namespace strings_hidden;

		std::ostringstream ss;

		//strings_hidden::operator<<(ss, x);
		//ss << x;

		return ss.str();
	}

	//turn any string into a type! (if defined)
	inline int destringify(std::string& v, std::string s)
	{
		v = s;
		return true;
	}

	inline std::string stringify(const int& x) { return std::to_string(x); }

	inline std::string stringify(const char& x) { std::string s;  s.push_back(x);  return s; }
	inline std::string stringify(const unsigned int& x) { return std::to_string(x); }
	inline std::string stringify(const bool& x) { return std::to_string(x); }
	inline std::string stringify(const double& x) { return std::to_string(x); };
	inline std::string stringify(const std::string& x) { return x; };

	//void tolowerstring(std::string& s);

	inline int destringify(bool& target, std::string s)
	{
		if (isNum(s))
		{
			if (s == "0")
			{
				target = false;
			}
			else
				target = true;

		}
		else
		{
			//tolowerstring(s);
			//TODO: fix this
			if (s == "true")
			{
				target = true;
			}
			else if (s == "false")
			{
				target = false;
			}
			else
				return 0;
		}
		return 1;
	}

	template<typename T> inline int destringify(const T& target, std::string s)
	{
		return -1; //cannot translate
	}

	inline int destringify(double& target, std::string s)
	{
		target = stod(s);
		return true;
	}

	inline int destringify(float& target, std::string s)
	{
		target = stof(s);
		return true;
	}

	inline int destringify(int& target, std::string s)
	{
		if (!isNum(s)) return false;

		target = stoi(s);
		return true;
	}

	inline int destringify(uint32_t& target, std::string s)
	{
		if (!isNum(s)) return false;

		target = stoi(s);
		return true;
	}

	inline int destringify(size_t& target, std::string s)
	{
		if (!isNum(s)) return false;

		target = stoi(s);
		return true;
	}

	//turn any type into a string! (if defined)
	template<typename T> inline std::string stringify(const std::vector<T>& x)
	{
		if (x.empty())
			return "{}";

		std::stringstream ss;
		ss << "{";
		bool f = true;
		for (auto& i : x)
		{
			if (!f)
				ss << ",";

			ss << " ";
			ss << stringify(i);
			ss << " ";

			f = false;
		}
		ss << "}";

		return ss.str();
	}



	template<typename T> int destringify(std::vector<T>& vec, std::string s)
	{
		vec.clear();
		std::vector<std::string> args = split(s, ',');
		for (auto& arg : args)
		{
			T obj;
			int err = strings::destringify(obj, arg);
			if (err == true)
				vec.push_back(obj);
			else
			{
				vec.clear();
				return err;
			}
		}
		return true;
	}

	template<typename T> inline std::string stringify(const Vector2D<T>& x)
	{
		std::stringstream ss;
		ss << x;

		return ss.str();
	}

	template<typename T> inline std::string stringify(const vRectangle<T>& x)
	{
		std::stringstream ss;
		ss << x;

		return ss.str();
	}

	template<typename T>
	inline int destringify(Vector2D<T>& v, std::string s)
	{
		std::stringstream ss(s);
		std::vector<std::string> out;
		const char* delim = ",";

		while (std::getline(ss, s, *delim)) {
			out.push_back(s);
		}

		if (out.size() > 2)
			return false;
		else if (out.size() == 1)
			v.x = v.y = std::stoi(out[0]);
		else
		{
			v.x = std::stoi(out[0]);
			v.y = std::stoi(out[1]);
		}

		return true;
	}

	inline int destringify(Color& v, std::string s)
	{
		std::vector<std::string> vec = split(s, ',');

		/*if (s == "rainbow")
		{
			v = AP22::getRainbow();
			return true;
		}*/

		if (vec.size() < 3 || vec.size() > 4)
			return 0;

		if (vec.size() >= 3)
		{
			int i;
			strings::destringify(i, vec.at(0));
			v.r = (uint8_t)i;
			strings::destringify(i, vec.at(1));
			v.g = (uint8_t)i;
			strings::destringify(i, vec.at(2));
			v.b = (uint8_t)i;
			if (vec.size() == 4)
			{
				strings::destringify(i, vec.at(3));
				v.a = (uint8_t)i;
			}
		}

		return true;
	}

	template<typename T1, typename T2>
	inline std::string stringify(const std::pair<T1, T2>& x)
	{
		std::stringstream ss;
		ss << "{";
		ss << stringify(x.first);
		ss << ",";
		ss << stringify(x.second);
		ss << "}";

		return ss.str();
	}

	template<typename T1, typename T2>
	inline int destringify(std::pair<T1, T2>& v, std::string s)
	{
		std::vector<std::string> vec = split(s, ',');

		if (vec.size() != 2) return false;

		destringify(v.first, vec.at(0));
		destringify(v.second, vec.at(1));

		return true;
	}

	template<typename T1, typename T2>
	inline std::string stringify(const std::map<T1, T2>& x)
	{
		std::stringstream ss;

		ss << "{";
		for (auto& p : x)
		{
			ss << stringify(p);
		}
		ss << "}";

		return ss.str();
	}

	template<typename T>
	inline std::string stringify(T* x)
	{
		std::stringstream ss;

		ss << x;

		return ss.str();
	}
}
