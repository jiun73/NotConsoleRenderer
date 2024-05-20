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


struct Modifier;
struct ModifierHandler;
struct TimeManager;