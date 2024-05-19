#pragma once
#include <map>
#include <unordered_map>
#include <array>
#include <vector>
#include <string>
#include <new>
#include <typeindex>
#include <cassert>
#include <tuple>
#include <functional>

typedef char HandlerID; //that would leave us with only 256 possible event types, but if we make them generic enough, it can work
typedef size_t FieldID;
typedef size_t ActorID;
typedef size_t FieldByteIndex;
typedef size_t TimeMs;
typedef size_t Bitmask64;
typedef size_t DatatypeID;
typedef char* RawData;
constexpr size_t MAX_DATA_TYPES() { return (sizeof(Bitmask64) * 8); }

using std::map;
using std::unordered_map;
using std::string;
using std::vector;
using std::launder;
using std::type_index;
using std::function;
using std::pair;

inline vector<size_t> get_list_from_bytes(Bitmask64 bytes)
{
	vector<size_t> ret;
	for (size_t i = 0; i < 32; i++)
	{
		if (bytes & (1ull << i))
		{
			ret.push_back(i);
		}
	}
	return ret;
}

struct Modifier;
struct ModifierHandler;
struct TimeManager;